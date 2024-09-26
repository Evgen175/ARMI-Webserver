#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "http_server.h"
#include "request_handler.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QHostAddress addr, uint port, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
        void slotReciveMessage(QString messag);

private slots:
        void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    http_server::HttpServer *HttpServer_;
    QHostAddress hostAddressARMI_;
    uint port_;
};
#endif // MAINWINDOW_H
