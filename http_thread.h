#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QDebug>
//#include <QDateTime>
#include <QRandomGenerator>
#include <QDataStream>

class ServerThread : public QThread
{
public:
    explicit ServerThread(int socketID, const QString& request, QObject* parent);

protected:
    void run() override;

signals:
    void error(QTcpSocket::SocketError socket_error);

public slots:
    void readyRead(/*QTcpSocket& socket*/);
    void disconnected();

private:
    int socketDescriptor_;
    QString text_;
    QTcpSocket* socket_;

   // QTcpSocket* socket;
   // qintptr socketDescriptor;
};

#endif // SERVERTHREAD_H
