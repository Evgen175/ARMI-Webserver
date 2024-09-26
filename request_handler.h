#ifndef HANDLER_H
#define HANDLER_H
#include <QDebug>
#include <iostream>
#include <thread>
#include <QApplication>
#include "file_handler.h"
#include "log_handle.h"



extern QString pathData_;
extern QString pathArmXML_;
//extern QString slash_;




inline QString Hex2String(const QString& value){
    auto valtmp = value.toLatin1();
    //if (value.find("%") == std::string::npos) return value.data();
    QString str = "";
    for (auto t = 0; t < valtmp.size(); ++t){
        if (valtmp.at(t) == '%' && valtmp.size() > (t + 2)){
            str += static_cast<char>((int) std::strtol(valtmp.mid(t + 1, 2), nullptr, 16));
            t+=2;
        }
        else {
            if (valtmp[t] == '+') {
                str += " ";
            } else
            str += valtmp.at(t);
        }
    }
    return str.toLatin1().toStdString().data();
}


namespace http_handler {

class RequestHandler : public QObject
{
    Q_OBJECT
public:
    explicit RequestHandler(QObject* parent = 0) : QObject(parent) {};
    ElementBody operator()(ElementBody& request) {

        ElementBody response; // Создание ответа

        if (request.method() != "GET")
        {
            response.status(file_handler::STATUS::BAD_RREQUEST);
            response.body(file_handler::STATUS::BAD_RREQUEST);
        }
        else
            if (request.method() == "GET") {
                response.version(request.version()); // Установка верии в соответствии с запросом
                request.target(Hex2String(request.target())); // проверка пути
                auto target = request.target().split("/");
                QString tmpPath;

                if (target.size() < 2) {
                    response.status(file_handler::STATUS::METHOD_NOT_ALLOWED);
                    response.body(file_handler::STATUS::METHOD_NOT_ALLOWED);
                }
                else
                if (target.at(1) == "API") {
                    if (target.at(2) == "LOG") {

                       // qDebug() << " +++++++++++++++" << target;
                        auto pth = Hex2String(request.find("PATH").data());
                        auto bd = request.find("BODY");
                        if (logHandler_(pth, bd)) {
                            response.status(file_handler::STATUS::OK);
                            response.body(file_handler::STATUS::OK);
                        } else {
                            response.status(file_handler::STATUS::NOT_IMPLEMENTED);
                            response.body(file_handler::STATUS::NOT_IMPLEMENTED);
                        }
                    }

                    if (target.at(2) == "FILE") {
                        auto file = Hex2String(request.find("PATH")).split("/").last();
                        qDebug() <<  file;

                        tmpPath = file == "Arm.xml" ? pathArmXML_ + "/Arm.xml" : Hex2String(request.find("PATH"));
                        qDebug() << tmpPath;
                        fileHandler_.HandleFile(response, tmpPath);
                    }
                }// API
                else {
                    fileHandler_.HandleFile(response, pathData_ + request.target());
                }
            qDebug() << "API = " << tmpPath;
            } // GET

        qDebug() <<  "РАЗМЕР = " << (response.body().size() - 1) * file_handler::SIZE_BLOCK + response.body().last().size() << request.target();
       // qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" << response.body().at(0).left(100) << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~";
        response("Content-Length", QString::number((response.body().size() - 1) * file_handler::SIZE_BLOCK + response.body().last().size()).toLocal8Bit());




       // qDebug() << response.body();
        return response;
    }
private:
    file_handler::FileHandler fileHandler_;
    HANDLER_LOG::LogHandler logHandler_;
};
} //namespace
#endif // HANDLER_H
