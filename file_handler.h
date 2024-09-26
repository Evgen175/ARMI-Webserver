#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <QList>
#include <QDebug>
#include <QFile>

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
    , {"video/mp4", {"mp4"}}
    , {"application/octet-stream", {".fbx"}}
};
//----------------------------------------------------
static QString FindExtension(const QString extension) {
    for (const auto it : extension_) {
        for (const auto& it_se : it.second) {
            if (it_se == extension) {
                return  it.first;//. it.data()->toStdString();
                break;
            }
        }
    }
    return "application/octet-stream";
}
//----------------------------------------------------
class ElementBody {
public:
    struct STATUS {
        const QString OK = "200 OK"; // хорошо
        const QString BAD_RREQUEST = "400 Bad Request"; // неправильный, некорректный запрос
        const QString NOT_FOUND = "404 Not Found"; // не найдено
        const QString METHOD_NOT_ALLOWED = "405 Method Not Allowed"; //метод не поддерживается
    };

    ElementBody(){}
    ~ElementBody() = default;
    void operator() (QByteArray& byteArray){
   //     qDebug() << "\n===========START======================";
        requestByte_ = std::move(byteArray);
        auto tmp = requestByte_.split('\n');
        auto headRequest = tmp.at(0).split(' ');

        if (headRequest.count() == 3 ) {
            method_ = headRequest.at(0);
            target_ = headRequest.at(1);
            version_ = headRequest.at(2).left(headRequest.at(2).size() - 1);

            for (int index = 1; tmp.size() > index; ++index)
            {
                auto ind = tmp.at(index).indexOf(": ");
                if (ind > 0)
                    serviceFields_.append({tmp.at(index).left(ind), tmp.at(index).right(tmp.at(index).size() - ind - 2).split('\r').first()});
            }
         //   qDebug() << requestByte_.data() << "\n ===========END======================\n";

            return;
        }
    }

    ///------ Коструктор копирования ---------------------------------------
    ElementBody(const ElementBody& eb) noexcept
    {
        requestByte_   = eb.requestByte_; // Входящий поток данных / тело ответа
        body_          = eb.body_;
        method_        = eb.method_; // Используемый метод запроса
        target_        = eb.target_; // тело запроса
        version_       = eb.version_; // Версия запроса / ответа
        serviceFields_ = eb.serviceFields_; // служедные поля запроса / ответа
        status_        = eb.status_; // статус ответа
    }

    //---------------------------------------------

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
        return version_.toLocal8Bit();
    };

  //  template<typename T>
    void version(QString version)
    {
        version_ = version;
    }
    QByteArray status() {
        return status_.toLocal8Bit();
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

   // template<typename field, typename value>
    ElementBody(QByteArray name, QByteArray data){
        serviceFields_.append({std::move(name), std::move(data)});
    }
    //template<typename field, typename value>
    void operator()(QByteArray name, QByteArray data){
        if (find(name, data) == nullptr) serviceFields_.append({name, data});
    }

    //--------------------------
    QByteArray find(QString name, QByteArray data = nullptr) {

        for (auto it : serviceFields_) {
            if (static_cast<QString>(it.first) == name) {
                if (data != nullptr) it.second = data;
              //qDebug() << "RETURN " << it.second;
                return it.second;
            }
        }
        return nullptr;
    }
private:

    /// Входящий поток данных
    QByteArray requestByte_;
    /// Тело ответа
    BodyResponse body_;
    /// Используемый метод запроса
    QString method_;
    /// тело запроса
    QString target_;
    /// Версия запроса / ответа
    QString version_;
    /// служедные поля запроса / ответа
    ServiceFields serviceFields_;
    /// статус ответа
    QString status_;


};
//----------------------------------------------------

namespace file_handler {

struct STATUS {
    constexpr static  const char* OK                    = "200 OK"; // хорошо
    constexpr static  const char* BAD_RREQUEST          = "400 Bad Request"; // неправильный, некорректный запрос
    constexpr static  const char* NOT_FOUND             = "404 Not Found"; // не найдено
    constexpr static  const char* METHOD_NOT_ALLOWED    = "405 Method Not Allowed"; //метод не поддерживается
    constexpr static  const char* NOT_IMPLEMENTED       = "501 Not Implemented"; //не реализовано
};

const int SIZE_BLOCK = 1024;

class FileHandler{

public:
    ElementBody MakeFileResponse(ElementBody& response, QString pathFile) {
        QString fileName = pathFile.split(("/"), QString::SkipEmptyParts).last();
        auto fileExten = fileName.mid(fileName.lastIndexOf(QRegExp("[.]"), fileName.size())).toLower();
        qDebug() << "MakeFileResponse " << fileExten << fileExten.size() << "CONTA  " << fileName.contains(".");
        if (fileName.contains(".") == true)
        {
            QFile file(pathFile);
            if (file.exists()){
                file.open(QIODevice::ReadOnly);
             //   qDebug() << "EXIST " << pathFile;
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
                response.body("file not found " + pathFile.toLocal8Bit());
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
} // namespase
#endif // FILE_HANDLER_H
