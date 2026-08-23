#include "mainwindow.h"

#include <QApplication>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QMessageBox>



QString pathData_;
QString pathArmXML_;



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Run programm" << argc << argv[0];

    QHostAddress hostAddressARMI = QHostAddress::Any;
    uint port = 8080;

    pathData_ = argc > 1 ? QString::fromLocal8Bit(argv[1])
                         : QStringLiteral("/home/astra/UKK_29M_Rus/UKK_LS_29M_Common/Data");
    if (!QDir(pathData_).exists()){
        QMessageBox::critical(nullptr, QStringLiteral("ARMI Webserver"),
                              QStringLiteral("Путь не найден:\n") + pathData_);
        return 1;
    }
    pathArmXML_ = argc > 2 ? QString::fromLocal8Bit(argv[2])
                           : QApplication::applicationDirPath();
    if (argc > 3) {
        hostAddressARMI = QHostAddress(QString::fromLocal8Bit(argv[3]));
        if (hostAddressARMI.isNull()) {
            hostAddressARMI = QHostAddress::Any;
        }
    }
    if (argc > 4) {
        bool ok = false;
        const uint parsed = QString::fromLocal8Bit(argv[4]).toUInt(&ok);
        if (ok && parsed > 0 && parsed < 65536) {
            port = parsed;
        }
    }

    qDebug() << pathData_ << "\n" << pathArmXML_ << hostAddressARMI << port;

    MainWindow w(hostAddressARMI, port);
    w.show();
    return a.exec();
}
