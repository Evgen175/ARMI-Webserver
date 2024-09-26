#include "http_server.h"
namespace http_server{


void HttpServer::slotSendMessage(QString messag)
{
    emit signalMessage(messag);
}



}
