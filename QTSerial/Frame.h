#ifndef FRAME_H
#define FRAME_H


#include <QDateTime>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QtMath>
#include <QDebug>
#include "mainwindow.h"
#include "iostream"
#include "comData.h"
#include "QVector"


enum
{
    X = 0,
    Y = 1,
    Z = 2,
    VEC_XYZ,
};

class Frame
{


public:
   QString originData;      //接收到的原始数据
   QString formData;        //处理后的数据

   //原始帧数据的缓存区
   QByteArray originBuffer;

   quint64 sysTimeMs;
   quint64 frameTimeMs;

   QString currentTime;
   //惯性输出的FrameID为0-OXFFFF
   quint64 FrameID;
   //程序中记录一下每超过一轮左移一位
   quint32 FrameIDoverLoad;


   qreal Groy[VEC_XYZ];
   qreal Acc[VEC_XYZ];
   qreal temperature;

};


//全局函数
void dataReceiveAnlTask(void);
int dataReceivePrepare(quint8 data);
void dataReceiveAnl(Frame &frameTemp);

QString getCurrentTime();   //获取当前系统时间 xx-xx-xx
QString getCurrentTimeMs(); //获取当前时间毫秒 基于系统数据
qint64 getTimeStamp();    //获得当前的时间戳 基于IMU数据
quint64 getFrameTimeStamp(); //根据IMU时间戳数据获得的时间戳

//保存数据
void SaveFrameData();

//全局变量定义处

//检测是否为一帧
extern bool frameOK;
//帧丢失计数器
extern quint64 frameLossCount;
extern quint64 frameCount;
//系统启动时间毫秒
extern quint64 sysStartTimeMs;
//系统接收帧的时间
extern quint64 sysCurrentTime;

extern  QVector<Frame> vframe;
#endif // FRAME_H
