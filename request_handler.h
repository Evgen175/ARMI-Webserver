#ifndef HANDLER_H
#define HANDLER_H
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QUrlQuery>
#include "file_handler.h"
#include "log_handle.h"



extern QString pathData_;
extern QString pathArmXML_;

inline QString Hex2String(const QString& value){
    return QString::fromUtf8(QByteArray::fromPercentEncoding(value.toUtf8().replace('+', ' ')));
}

enum class PathScope { DataOnly, DataAndExe, InstallTree };

inline bool PathIsAllowed(const QString& path, PathScope scope)
{
    QStringList roots;
    if (!pathData_.isEmpty()) {
        roots << pathData_;
        if (scope == PathScope::InstallTree) {
            roots << QFileInfo(QDir(pathData_).absolutePath()).absolutePath();
        }
    }
    if (scope != PathScope::DataOnly && !pathArmXML_.isEmpty()) {
        roots << pathArmXML_;
    }
    for (const QString& root : roots) {
        if (PathIsUnderRoot(root, path)) {
            return true;
        }
    }
    return false;
}

inline void ParseQueryInto(ElementBody& request)
{
    const QString tgt = request.target();
    const int qpos = tgt.indexOf(QLatin1Char('?'));
    if (qpos < 0) {
        return;
    }
    request.target(tgt.left(qpos));
    const QUrlQuery query(tgt.mid(qpos + 1));
    const auto items = query.queryItems();
    for (const auto& item : items) {
        if (item.first.compare(QLatin1String("path"), Qt::CaseInsensitive) == 0
                && request.find("PATH").isEmpty()) {
            request(QByteArray("PATH"), item.second.toUtf8());
        }
        if (item.first.compare(QLatin1String("body"), Qt::CaseInsensitive) == 0
                && request.find("BODY").isEmpty()) {
            request(QByteArray("BODY"), item.second.toUtf8());
        }
    }
}

inline void Reject(ElementBody& response, const char* status)
{
    response.status(status);
    response.body(status);
}


namespace http_handler {

class RequestHandler : public QObject
{
    Q_OBJECT
public:
    explicit RequestHandler(QObject* parent = 0) : QObject(parent) {};
    ElementBody operator()(ElementBody& request) {

        ElementBody response;
        response.version(QStringLiteral("HTTP/1.1"));
        response("Connection", "close");

        if (request.method() != "GET")
        {
            Reject(response, file_handler::STATUS::BAD_RREQUEST);
        }
        else {
                response.version(request.version());
                request.target(Hex2String(request.target()));
                ParseQueryInto(request);
                auto target = request.target().split("/");
                QString tmpPath;

                if (target.size() < 2) {
                    Reject(response, file_handler::STATUS::METHOD_NOT_ALLOWED);
                }
                else
                if (target.size() > 2 && target.at(1) == "API") {
                    if (target.at(2) == "LOG") {
                        auto pth = Hex2String(QString::fromUtf8(request.find("PATH")));
                        auto bd = request.find("BODY");
                        if (!PathIsAllowed(pth, PathScope::InstallTree)) {
                            Reject(response, file_handler::STATUS::FORBIDDEN);
                        } else if (logHandler_(pth, bd)) {
                            response.status(file_handler::STATUS::OK);
                            response.body(file_handler::STATUS::OK);
                        } else {
                            response.status(file_handler::STATUS::INTERNAL_ERROR);
                            response.body(file_handler::STATUS::INTERNAL_ERROR);
                        }
                    }
                    else if (target.at(2) == "FILE") {
                        const QString pathHdr = Hex2String(QString::fromUtf8(request.find("PATH")));
                        const QString file = QFileInfo(pathHdr).fileName();
                        qDebug() <<  file;

                        tmpPath = file.compare(QLatin1String("Arm.xml"), Qt::CaseInsensitive) == 0
                                ? QDir(pathArmXML_).filePath(QStringLiteral("Arm.xml"))
                                : pathHdr;
                        qDebug() << tmpPath;
                        if (!PathIsAllowed(tmpPath, PathScope::DataAndExe)) {
                            Reject(response, file_handler::STATUS::FORBIDDEN);
                        } else {
                            fileHandler_.HandleFile(response, tmpPath);
                        }
                    }
                    else {
                        Reject(response, file_handler::STATUS::NOT_FOUND);
                    }
                }
                else {
                    QString rel = request.target();
                    if (rel.startsWith(QLatin1Char('/'))) {
                        rel = rel.mid(1);
                    }
                    tmpPath = QDir::cleanPath(QDir(pathData_).absoluteFilePath(rel));
                    if (!PathIsAllowed(tmpPath, PathScope::DataOnly)) {
                        Reject(response, file_handler::STATUS::FORBIDDEN);
                    } else {
                        fileHandler_.HandleFile(response, tmpPath);
                    }
                }
            qDebug() << "API = " << tmpPath;
            }

        qint64 contentLength = 0;
        if (!response.body().isEmpty()) {
            contentLength = (response.body().size() - 1) * file_handler::SIZE_BLOCK
                            + response.body().last().size();
        }
        qDebug() <<  "РАЗМЕР = " << contentLength << request.target();
        response("Content-Length", QString::number(contentLength).toUtf8());

        return response;
    }
private:
    file_handler::FileHandler fileHandler_;
    HANDLER_LOG::LogHandler logHandler_;
};
} //namespace
#endif // HANDLER_H
