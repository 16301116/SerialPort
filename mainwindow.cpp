//AUH 编队项目控制上位机
#if _MSC_VER >= 1600
    //告诉编译器编码方式，防止中文乱码
    #pragma execution_character_set("utf-8")
#endif
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QString>
#include <QTextCodec>
#include <iostream>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QDir>

//#define Encoder_sum 7//编码器返回值位数
QString Encoder_bytes_chars     ="01 03 00 03 00 01 74 0A";//编码器查询
QString RelayOpen_bytes_chars   ="02 0F 00 00 00 04 01 0F 3E 87";//继电器开启
QString RelayClose_bytes_chars  ="02 0F 00 00 00 04 01 00 7E 83";//继电器关闭

uint16_t ModbusCRC16(QByteArray receivedata);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    askDataTimer = new QTimer(this);
    askDataTimer->stop();
    connect(askDataTimer, &QTimer::timeout, this, [=]()
            {
                if (serial->isOpen())
                    askData();
            });
    serial          = new QSerialPort(this);
    uploadDataTimer = new QTimer(this);
    uploadDataTimer->stop();
    serialport_init();
    QObject::connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    ui->btn_getData->setEnabled(false);
    ui->btn_stopGetData->setEnabled(false);
    ui->btn_ClosePort->setEnabled(false);

//    QByteArray test;
//    convertStringToHexString(RelayOpen_bytes_chars,test);
//    int ID = test[0]*256 + test[1];
//    qDebug()<<ID;
    ui->btn_openRelay->setEnabled(false);
    ui->btn_closeRelay->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_ReloadPort_clicked()
{
    serialport_init();
}

void MainWindow::on_btn_OpenPort_clicked()
{
    this->serial->setPortName(ui->comboBox_port->currentText());
    qint32 baudrate = ui->comboBox_baud->currentText().toInt();
    if (serial->open(QIODevice::ReadWrite))
    {
        serial->setBaudRate(baudrate);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);

        ui->btn_OpenPort->setEnabled(false);
        ui->btn_ReloadPort->setEnabled(false);
        ui->btn_ClosePort->setEnabled(true);
        ui->btn_getData->setEnabled(true);
        ui->btn_openRelay->setEnabled(true);

        ui->comboBox_port->setEnabled(false);
        ui->comboBox_baud->setEnabled(false);
    }
    else
    {
        QMessageBox::information(this, "串口", "端口打开失败，请检查");
    }
}

void MainWindow::on_btn_ClosePort_clicked()
{
    serial->close();
    askDataTimer->stop();
    ui->btn_OpenPort->setEnabled(true);
    ui->btn_ReloadPort->setEnabled(true);
    ui->btn_ClosePort->setEnabled(false);
    ui->btn_openRelay->setEnabled(false);
    ui->btn_closeRelay->setEnabled(false);
    ui->btn_getData->setEnabled(false);
    ui->btn_stopGetData->setEnabled(false);
    ui->comboBox_port->setEnabled(true);
    ui->comboBox_baud->setEnabled(true);
}

void MainWindow::on_btn_getData_clicked()
{
    askDataTimer->setInterval(ui->comboBox_timer->currentText().toInt());
    askDataTimer->start();
    ui->btn_getData->setEnabled(false);
    ui->btn_stopGetData->setEnabled(true);
}

void MainWindow::on_btn_stopGetData_clicked()
{
    askDataTimer->stop();
    ui->btn_getData->setEnabled(true);
    ui->btn_stopGetData->setEnabled(false);
}

void MainWindow::on_btn_openRelay_clicked()
{
    Relay_Ctl(1);
//    ui->btn_openRelay->setEnabled(false);
//    ui->btn_closeRelay->setEnabled(true);
}

void MainWindow::on_btn_closeRelay_clicked()
{
    Relay_Ctl(0);
//    ui->btn_openRelay->setEnabled(true);
//    ui->btn_closeRelay->setEnabled(false);
}

void MainWindow::askData()
{
    if(serial->isOpen())
    {
        QByteArray Encoder_bytes;
//        Encoder_bytes.append(QByteArray::fromRawData(Encoder_bytes_chars,8));
        convertStringToHexString(Encoder_bytes_chars,Encoder_bytes);
        this->serial->write(Encoder_bytes);
        this->serial->waitForBytesWritten(2);
        qDebug()<<Encoder_bytes;
    }
    else
    {
        QMessageBox::information(this, "title", "端口打开失败，请先打开串口");
    }
}

