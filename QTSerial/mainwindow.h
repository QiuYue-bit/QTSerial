#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


/* -----------------------------------------------
 *          User include
 *  ---------------------------------------------*/
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QtMath>
#include <QDateTime>
#include <QFileDialog>//文件对话框类
#include <QTextBlock>//文本块设置
#include <QTextEdit>
//user
#include "port.h"


//debug
#include <QDebug>
//data process
#include <Frame.h>
#include "comData.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /*
     *  自定义函数
     */

    // UI界面显示
    void fetchNewConfig(ConfigData &configData);//获取最新页面数据，并跟新全局setting配置
    void showRecvFrameCount(int count);//显示接收帧计数
    void showRecvCount(int count);//显示接收字节计数






    void setSendData(QString& str,bool isHex);//将该数据，设置为发送数据
    void sendMsg(QString& msg);//发送数据

public slots:
    //接收后，进行数据解析
    void parseMsg(QByteArray recvData);

private slots:


    //按钮和CheckBox
    void on_btnFillAvailablePorts_clicked();//刷新本机串口
    void on_btnSend_clicked();//发送按钮

    void on_btnClear_clicked();//清空接收数据框
    void on_btnOpenClose_clicked();
    void on_ckbHexDisplay_clicked(bool checked);//十六进制显示
    void on_ckbSave2File_clicked();//保存到文件
    //


private:
    Ui::MainWindow *ui;

    Port* port = nullptr;//接口层函数

};

extern MainWindow* mainWindow;//主窗口的指针

#endif // MAINWINDOW_H
