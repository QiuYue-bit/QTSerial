#include "Frame.h"
ConfigData configData;//定义全局的一个配置数据

//全局变量
QVector<Frame> vframe;
quint64 sysStartTimeMs;
bool frameOK;
quint64 frameLossCount = 0;
quint64 frameCount = 0;

static quint8 DataBuffer[32];


int dataReceivePrepare(quint8 data)
{
    static quint8 state = 0;
    quint8 sumCheck=0;
    bool dataOk = 0;
    if(state == 0 && data == 0x55)
    {
        DataBuffer[state] = data;
        state++;
    }
    else if (state==1 && data == 0xa4)
    {
        DataBuffer[state] = data;
        state++;
    }
    else if (state==2 && data == 0x1c)
    {
        DataBuffer[state] = data;
        state++;
    }
    // 数据state = 30的时候存的是温度
    else if(state < 31)
    {
        DataBuffer[state] = data;
        state++;
    }
    // 和校验位
    else if(state == 31)
    {
        for(quint8 i=2;i<=30;i++)
            sumCheck+=DataBuffer[i];
        DataBuffer[state] = data;
        state++;
    }
    //备用字节
    else if(state == 32 && data==0x00)
    {
        DataBuffer[state] = 0x00;
        //如果最后一个字节也是对的，那就保存数据并解析
        if(sumCheck == DataBuffer[state-1])
        {
            dataOk=1;
            Frame frametemp;

            //原始数据赋值
            for(int i =0;i<33;i++)
                frametemp.originBuffer[i]=DataBuffer[i];

            //对帧数据进行处理
            dataReceiveAnl(frametemp);
            //记录该帧
            vframe.push_back(frametemp);
            return 1;
        }
        else
        {
            return 0;
            dataOk=0;
        }
        //一帧
        state=0;
    }
    else
    {
        state=0;
        return 0;
    }


    return 0;

}


void dataReceiveAnl(Frame &frameTemp)
{


    QByteArray data_buf = frameTemp.originBuffer;

    /*
     *      解析数据
     */

    quint8 BYTE0,BYTE1,BYTE2,BYTE3;
    qint32 gryTemp[VEC_XYZ];
    qint32 accTemp[VEC_XYZ];
    qint16 temp;

    /*            FrameID          */
    BYTE0 = data_buf[3];
    BYTE1 = data_buf[4];
    frameTemp.FrameID =BYTE1<<8 |BYTE0 ;


    /*            GRY          */
    BYTE0 = data_buf[5];
    BYTE1 = data_buf[6];
    BYTE2 = data_buf[7];
    BYTE3 = data_buf[8];
    gryTemp[X] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Groy[X] = (qreal)gryTemp[X]/2000;

    BYTE0 = data_buf[9];
    BYTE1 = data_buf[10];
    BYTE2 = data_buf[11];
    BYTE3 = data_buf[12];
    gryTemp[Y] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Groy[Y] = (qreal)gryTemp[Y]/2000;

    BYTE0 = data_buf[13];
    BYTE1 = data_buf[14];
    BYTE2 = data_buf[15];
    BYTE3 = data_buf[16];
    gryTemp[Z] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Groy[Z] = (qreal)gryTemp[Z]/2000;


    /*            ACC          */
    BYTE0 = data_buf[17];
    BYTE1 = data_buf[18];
    BYTE2 = data_buf[19];
    BYTE3 = data_buf[20];
    accTemp[X] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Acc[X] = (qreal)accTemp[X]/1024;

    BYTE0 = data_buf[21];
    BYTE1 = data_buf[22];
    BYTE2 = data_buf[23];
    BYTE3 = data_buf[24];
    accTemp[Y] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Acc[Y] = (qreal)accTemp[Y]/1024;

    BYTE0 = data_buf[25];
    BYTE1 = data_buf[26];
    BYTE2 = data_buf[27];
    BYTE3 = data_buf[28];
    accTemp[Z] =((BYTE3<<24)|(BYTE2<<16)|(BYTE1<<8)|(BYTE0));
    //单位转换 °/s
    frameTemp.Acc[Z] = (qreal)accTemp[Z]/1024;

    // TEMP
    BYTE0 = data_buf[29];
    BYTE1 = data_buf[30];
    temp =BYTE1<<8 |BYTE0 ;
    //单位转换 °/s
    frameTemp.temperature = (qreal)temp /10;


    /*
     *  FrameID部分测试
     */
    if(vframe.size() >= 1)
    {
        qint64 frameIdDiffer = 0;
        quint64 currentID = frameTemp.FrameID;
        quint64 lastID = vframe.back().FrameID;
        frameIdDiffer =currentID-lastID;

        if(frameIdDiffer > 0)
        {
            frameCount+=frameIdDiffer;
            frameLossCount+=frameIdDiffer-1;
        }
        else
        {
            frameLossCount+=frameIdDiffer+65536-1;
            frameCount+=frameIdDiffer+65536;
        }

        mainWindow->showRecvFrameCount(frameCount);
    }

    /*
     *      保存原始数据与解析数据
     *  解析后的数据格式 系统时间hh:mm:MM 系统时间ms 时间戳时间ms frameID gry_x gry_y gry_z acc_x acc_y acc_z temprature
     *  原始数据格式 系统时间ms buff
     */


    //原始数据
    frameTemp.originData =QString("");
    QString str("");
    for(uint8_t i =0;i<33;i++)
        str+=QString(" %1").arg((unsigned char)data_buf[i], 2, 16, QLatin1Char('0'));//按照16进制输出，两位不足时前面补零
    frameTemp.originData+=frameTemp.currentTime + QString(' ')+QString::number(frameTemp.sysTimeMs) +QString(' ')+str;


    //解析后的数据
    frameTemp.formData = QString("");
    frameTemp.formData += \
            frameTemp.currentTime + QString(' ')\
            +  QString::number(frameTemp.sysTimeMs) +QString(' ') \
            +  QString::number(frameTemp.frameTimeMs) +QString(' ')\
//            + QString::number(frameTemp.frameTimeMs) +QString(' ')
            + QString::number(frameTemp.FrameID) +QString(' ') \
            + QString::number(frameTemp.Groy[X],'f',6) +QString(' ') \
            + QString::number(frameTemp.Groy[Y],'f',6) +QString(' ') \
            + QString::number(frameTemp.Groy[Z],'f',6) +QString(' ') \
            + QString::number(frameTemp.Acc[X],'f',6) +QString(' ') \
            + QString::number(frameTemp.Acc[Y],'f',6) +QString(' ') \
            + QString::number(frameTemp.Acc[Z],'f',6) +QString(' ') \
            + QString::number(frameTemp.temperature,'f',6)\
            ;

        qint64 differ=frameTemp.sysTimeMs*10 - frameTemp.frameTimeMs;

        qDebug()<<"frameLossCount:"<<frameLossCount<<"differ"<<differ;
}

