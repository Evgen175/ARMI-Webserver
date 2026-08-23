#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <QList>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

/// служедные поля запроса / ответа
using ServiceFields = QList<QPair<QByteArray, QByteArray>>;
using BodyResponse  = QList<QByteArray>;

//----------------------------------------------------
static QList <QPair<QString, QList<QString>>> extension_{
      {"text/html", {".htm", ".html"}}
    , {"text/css", {".css"}}
    , {"text/plain", {".txt"}}
    , {"text/javascript", {".js"}}
    , {"application/json", {".json"}}
    , {"application/xml", {".xml"}}
    , {"image/png", {".png"}}
    , {"image/jpeg", {".jpg", ".jpe", ".jpeg"}}
    , {"image/gif", {".gif"}}
    , {"image/bmp", {".bmp"}}
    , {"image/vnd.microsoft.icon", {".ico"}}
    , {"image/tiff", {".tiff", ".tif"}}
    , {"image/svg+xml", {".svg", ".svgz"}}
    , {"audio/mpeg", {".mp3"}}
    , {"application/x-shockwave-flash", {".swf"}}
    , {"video/x-msvideo", {".avi"}}
    , {"video/mp4", {".mp4"}}
    , {"application/octet-stream", {".fbx"}}
};
//----------------------------------------------------
static QString FindExtension(const QString extension) {
    for (const auto it : extension_) {
        for (const auto& it_se : it.second) {
            if (it_se == extension) {
                return  it.first;
            }
        }
    }
    return "application/octet-stream";
}

/// true, если path лежит внутри root (после clean/canonical).
inline bool PathIsUnderRoot(const QString& root, const QString& path)
{
    if (root.isEmpty() || path.isEmpty()) {
        return false;
    }
    QString rootCanon = QDir(root).canonicalPath();
    if (rootCanon.isEmpty()) {
        rootCanon = QDir::cleanPath(QDir(root).absolutePath());
    }
    const QFileInfo fi(QDir::cleanPath(QDir::fromNativeSeparators(path)));
    QString target = fi.exists() ? fi.canonicalFilePath() : QDir::cleanPath(fi.absoluteFilePath());
    if (rootCanon.isEmpty() || target.isEmpty()) {
        return false;
    }
    const QString rootNative = QDir::toNativeSeparators(rootCanon);
    const QString targetNative = QDir::toNativeSeparators(target);
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
    if (targetNative.compare(rootNative, cs) == 0) {
        return true;
    }
    QString prefix = rootNative;
    if (!prefix.endsWith(QDir::separator())) {
        prefix += QDir::separator();
    }
    return targetNative.startsWith(prefix, cs);
}
//----------------------------------------------------
class ElementBody {
public:
    struct STATUS {
        const QString OK = "200 OK";
        const QString BAD_RREQUEST = "400 Bad Request";
        const QString FORBIDDEN = "403 Forbidden";
        const QString NOT_FOUND = "404 Not Found";
        const QString METHOD_NOT_ALLOWED = "405 Method Not Allowed";
    };

    ElementBody(){}
    ~ElementBody() = default;
    void operator() (QByteArray& byteArray){
        requestByte_ = std::move(byteArray);
        auto tmp = requestByte_.split('\n');
        if (tmp.isEmpty()) {
            return;
        }
        auto headRequest = tmp.at(0).trimmed().split(' ');
        if (headRequest.size() >= 2) {
            method_ = QString::fromUtf8(headRequest.at(0).trimmed());
            target_ = QString::fromUtf8(headRequest.at(1).trimmed());
            version_ = headRequest.size() >= 3
                    ? QString::fromUtf8(headRequest.at(2).trimmed())
                    : QStringLiteral("HTTP/1.1");

            for (int index = 1; tmp.size() > index; ++index)
            {
                auto line = tmp.at(index).trimmed();
                if (line.isEmpty()) {
                    break;
                }
                const int ind = line.indexOf(':');
                if (ind > 0) {
                    serviceFields_.append({line.left(ind).trimmed(),
                                           line.mid(ind + 1).trimmed()});
                }
            }
        }
    }

