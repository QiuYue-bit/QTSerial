#include "port.h"
#include <QDebug>//输出调试信息
#include <QSerialPort>//使用Qt自带的串口类
#include <QMessageBox>
#include <QTimer>
#include "mainwindow.h"
#include "Frame.h"

Port::Port(QObject *parent)
{

    pWidget =  (QWidget*) parent;//保存父窗口指针

    m_serial = new QSerialPort(parent);//串口对象


    /*
     * connect 1.信号发出者地址 2.发什么信号 3.在哪个类触发(地址) 4.槽函数
    */

    //关联串口的读取信号
    connect(m_serial, &QSerialPort::readyRead, this, &Port::slotRecvData);

    //连接自定义的接收后通知处理的信号和槽，采用队列缓存机制
    connect(this, SIGNAL(receivedSignal(QByteArray)),pWidget, SLOT(parseMsg(QByteArray)),Qt::QueuedConnection);


}


Port::~Port()
{
       closePort();//关闭串口
}

/**
函数功能：将帧转换成字符串
输入输出：unsigned char*buf 帧缓存数据, unsigned int bufLen 帧长度
返回值：QString 转换后的字符串
说明：方便打印调试
**/
QString Port::toString(unsigned char*buf, unsigned int bufLen)
{
    QString str("");
    str += QString("Len %1,").arg(bufLen);//报文的长度
    unsigned int i = 0;
    for ( ; i < bufLen; ++i)//打印报文体
        str += QString(" %1").arg((unsigned char)buf[i], 2, 16, QLatin1Char('0'));//按照16进制输出，两位不足时前面补零
    return str;
}

/**
函数功能：打开端口
输入输出：ConfigData &configData 配置参数
返回值：int 0-成功，-1参数错误，-2打开失败
说明：对外接口函数
**/
int Port::startPort(ConfigData &configData)
{
    recvData.clear();//清除接收缓存
    sendCount = 0;//发送帧计数
    recvCount = 0;//接收帧计数

    m_serial->setPortName(configData.name);//设定端口号
    m_serial->setBaudRate(configData.baudRate);//设定波特率
    m_serial->setDataBits(configData.dataBits);//设置数据位
    m_serial->setParity(configData.parity);//设置校验位
    m_serial->setStopBits(configData.stopBits);//设定停止位
    m_serial->setFlowControl(configData.flowControl);//关闭流控

    if(m_serial->isOpen())
    {
        //判断是否处于打开状态，可避免出现二次关闭
        m_serial->close();//关闭端口
        qDebug()<<"serial close is called";//调试打印用
    }

    if (!m_serial->open(QIODevice::ReadWrite))
    {
        //读写方式打开串口
        qDebug()<<tr("Can't open %1, error code %2,%3").arg(m_serial->portName()).arg(m_serial->error()).arg(m_serial->errorString());//Port类只打印调试信息
        return -1;//发送错误给协议层，用户提示，由协议层弹框显示
    }else {
        qDebug()<<"serial is opened true";
        m_serial->flush();//刷掉上次的数据
        return 0;
    }
}

///函数功能：关闭串口
void Port::closePort()
{
    if(m_serial != nullptr)//是否为空，避免空指针的使用
        m_serial->close();
}

/**
函数功能：消息发送
输入输出：QByteArray& bytes 待发数据
返回值：int 0-成功，-1-失败
说明：
**/
int Port::sendPort(QByteArray& bytes)
{
    if(m_serial->isOpen())
    {
        //m_serial->flush();//编程参考：若采用应答方式，这里可以清除接收区,防止干扰导致接收到杂乱数据
        //recvData.clear();
        //QByteArray buf = m_serial->readAll();//清空
        int count = m_serial->write(bytes);//调用qt串口发送函数
        sendCount += count;//发送报文计数
        qDebug()<<"sendPort:"<<count<<bytes;
    }
    else {
        qDebug()<<"Can't Write, Port is Not Open!";//底层只在调试框打印消息，用户提示放到上层完成
        return -1;//返回
    }
    return 0;
}