QString getCurrentTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    return dateTime.toString("HH:mm:ss.zzz");

}
/*******************获得当前时间************************************/
QString getCurrentTimeMs()
{
    QString timestamp;
    QDateTime dateTime = QDateTime::currentDateTime();
    timestamp = dateTime.toString("HH:mm:ss.zzz -> ");
    return timestamp;
}

/*******************获得时间戳 （毫秒）************************************/
qint64 getTimeStamp()
{
    QDate dt;
    QDateTime dateTime = QDateTime::currentDateTime();
    QDateTime today;
    today.setDate(dt.currentDate());
    return dateTime.toMSecsSinceEpoch() - today.toMSecsSinceEpoch();
}

/*******************基于IMU时间戳的时间 （毫秒）************************************/
quint64 getFrameTimeStamp()
{
    //400HZ
    return sysStartTimeMs*10+frameCount*25;
}
/*******************保存数据************************************/
void SaveFrameData()
{
//    qDebug()<<"on_ckbSave2File_clicked";

//    QDateTime now = QDateTime::currentDateTime();//当前时间
//    QString def = now.toString("yyyy-MM-dd.hh.mm") + ".txt";
//    QString fileName = QFileDialog::getSaveFileName(this,tr("Please input a file name"),def,tr("text(*.txt)"));//save函数类似另存为，可输入新的文件名

//    //参数说明：this代表此对话框作为父窗口，第二个参数是对话框的标题，第三个参数代表默认显示的文件名和路径（只写路径时无文件名），第四个参数是过滤器，其中的*代表任意匹配不能漏了
//    if(fileName.isEmpty())//判断是否为空
//    {
//        qDebug()<<tr("FileName is Null, Do Nothing!");//调试框内输出
//        return;//不进行任何改变
//    }

//    QFile recordFile;//Qt的文件对象
//    QTextStream textStream;//Qt文本文件流
//    recordFile.setFileName(fileName);//设定文件名
//    qDebug()<<tr("save File Name：")<<fileName;//调试文件名

//    if (recordFile.open(QIODevice::Text|QFile::WriteOnly|QFile::Truncate))//Append//以只写、附加方式打开Text文件
//    {
//        textStream.setDevice(&recordFile);//设定文本流对象
//        //textStream.setCodec(QTextCodec::codecForName("GB18030"));//读取txt文件时，使用此语句，可避免出现中文乱码。
//        //写文件时，使用QString::fromUtf8()将中文包起来可避免乱码
//        QString content = ui->plainTextEdit->toPlainText();
//        textStream<<content<<endl;
//        textStream.flush();//写入硬盘
//    }
//    recordFile.close();//关闭文件//可多次关闭
}
