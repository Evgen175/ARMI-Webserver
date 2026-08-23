#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QHostAddress addr, uint port, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hostAddressARMI_(addr)
    , port_(port)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("ARMI Webserver"));
    HttpServer_ = new http_server::HttpServer(hostAddressARMI_, port_, this);

    connect(HttpServer_, &http_server::HttpServer::signalMessage, this, &MainWindow::slotReciveMessage);
    if (HttpServer_->isListening()) {
        ui->textOut->append(QStringLiteral("Сервер %1:%2")
                            .arg(hostAddressARMI_.toString())
                            .arg(port_));
    } else {
        ui->textOutErr->append(QStringLiteral("Сервер не запущен"));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::slotReciveMessage(QString messag)
{
    QString color = messag.indexOf("200 OK") == -1 ? "red" : messag.indexOf("/API/FILE") > 0 ? "#5599ff" : messag.indexOf("/API/LOG") > 0 ? "green" : "#000000";
    messag = "<font color=\'" + color +"\'>" + messag.toHtmlEscaped() + "</font>";

    if (color == "red") ui->textOutErr->append(messag);
    else if (color == "#5599ff" || color == "green") ui->textOutFileLog->append(messag);
    else ui->textOut->append(messag);
}

void MainWindow::on_pushButton_clicked()
{
    ui->textOut->clear();
}
