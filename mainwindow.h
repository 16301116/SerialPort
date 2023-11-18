#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <QSerialPort>
#include <QSettings>
#include <QTimer>
#include <QFileInfo>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QSerialPort *serial;
    QTimer      *askDataTimer;
    QTimer      *uploadDataTimer;
    int         Order = 1;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void askData();
    void serialport_init();
    void readData();
    void Relay_Ctl(int);
    void convertStringToHexString(const QString &str, QByteArray &byteData);
    char convertCharToHex(char ch);
    int  Hallvalue  = 0;
    int  Prevalue   = 0;
    int  Deltavalue = 0;
    int  trun_num   = 0;

private slots:
    void on_btn_ReloadPort_clicked();

    void on_btn_OpenPort_clicked();

    void on_btn_ClosePort_clicked();

    void on_btn_getData_clicked();

    void on_btn_stopGetData_clicked();

    void on_btn_openRelay_clicked();

    void on_btn_closeRelay_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
