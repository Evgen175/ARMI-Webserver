#ifndef HANDLEXML_H
#define HANDLEXML_H

#include <QDebug>
#include <qstring.h>
#include <QDir>
#include <QXmlStreamWriter>
#include <QtXml/QDomDocument>


namespace  HANDLER_LOG
{

class LogHandler : public QObject {

public:
    LogHandler(){}
    ~LogHandler() {

    };
    bool operator() (QString path, QByteArray byteArray);

private:
    QByteArray message_;
    QString strFileXML_;

    ///сохранение результатов экзамена в Log_NN.xml
    void SaveTraineeLog(int intTraineeId,QString strMode,QMap<QString,QString> mapLog);
    bool WriteDocXML(QDomDocument domDocXML, QString strFile);

};


} //namespace

#endif // HANDLEXML_H
