#ifndef HANDLEXML_H
#define HANDLEXML_H

#include <QDebug>
#include <QObject>
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

    bool WriteDocXML(QDomDocument domDocXML, QString strFile);

};


} //namespace

#endif // HANDLEXML_H
