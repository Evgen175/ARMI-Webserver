#include "mainwindow.h"

#include <QApplication>
#include <QString>
#include <QDebug>



QString pathData_;
QString pathArmXML_;
//QString slash_;



using namespace std::literals;

namespace {
template<typename Fn>
void RunWorker(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::thread> workers;
    workers.reserve(n - 1);
    while (--n) {
        workers.emplace_back(fn);
    }
}
} //namespace


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   // http_server::Logger logger_ = new http_server::Logger;
    qDebug() << "Run programm" << argc << "SDFSD"  << argv[0];
   // logger_( "Run programm");
    const unsigned num_thread = std::thread::hardware_concurrency();
 //   net::io_context ioc(num_thread);
    qDebug() << "Количество потоков = " << num_thread;

//    net::signal_set signals_(ioc, SIGINT, SIGTERM);
//    signals_.async_wait([&ioc](const boost::system::error_code& ec, [[maybe_unused]] int signal_number){
//        if (!ec) {
//            qDebug() << "Сервер остановлен";
//            ioc.stop();
//        }
//    });



//#ifdef Q_OS_LINUX
//  slash_ = "/";
//#else
//    qDebug() << "Windows version";
//    slash_ = "\\";
//#endif


    QHostAddress hostAddressARMI = QHostAddress::Any; // прослушивание IP адресов
    uint    port = 8080;



    pathData_ = argv[1] == nullptr ? "/home/astra/UKK_29M_Rus/UKK_LS_29M_Common/Data" : argv[1];
    if (!QDir(argv[1]).exists()){
        qDebug() << "Путь не найден " << argv[1];
        exit(1);
    }
    pathArmXML_ = argc == 1 ? QApplication::applicationDirPath() : argv[2];


    qDebug() << pathData_ << "\n" << pathArmXML_;

    auto api_strand = std::make_shared<http_handler::RequestHandler>();
  //  RunWorker(std::max(1u, num_thread), [&ioc]{
       // ioc.run();
  //  });
    MainWindow w(hostAddressARMI, port);
    w.show();
    return a.exec();
}
