#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QElapsedTimer>
#include <QUrl>

#include "request_handler.h"


class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject *parent) : QObject(parent){}

    void sendMes (QStringList mess)
    {
        QString messS = QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))
                        + QLatin1String("   ");
        for (const QString& it : mess) {
            messS += it + QLatin1String("  ");
        }
        emit signalSendLog(messS);
    }
signals:
    void signalSendLog(QString mess);
};


namespace http_server {

class SessionTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit SessionTask(qintptr descriptor)
        : descriptor_(descriptor)
    {
        setAutoDelete(true);
    }

    void run() override;

signals:
    void signalSendLog(QString mess);

private:
    QByteArray readHttpRequest(QTcpSocket& socket);

    qintptr descriptor_;
};


class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    HttpServer(QObject* parent = 0) :QTcpServer(parent){
    }
    HttpServer(QHostAddress hostAddressARMI, uint port, QObject* parent = 0)
        : QTcpServer(parent), hostAddressARMI_(hostAddressARMI), port_(port)
    {
        QThreadPool::globalInstance()->setMaxThreadCount(
                    qMax(4, QThread::idealThreadCount() * 2));
        if (!listen(hostAddressARMI_, port_))
        {
            QMessageBox::critical(0,"Сервер","Сервер не запущен\n" + errorString());
            close();
        }
    };

public slots:
    void slotSendMessage(QString messag);

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        auto *task = new SessionTask(socketDescriptor);
        connect(task, &SessionTask::signalSendLog, this, &HttpServer::signalMessage,
                Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(task);
    }

private:
    QHostAddress hostAddressARMI_;
    uint port_;

signals:
    void signalMessage(QString messag);
};


} // namespace
#endif // HTTP_SERVER_H
