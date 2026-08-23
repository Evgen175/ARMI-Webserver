#include "log_handle.h"

#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QMap>
#include <QSaveFile>
#include <QRegExp>
#include <QXmlStreamWriter>


namespace HANDLER_LOG {

static QString SafeXmlName(QString name)
{
    name.replace(QRegExp("[^A-Za-z0-9_.-]"), "_");
    if (name.isEmpty() || (!name[0].isLetter() && name[0] != QLatin1Char('_'))) {
        name.prepend(QLatin1Char('_'));
    }
    return name.left(64);
}

bool LogHandler::operator()(QString strFileXML, QByteArray byteArray) {
    message_ = byteArray;
    auto tmpArray = message_.split('|');
    strFileXML_ = strFileXML;

    const QString dirPath = QFileInfo(strFileXML_).absolutePath();
    if (!QDir(dirPath).exists()) return false;

    QLockFile lockFile(strFileXML_ + QLatin1String(".lock"));
    lockFile.setStaleLockTime(10000);
    if (!lockFile.tryLock(3000)) {
        return false;
    }



    bool blnExam = tmpArray.at(0).indexOf("EXAM") >= 0;

    QMap<QString, QString> mapLog;
    for (int i = 1; tmpArray.size() > i; ++i) {
        auto spl = tmpArray.at(i).split('=');
      //  qDebug() << spl.first() << "\t" << spl.last().data();
        mapLog.insert(QString::fromUtf8(spl.first()), QString::fromUtf8(spl.last()));
    }
        //открытие и чтение .xml
        //если файла нет, то создаем
        QFile fileXML(strFileXML_);
        if (!fileXML.exists())
        {
            if (!fileXML.open(QIODevice::WriteOnly)) {
                return false;
            }
            QXmlStreamWriter xmlStreamWriter(&fileXML);
            xmlStreamWriter.setAutoFormatting(true);
            xmlStreamWriter.writeStartDocument();
            xmlStreamWriter.writeStartElement("Log");
            xmlStreamWriter.writeEndElement();
            xmlStreamWriter.writeEndDocument();
            fileXML.close();
        }
        QFile xmlFile(strFileXML_);
        if (!xmlFile.open(QIODevice::ReadWrite)) {
            return false;
        }
        QDomDocument domDocXML;
        domDocXML.setContent(xmlFile.readAll());
        QDomElement domElement = domDocXML.documentElement(); //Получение Root Element
        //режим LOG || LOG_EXAM

        if (blnExam)
        {
            //ищем наш экзамен и попытку
            bool blnYes=false;
            QDomNodeList domNodeList = domElement.childNodes();
            for (int i = 0; i < domNodeList.count(); i++)
            {
                if (domNodeList.at(i).nodeName() == "Exam")//
                {
                    QDomElement domE = domNodeList.at(i).toElement();
                    if (domE.attributeNode("Key").value() == mapLog["Key"] && domE.attributeNode("Attempt").value() == mapLog["Attempt"])
                    {
                        blnYes = true;
                        domE.attributeNode("AnswersTrue").setValue(mapLog["AnswersTrue"]);
                        domE.attributeNode("AnswersFalse").setValue(mapLog["AnswersFalse"]);
                        domE.attributeNode("Finish").setValue(mapLog["Finish"]);
                        domE.attributeNode("ElapsedTime").setValue(mapLog["ElapsedTime"]);
                        domE.attributeNode("ResultVal").setValue(mapLog["ResultVal"]);
                        domE.attributeNode("ResultText").setValue(mapLog["ResultText"]);
                        domE.attributeNode("DateFinish").setValue(mapLog["DateFinish"]);
                        //вопросы
                        QDomNodeList domNodeListQ = domE.childNodes();
                        for (int j = 0; j < domNodeListQ.count(); j++)
                        {
                            if (domNodeListQ.at(j).nodeName().startsWith("_"))
                            {
                                QString strQuestionKey = domNodeListQ.at(j).nodeName();
                                //ищем вопрос по списку mapExam и записываем данные из узла дерева
                                QMap<QString,QString>::iterator it = mapLog.begin();
                                for ( ; it != mapLog.end(); it++)
                                {
                                    if (strQuestionKey == it.key().section(":", 0, 0))
                                    {
                                        QString strAnswer = it.key().section(":", 1);
                                        //переписываем значение аттрибута или добавляем
                                        if (domNodeListQ.at(j).toElement().attributeNode("Answer").isAttr()) domNodeListQ.at(j).toElement().attributeNode("Answer").setValue(strAnswer);
                                        else domNodeListQ.at(j).toElement().setAttribute("Answer", strAnswer);
                                        break;
                                    }
                                }
                            }
                        }//вопросы
                        break;
                     }
                 }
             }
            //если не найден, то создаем новый экзамен
            if (!blnYes)
            {
                QDomElement domE = domDocXML.createElement("Exam");
                //цикл по списку mapExam
                QMap<QString,QString>::iterator it = mapLog.begin();
                for ( ; it != mapLog.end(); it++)
                {
                    if (it.key().left(1) == "_")//вопросы
                    {
                        QString strQuestionKey = it.key().section(":", 0, 0);
                        QString strAnswer = it.key().section(":", 1);
                        QString strQuestion=it.value();
                        QDomElement domEQ=domDocXML.createElement(SafeXmlName(strQuestionKey));
                        domEQ.setAttribute("Question", strQuestion);
                        domEQ.setAttribute("Answer", strAnswer);
                        domE.appendChild(domEQ);
                    }
                    else domE.setAttribute(it.key(), it.value());//аттрибуты - параметры экзамена
                }
                domElement.appendChild(domE);
            }
        }
        else //LOG
        {   //добавляем время обучения
            QDomElement domE=domDocXML.createElement(SafeXmlName(mapLog["Key"]));
            //цикл по списку mapLog
            QMap<QString,QString>::iterator it=mapLog.begin();
            for ( ; it != mapLog.end(); it++)
            {
                if (it.key()!="Key") domE.setAttribute(it.key(), it.value());
                //QDomText domText=domDocXML.createTextNode(it.value());
                //domE.appendChild(domText);
            }
            domElement.appendChild(domE);
        }
        xmlFile.close();
        //запись QDomDocument в xml файл
      return  WriteDocXML(domDocXML, strFileXML);
}
//================================================================================
bool LogHandler::WriteDocXML(QDomDocument domDocXML, QString strFile)
{
    QFile fileXML;
    if (QFile::exists(strFile + ".bak")) {
        QFile::remove(strFile + ".bak");
    }
    if (QFile::exists(strFile)) {
        QFile::copy(strFile, strFile + ".bak");
    }
    QSaveFile saveFile(strFile);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    if (saveFile.write(domDocXML.toByteArray()) < 0) {
        return false;
    }
    return saveFile.commit();
}
} // namespace
