/*
 * 程序共用的全局变量及数据
*/
#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QSerialPort>//串口有关的类型
#include <QVector>

///协议解析用的格式
typedef struct {
  unsigned int length;//字段长度，单位byte
  unsigned int type;//字段类型，其中：0-char,1-uchar, 2-short,3-ushort, 4-int,5-uint,6-float
  QByteArray data;//该字段的数据
}Format;




/*-----
功能: 配置数据
说明: 端口配置数据，可保持到配置文件中
-----*/
typedef struct {


    QString name="COM1";//串口端口名称
    qint32 baudRate=9600;//串口波特率
    QSerialPort::DataBits dataBits=QSerialPort::Data8;//数据位数//实际使用的类型，和string开头的对应，可方便使用
    QSerialPort::Parity parity=QSerialPort::Parity::EvenParity; //偶校验
    QSerialPort::StopBits stopBits=QSerialPort::OneStop;//停止位1位
    QSerialPort::FlowControl flowControl=QSerialPort::NoFlowControl;//流量控制 无


    /*RECV*/
    bool isHexDisplay=false;//是否十六进制显示

    /*CONTENT*/
    bool isRecvProtocol=false;//是否使用接收协议
    bool isHexSend=false;//是否使用十六进制发送


    /*FRAMESYNC*/
    unsigned char frameSyncType=0;//帧同步类型：0-超时，1-帧头帧尾，2-定长段帧, 3-帧头加定长
    unsigned short intervalTime=40;//帧间隔时间
    unsigned char frameHead=0x02;//帧头标识符
    unsigned char frameTail=0x03;//帧尾标识符
    int fixFrameLen=2;//固定帧长度
    unsigned char headFixHead=0x02;//帧头加定长的帧头
    int headFixLen=2;//帧头加定长的帧尾
    //format
    QVector<Format> formatSend;//存储发送格式
    QVector<Format> formatRecv;//存储接收格式
    QByteArray geometry;//主窗口的几何尺寸

}ConfigData;

extern ConfigData configData;//声明extern以方便引用此头文件的地方，都可直接使用此全局变量




#endif // GLOBAL_H
