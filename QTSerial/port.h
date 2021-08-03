#ifndef PORT_H
#define PORT_H

#include <QObject>//Qt对象支持消息槽机制
#include <QMutex>//互斥量
#include <QThread>//多线程用
#include "comData.h"

class QByteArray;//前置类声明
class QTimer;
class QMessageBox;
class QSerialPort;//Qt自带的串口类

class Port: public QObject
{
    Q_OBJECT

public:
    explicit Port(QObject *parent = nullptr);//父窗口参数，便于注销管理
    ~Port();

    int startPort(ConfigData &configData);//打开串口，对外接口函数
    void closePort();//关闭端口，对外接口函数


    QString toString(unsigned char*buf, unsigned int bufLen);//转换成字符输出以便调试
    int sendPort(QByteArray& bytes);//消息发送函数
    void procFrameSync(ConfigData& configData);//处理帧同步。划分不同的帧

    QSerialPort *m_serial = nullptr;//串口对象指针

    int sendCount = 0;//发送帧计数
    int recvCount = 0;//接收帧计数

private:
    QWidget* pWidget;//保存的父窗口指针，以便不时之需
    QMutex mutex;//同步操作，用于多线程操作时的互斥

    QByteArray sendData;//发送数据缓存数组
    QByteArray recvData;//接收数据缓存数组

    QTimer* readIntervalTimer; //接收间隔时间定时器,只有接收间断后，才开始上报一帧信息，方便文本类操作

signals:
    void receivedSignal(QByteArray bytes);//自定义的一个信号，当串口收到数据并解析完成后发送


public slots:
    void slotRecvData();//处理接收的槽函数
};



#endif // PORT_H