/**
函数功能:读串口
输入输出:
返回值:void
说明:槽函数
**/
void Port::slotRecvData()
{

    //获取第一次的系统启动时间
    if(!sysStartTimeMs)
    {
        sysStartTimeMs = getTimeStamp();
        //空读一次
        m_serial->readAll();
        return;
    }


    QByteArray buf = m_serial->readAll();//将已接收数据全部存储
    if(!buf.isEmpty())//若数据不为空
    {
        mutex.lock();//由于接收数据会被两个线程操作，因此需要加互斥锁//在所有操作此全局变量的地方都要加锁
        recvData.append(buf);//附加数据到recvData接收缓存中
        recvCount += buf.size();

        procFrameSync(configData);//帧同步处理
        mutex.unlock();//解锁
    }
    mainWindow->showRecvCount(recvCount);

}

/**
函数功能：帧同步处理
输入输出：ConfigData& configData 帧同步的配置
返回值：void
说明：
**/
void Port::procFrameSync(ConfigData& configData)
{
    QByteArray *pPortRecv = &recvData;

    do{//循环执行，可解决一次来多帧的情况
        //剔除开头增加帧头SOF时的处理，可方便解析并增强容错
        int index=0;//起点字节位置
        for(index=0; index<pPortRecv->length(); index++)//查找是否有SOF起点
        {
            unsigned char sof = pPortRecv->at(index);//取第index个数据
            if((unsigned char)configData.headFixHead == sof)//
                break;
        }
        if(0 != index)//起点SOF不在第一个位置上或者没有找到
        {

            pPortRecv->remove(0,index);//删除前面不是起点的部分
        }

        if(pPortRecv->length() >= int(configData.headFixLen))//判断长度是否满足要求
        {
            emit receivedSignal(pPortRecv->mid(0,configData.headFixLen));//发送接收到的信号,缓存并由主窗口进行二次处理
            pPortRecv->remove(0, configData.headFixLen);//在缓冲区中删除掉本帧数据
        }
    } while(pPortRecv->length() >= int(configData.headFixLen));//接收数据长度大于等于最小头部长度



    //    while(pPortRecv->length() >= int(configData.headFixLen))
    //    {
    //        int index=0;//起点字节位置

    //        //在一帧数据中寻找
    //        for(index=0; index<int(configData.headFixLen); index++)
    //        {
    //            unsigned char sof = pPortRecv->at(index);//取第index个数据
    //            if((unsigned char)configData.headFixHead == sof)
    //            {
    //                int result = 0;

    //                //空调用两次 确保state是从零开始的
    //                dataReceivePrepare(0x00);
    //                dataReceivePrepare(0x00);
    //                for(uint8_t i=0;i<int(configData.headFixLen);i++)
    //                    result = dataReceivePrepare(pPortRecv->at(index+i));
    //                if(result)
    //                    break;
    //            }
    //        }

    //        if(0 != index)//起点SOF不在第一个位置上或者没有找到
    //        {
    //            frameLossCount++;
    //            pPortRecv->remove(0,index);//删除前面不是起点的部分
    //        }

    //        if(pPortRecv->length() >= int(configData.headFixLen))//判断长度是否满足要求
    //        {
    //            emit receivedSignal(pPortRecv->mid(0,configData.headFixLen));//发送接收到的信号,缓存并由主窗口进行二次处理
    //            pPortRecv->remove(0, configData.headFixLen);//在缓冲区中删除掉本帧数据
    //            frameCount++;
    //            mainWindow->showRecvFrameCount(frameCount);
    //        }

    //    }

    //    while(pPortRecv->length() >= int(configData.headFixLen))
    //    {
    //        int index=0;//起点字节位置
    //        for(index=0; index<pPortRecv->length(); index++)//查找是否有SOF起点
    //        {
    //            unsigned char sof = pPortRecv->at(index);//取第index个数据
    //            if((unsigned char)configData.headFixHead == sof)//
    //                break;
    //        }
    //        if(0 != index)//起点SOF不在第一个位置上或者没有找到
    //        {
    //            frameLossCount++;
    //            pPortRecv->remove(0,index);//删除前面不是起点的部分
    //        }

    //        if(pPortRecv->length() >= int(configData.headFixLen))//判断长度是否满足要求
    //        {
    //            emit receivedSignal(pPortRecv->mid(0,configData.headFixLen));//发送接收到的信号,缓存并由主窗口进行二次处理
    //            pPortRecv->remove(0, configData.headFixLen);//在缓冲区中删除掉本帧数据
    //            frameCount++;
    //            mainWindow->showRecvFrameCount(frameCount);
    //        }
    //    }

}