void MainWindow::Relay_Ctl(int Order)
{
    if(serial->isOpen())
    {
        switch (Order)
        {
        case 0:
        {
//            while(5)
//            {
                QByteArray RelayClose_bytes;
//                RelayClose_bytes.append(QByteArray::fromRawData(RelayClose_bytes_chars,10));
                convertStringToHexString(RelayClose_bytes_chars,RelayClose_bytes);
                this->serial->write(RelayClose_bytes);
                this->serial->waitForBytesWritten(2);
//            }
                qDebug()<<RelayClose_bytes;
                qDebug("关闭");
            break;
        }
        case 1:
        {
//            while(5)
//            {
                QByteArray RelayOpen_bytes;
//                RelayOpen_bytes.append(QByteArray::fromRawData(RelayOpen_bytes_chars,10));
                convertStringToHexString(RelayOpen_bytes_chars,RelayOpen_bytes);
                this->serial->write(RelayOpen_bytes);
                this->serial->waitForBytesWritten(2);
//            }
                qDebug()<<RelayOpen_bytes;
                qDebug("打开");
            break;
        }
        default:
            break;
        }
    }
    else
    {
        QMessageBox::information(this, "title", "端口打开失败，请先打开串口");
    }
}

void MainWindow::readData()
{
    QByteArray buf;
    QByteArray bufhex;
    QString data = serial->readAll();
    convertStringToHexString(data,buf);
//    qDebug()<<buf;
    bufhex = QByteArray::fromHex(buf);
    if(!buf.isEmpty())
    {
        uint16_t CRCCheckReslut   = ModbusCRC16(buf);
        char     CRCCheck_LowBit  = CRCCheckReslut & 0x00FF;
        char     CRCCheck_HighBig = CRCCheckReslut >> 8;
        if ((CRCCheck_LowBit == buf[buf.size() - 2]) && (CRCCheck_HighBig == buf[buf.size() - 1]))
        {
            int sensorID = buf[0];
            qDebug()<<buf[3];
            qDebug()<<buf[4];
            switch (sensorID)
            {
            case 1:
            {
                Hallvalue = buf[3]*256 + buf[4];
                Deltavalue = Hallvalue - Prevalue;
                if (Deltavalue<-2048)
                {
                    trun_num +=1;
                }
                else if (Deltavalue>2048)
                {
                    trun_num -=1;
                }
                break;
            }
            case 2:
            {
                break;
            }
            }
        }
    }
    buf.clear();
}

void MainWindow::serialport_init()
{
    ui->comboBox_port->clear();
    foreach (const QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        ui->comboBox_port->addItem(info.portName());
    }
}

//Qstring 转为 16进制的函数
void MainWindow::convertStringToHexString(const QString &str, QByteArray &byteData)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    byteData.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = convertCharToHex(hstr);
        lowhexdata = convertCharToHex(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        byteData[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    byteData.resize(hexdatalen);
}

//另一个 函数 char 转为 16进制
char MainWindow::convertCharToHex(char ch)
{
    /*
            0x30等于十进制的48，48也是0的ASCII值，，
            1-9的ASCII值是49-57，，所以某一个值－0x30，，
            就是将字符0-9转换为0-9

            */
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

uint16_t ModbusCRC16(QByteArray receivedata)
{
    int      len  = receivedata.size() - 2;
    uint16_t wcrc = 0XFFFF;    //预置16位crc寄存器，初值全部为1
    uint8_t  temp;             //定义中间变量
    int      i = 0, j = 0;     //定义计数
    for (i = 0; i < len; i++)  //循环计算每个数据
    {
        temp = receivedata.at(i);
        wcrc ^= temp;
        for (j = 0; j < 8; j++)
        {
            //判断右移出的是不是1，如果是1则与多项式进行异或。
            if (wcrc & 0X0001)
            {
                wcrc >>= 1;      //先将数据右移一位
                wcrc ^= 0XA001;  //与上面的多项式进行异或
            }
            else             //如果不是1，则直接移出
                wcrc >>= 1;  //直接移出
        }
    }
    temp = wcrc;  //crc的值
    return wcrc;
}