    ElementBody(const ElementBody& eb)
    {
        requestByte_   = eb.requestByte_;
        body_          = eb.body_;
        method_        = eb.method_;
        target_        = eb.target_;
        version_       = eb.version_;
        serviceFields_ = eb.serviceFields_;
        status_        = eb.status_;
    }

    ServiceFields elements(){
        return serviceFields_;
    }

    BodyResponse body(){
        return body_;
    }

    void body(QByteArray line){
        body_.push_back(std::move(line));
    }
    void bodyClear(){
        body_.clear();
    }

    QString method(){
        return method_;
    };

    QString target(){
        return target_;
    };
    void target(QString val) {
        target_ = std::move(val);
    }

    QByteArray version(){
        return version_.toUtf8();
    };

    void version(QString version)
    {
        version_ = version;
    }
    QByteArray status() {
        return status_.toUtf8();
    }

   void status(QString status) {
        status_ = status;
    }

    void clear(){
        requestByte_.clear();
        method_.clear();
        target_.clear();
        version_.clear();
        serviceFields_.clear();
        status_.clear();
        body_.clear();
    }
     QByteArray const requestByte(){
        return requestByte_;
    }
    void requestByte(QByteArray response) {
        requestByte_ = std::move(response);
    }

    ElementBody(QByteArray name, QByteArray data){
        serviceFields_.append({std::move(name), std::move(data)});
    }
    void operator()(QByteArray name, QByteArray data){
        if (find(name).isEmpty()) serviceFields_.append({name, data});
    }

    QByteArray find(QString name, QByteArray data = QByteArray()) {

        for (auto &it : serviceFields_) {
            if (QString::fromLatin1(it.first).compare(name, Qt::CaseInsensitive) == 0) {
                if (!data.isNull()) it.second = data;
                return it.second;
            }
        }
        return QByteArray();
    }
private:
    QByteArray requestByte_;
    BodyResponse body_;
    QString method_;
    QString target_;
    QString version_;
    ServiceFields serviceFields_;
    QString status_;
};
//----------------------------------------------------

namespace file_handler {

struct STATUS {
    constexpr static  const char* OK                    = "200 OK";
    constexpr static  const char* BAD_RREQUEST          = "400 Bad Request";
    constexpr static  const char* FORBIDDEN             = "403 Forbidden";
    constexpr static  const char* NOT_FOUND             = "404 Not Found";
    constexpr static  const char* METHOD_NOT_ALLOWED    = "405 Method Not Allowed";
    constexpr static  const char* NOT_IMPLEMENTED       = "501 Not Implemented";
    constexpr static  const char* INTERNAL_ERROR        = "500 Internal Server Error";
};

const int SIZE_BLOCK = 1024;

class FileHandler{

public:
    ElementBody MakeFileResponse(ElementBody& response, QString pathFile) {
        const QFileInfo info(pathFile);
        const QString fileName = info.fileName();
        const QString fileExten = info.suffix().isEmpty()
                ? QString()
                : QStringLiteral(".") + info.suffix().toLower();
        qDebug() << "MakeFileResponse " << fileExten << fileName;
        if (!fileExten.isEmpty())
        {
            QFile file(pathFile);
            if (file.exists()){
                if (!file.open(QIODevice::ReadOnly)) {
                    response.status(file_handler::STATUS::INTERNAL_ERROR);
                    response.body("cannot open file");
                    return response;
                }
                response.status(file_handler::STATUS::OK);
                response("Content-Type", FindExtension(fileExten).toUtf8());
                while (file.bytesAvailable())
                {
                       response.body(file.read(SIZE_BLOCK));
                }
                file.close();
            }
            else {
                response.status(file_handler::STATUS::NOT_FOUND);
                response.body("file not found");
            }

        } else {
            response.status(file_handler::STATUS::BAD_RREQUEST);
            QByteArray err = file_handler::STATUS::BAD_RREQUEST;
            response.body(err);
        }

        return response;
    }
//--------------------------------------------------
    ElementBody HandleFile(ElementBody& response, QString path){
        return MakeFileResponse(response, path);
    }
};
} // namespace
#endif // FILE_HANDLER_H
