#include "http_server.h"

#include <QDateTime>
#include <QStringList>
#include <QUrl>

namespace http_server{

void HttpServer::slotSendMessage(QString messag)
{
    emit signalMessage(messag);
}

QByteArray SessionTask::readHttpRequest(QTcpSocket& socket)
{
    QByteArray data;
    QElapsedTimer timer;
    timer.start();
    const int timeoutMs = 8000;
    const int maxSize = 256 * 1024;
    while (timer.elapsed() < timeoutMs && data.size() < maxSize) {
        if (data.contains("\r\n\r\n") || data.contains("\n\n")) {
            break;
        }
        const int remain = timeoutMs - static_cast<int>(timer.elapsed());
        if (!socket.waitForReadyRead(qMax(1, remain))) {
            break;
        }
        data += socket.readAll();
    }
    return data;
}

void SessionTask::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(descriptor_)) {
        emit signalSendLog(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss")
                           + "   socket error  " + socket.errorString());
        return;
    }

    QElapsedTimer timeStart;
    timeStart.start();
    QByteArray raw = readHttpRequest(socket);
    ElementBody request;
    request(raw);

    http_handler::RequestHandler handler;
    ElementBody response = handler(request);

    socket.write(response.version() + " " + response.status() + "\r\n");
    for (auto it : response.elements()) {
        socket.write(it.first + ": " + it.second + "\r\n");
    }
    socket.write("\r\n");
    for (auto it : response.body()) {
        socket.write(it);
    }
    socket.flush();
    socket.waitForBytesWritten(3000);

    QStringList tm;
    tm.append(request.target() == "/API/LOG" ? QString::fromUtf8(request.find("PATH"))
                                             : socket.peerAddress().toString());
    tm.append({QString::fromUtf8(response.status())
                , QUrl::fromPercentEncoding(request.target().toUtf8())
                , QString::fromUtf8(request.find("PATH"))
                , QString::number(static_cast<qint64>(timeStart.elapsed())) + QLatin1String(" msec")});
    QString messS = QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))
                    + QLatin1String("   ");
    for (const QString& it : tm) {
        messS += it + QLatin1String("  ");
    }
    emit signalSendLog(messS);

    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState) {
        socket.waitForDisconnected(1000);
    }
}

}
