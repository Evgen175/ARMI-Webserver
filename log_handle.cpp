#include "log_handle.h"


namespace HANDLER_LOG {



bool LogHandler::operator()(QString strFileXML, QByteArray byteArray) {
    message_ = byteArray;
    auto tmpArray = message_.split('|');
    strFileXML_ = strFileXML;


  //  qDebug() << message_.data() << "\n" << message_.toStdString().c_str();


    if (!QDir(strFileXML_.left(strFileXML_.size() - strFileXML_.split("/").last().size() )).exists()) return false;



    bool blnExam = tmpArray.at(0).indexOf("EXAM") >= 0;

    QMap<QString, QString> mapLog;
    for (int i = 1; tmpArray.size() > i; ++i) {
        auto spl = tmpArray.at(i).split('=');
      //  qDebug() << spl.first() << "\t" << spl.last().data();
        mapLog.insert(spl.first(), spl.last().data());
    }
        //открытие и чтение .xml
        //если файла нет, то создаем
        QFile fileXML(strFileXML_);
        if (!fileXML.exists())
        {
            fileXML.open(QIODevice::WriteOnly);
            QXmlStreamWriter xmlStreamWriter(&fileXML);
            xmlStreamWriter.setAutoFormatting(true);
            xmlStreamWriter.writeStartDocument();
            xmlStreamWriter.writeStartElement("Log");
            xmlStreamWriter.writeEndElement();
            xmlStreamWriter.writeEndDocument();
            fileXML.close();
        }
        QFile xmlFile(strFileXML_);
        xmlFile.open(QIODevice::ReadWrite);//открытие и чтение .xml
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
                        QDomElement domEQ=domDocXML.createElement(strQuestionKey);//ключ
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
            QDomElement domE=domDocXML.createElement(mapLog["Key"]);
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
        //запись QDomDocument в xml файл
      return  WriteDocXML(domDocXML, strFileXML);
}
//================================================================================
bool LogHandler::WriteDocXML(QDomDocument domDocXML, QString strFile)
{
    bool blnOk=false;
    //открытие и чтение .xml
    QFile fileXML;
    if (!fileXML.exists(strFile))
    {
        //QMessageBox msgBox;
//        msgBox.setIcon(QMessageBox::Critical);
//        msgBox.setText("Файл не найден: \n"+strFile);
//        msgBox.exec();
    }
    else
    {
        //копия старого
        if (fileXML.exists(strFile+".bak")) fileXML.remove(strFile+".bak");
        fileXML.copy(strFile, strFile+".bak");
        //Открытие файла для записи
        fileXML.setFileName(strFile);
        if (!fileXML.open(QIODevice::ReadWrite))
        {
//            QMessageBox msgBox;
//            msgBox.setIcon(QMessageBox::Critical);
//            msgBox.setText("Файл для записи не открывается: \n"+strFile);
//            msgBox.exec();
        }
        else
        {
            //обнуление и запись domDocXML в fileXML
            fileXML.resize(0);
            if (!fileXML.write(domDocXML.toByteArray()))
            {
//                fileXML.close();
//                QMessageBox msgBox;
//                msgBox.setIcon(QMessageBox::Critical);
//                msgBox.setText("Ошибка записи в файл: \n"+strFile);
//                msgBox.exec();
            }
            else blnOk=true;
            fileXML.close();
        }
    }
    return blnOk;
}
} // namespace
