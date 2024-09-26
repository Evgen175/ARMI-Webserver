#include "http_thread.h"

ServerThread::ServerThread(int socketID, const QString& request, QObject* parent) : QThread(parent), socketDescriptor_(socketID), text_(request)
{
}

void ServerThread::run()
{
    socket_ = new QTcpSocket();


    if (!socket_->setSocketDescriptor(socketDescriptor_)) {
      //  emit error(socket_->error());
        return;
    }

    connect(socket_, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
//    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));


    qDebug() << socketDescriptor_ << " Client connected";

    QByteArray response;
    QDataStream out(&response, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_1);
    out << text_;

    socket_->write(response);
    socket_->disconnectFromHost();
    socket_->waitForDisconnected();

    //exec();
}

void ServerThread::readyRead(/*QTcpSocket& socket*/){

    QByteArray data_ = socket_->readAll();

    qDebug() << socketDescriptor_ << " Отправил запрос" << data_;
    socket_->write(data_);
}
void ServerThread::disconnected(){
    qDebug() << socketDescriptor_ << " Отсоединён";
    //socket->deleteLater();
}
