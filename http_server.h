#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <qthread.h>
#include <QElapsedTimer>
#include <QUrl>

#include "request_handler.h"


class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject *parent) : QObject(parent){}

    void sendMes (QStringList mess)
    {
        auto messS = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss").toLatin1() + "   ";
        for ( auto it : mess)
        messS += it + "  ";
        emit signalSendLog(messS);
    }
signals:
    void signalSendLog(QString mess);
};


namespace http_server {

class SessionBase : public Logger
{
    Q_OBJECT
    QByteArray byteArray_{};
    ElementBody request_;
    qintptr descriptor_;
    QTcpSocket* socket_;
    http_handler::RequestHandler handler_;
    QElapsedTimer timeStart_;
public:
    explicit SessionBase(qintptr descriptor, QObject* parent = 0)
        : Logger(parent), descriptor_(descriptor)
    {
        socket_ = new QTcpSocket(this);
        if (!socket_->setSocketDescriptor(descriptor_)){
            QString err = socket_->errorString();
            auto addr =socket_->peerAddress().toString();
            Logger::sendMes({addr, err});
            QMessageBox::critical(0, "Error", "Connection errore", socket_->errorString());
        }
    }
//-------------------------------------------------------
    void slotNewConnection()//ДЛЯ СЕТИ
    {
        connect(socket_, &QTcpSocket::disconnected, this, &SessionBase::slotDisconnectEvent);//отключившийся обучаемый
        connect(socket_, &QTcpSocket::readyRead, this, &SessionBase::slotRequestRead);//получение от АРМО
    }

    ~SessionBase(){
//        logger_.Message("~SessionBase()");
      //  emit destroyed();
    };

private slots:
    void slotRequestRead()
    {
        timeStart_.start();
        byteArray_ = socket_->readAll();
        socket_->waitForReadyRead(1);
        request_.clear();
        request_(byteArray_);

        onRead();
    }

    void slotDisconnectEvent()
    {
        socket_->disconnect(this);
        auto rt = "Disconnected" + socket_->peerAddress().toString();
        destroyed();
}

private:

    void onRead(){
        writeResponse(handler_(request_));
    }


    void writeResponse(const ElementBody&& resp)
    {
        ElementBody response = std::move(resp);
        socket_->write(response.version() + " " + response.status() + "\r\n");
        for (auto it : response.elements()) // ОСТАЛЬНЫЕ ПОЛЯ
        {
            socket_->write(it.first + ": " + it.second + "\r\n");
        }

        // ПРИМЕР auto response_ = "HTTP/1.1 200 OK\r\nContent-Length: " + QString::number(str.toUtf8().size()) + "\r\n\r\n" + str;
        socket_->write("\r\n");
        for (auto it : response.body() ){// Тело ответа
            socket_->write(it);
        }
        socket_->flush();
        QStringList tm;
        tm.append(request_.target() == "/API/LOG" ? request_.find("PATH") : socket_->peerAddress().toString().split(":").last());
        tm.append({response.status()
                    , QUrl::fromPercentEncoding(request_.target().toLatin1())
                    , request_.find("PATH").data()
                    , QString::number((qint64)timeStart_.elapsed()) + " msec"});
        Logger::sendMes(tm);
        socket_->close();
        qDebug() << "CLOSE " << socket_->flush();
    }
};


//-------------------------------------------------------------
class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    HttpServer(QObject* parent = 0) :QTcpServer(parent){
    }
    HttpServer(QHostAddress hostAddressARMI, uint port, QObject* parent = 0)
        : QTcpServer(parent), hostAddressARMI_(hostAddressARMI), port_(port)
    {
        if (!listen(hostAddressARMI_, port_)) //установка номера порта
        {
            QMessageBox::critical(0,"Сервер","Сервер не запущен\n" + errorString());
            close();
            std::exit(0);
        }
    };


//    Logger logger_;

public slots:
    void slotSendMessage(QString messag);

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        SessionBase *sessionBase_ = new SessionBase(socketDescriptor);
        QThread* thread = new QThread();
        sessionBase_->moveToThread(thread);
        connect(thread, &QThread::started, [sessionBase_](){
            sessionBase_->slotNewConnection();
        });
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(sessionBase_, &SessionBase::destroyed, thread, &QThread::quit);
        connect(sessionBase_, &SessionBase::destroyed, [sessionBase_]() {
            qDebug() << "DELETE_LETER " << QThread::currentThreadId();
            sessionBase_->deleteLater();
        });
        connect(sessionBase_, &SessionBase::Logger::signalSendLog, [=](QString mess){emit signalMessage(mess);});
        qDebug() << "QThread::currentThreadId()" << QThread::currentThreadId();
        thread->start();
    }

private:
    SessionBase *session_;
    QHostAddress hostAddressARMI_;
    uint port_;
    QMap <int, int> mapUsersConnected;

signals:
    void signalMessage(QString messag);
};


} //namespase
#endif // HTTP_SERVER_H
