#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Frame.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //模拟一下按键输入
    on_btnFillAvailablePorts_clicked();
    port = new Port(this);//创建端口层

    //Connect


}

MainWindow::~MainWindow()
{
    delete ui;
}


/* ---------------------------------------------------------------------------------------------------
 *            UI Fuctions
 * --------------------------------------------------------------------------------------------------*/



/**
 * @brief MainWindow::fetchNewConfig
 * @param configData
 * 获取最新页面数据，并跟新全局setting配置
 */
void MainWindow::fetchNewConfig(ConfigData &configData)
{
    configData.name = ui->cbSerialPort->currentText().trimmed();//获取端口号
    configData.baudRate = ui->cbBaudRate->currentText().trimmed().toInt();//获取波特率

    //这部分写死
    configData.parity =QSerialPort::Parity::EvenParity;
    configData.dataBits =QSerialPort::DataBits::Data8;
    configData.stopBits =QSerialPort::StopBits::OneStop;

    configData.isHexDisplay= ui->ckbHexDisplay->isChecked();//是否十六进制

    //帧头
    configData.headFixLen = 33;
    configData.headFixHead = 0x55;


}


/**
 * @brief MainWindow::showRecvFrameCount
 * @param count
 * 显示接收帧计数
 */
void MainWindow::showRecvFrameCount(int count)
{
    ui->lbFrameCount->setText(QString::number(count));//显示接收帧计数
}



/**
 * @brief MainWindow::showRecvCount
 * @param count
 * 显示接收字节计数
 * Done
 */
void MainWindow::showRecvCount(int count)
{
    ui->lbRecvCount->setText(QString::number(count));//显示接收字节计数
}




/* ---------------------------------------------------------------------------------------------------
 *            slots
 * --------------------------------------------------------------------------------------------------*/

/**
 * @brief MainWindow::on_btnOpenClose_clicked
 * @param checked
 * Done
 */
void MainWindow::on_btnOpenClose_clicked()
{

    qDebug()<<"Open is clicked";

    //open打开串口
    if(!port->m_serial->isOpen())
    {
        //刷新串口配置信息
        fetchNewConfig(configData);

        //串口打开失败
        if(0!= port->startPort(configData))//调用协议层接口函数打开端口
        {
            ui->lbStatus->setText(QString("Open Failed"));
        }
        else
        {
            ui->btnOpenClose->setText("关闭");//界面的按钮调整状态
            ui->btnSend->setEnabled(true);//允许发送
            ui->lbStatus->setText(QString("connected"));
        }

    }else//close关闭串口
    {
        ui->btnOpenClose->setText("打开");//界面的按钮调整状态
        port->closePort();//调用协议层接口函数打开端口
        ui->lbStatus->setText(QString("disconnected"));
    }
}

/**
 * @brief MainWindow::on_btnFillAvailablePorts_clicked
 * 刷新本机串口
 * Done
 */
void MainWindow::on_btnFillAvailablePorts_clicked()
{
    qDebug()<<"on_btnFillAvailablePorts_clicked";
    ui->cbSerialPort->clear();
    foreach(const QSerialPortInfo info,QSerialPortInfo::availablePorts()) // 获取可用串口
    {
        ui->cbSerialPort->addItem(info.portName());  // 将端口名显示出来
    }
}


/**
 * @brief MainWindow::on_ckbSave2File_clicked
 * 保存到文件
 * unDone
 */
void MainWindow::on_ckbSave2File_clicked()
{
    qDebug()<<"on_ckbSave2File_clicked";

    QDateTime now = QDateTime::currentDateTime();//当前时间
    QString def = now.toString("yyyy-MM-dd.hh.mm") + ".txt";
    QString fileName = QFileDialog::getSaveFileName(this,tr("Please input a file name"),def,tr("text(*.txt)"));//save函数类似另存为，可输入新的文件名


    /*
     *      保存解析后的数据
     */
    //参数说明：this代表此对话框作为父窗口，第二个参数是对话框的标题，第三个参数代表默认显示的文件名和路径（只写路径时无文件名），第四个参数是过滤器，其中的*代表任意匹配不能漏了
    if(fileName.isEmpty())//判断是否为空
    {
        qDebug()<<tr("FileName is Null, Do Nothing!");//调试框内输出
        return;//不进行任何改变
    }

    QFile recordFile;//Qt的文件对象
    QTextStream textStream;//Qt文本文件流
    recordFile.setFileName(fileName);//设定文件名
    qDebug()<<tr("save File Name：")<<fileName;//调试文件名

    if (recordFile.open(QIODevice::Text|QFile::WriteOnly|QFile::Truncate))//Append//以只写、附加方式打开Text文件
    {
        textStream.setDevice(&recordFile);//设定文本流对象
        //textStream.setCodec(QTextCodec::codecForName("GB18030"));//读取txt文件时，使用此语句，可避免出现中文乱码。
        //写文件时，使用QString::fromUtf8()将中文包起来可避免乱码
        for(auto frame:vframe)
        {
            QString content = frame.formData;
            textStream<<content<<endl;
            textStream.flush();//写入硬盘
        }

    }
    recordFile.close();//关闭文件//可多次关闭

    /*
     *      保存原始数据
     */

    //参数说明：this代表此对话框作为父窗口，第二个参数是对话框的标题，第三个参数代表默认显示的文件名和路径（只写路径时无文件名），第四个参数是过滤器，其中的*代表任意匹配不能漏了
    if(fileName.isEmpty())//判断是否为空
    {
        qDebug()<<tr("FileName is Null, Do Nothing!");//调试框内输出
        return;//不进行任何改变
    }

    QFile recordFile1;//Qt的文件对象
    QTextStream textStream1;//Qt文本文件流
    QString defOrigin =now.toString("yyyy-MM-dd.hh.mm") +"_origin"+ ".txt";
    QString fileNameOrigin = "C:/Users/Diven/Desktop/"+defOrigin;
    recordFile1.setFileName(fileNameOrigin);//设定文件名
    qDebug()<<tr("save File Name：")<<fileNameOrigin;//调试文件名

    if (recordFile1.open(QIODevice::Text|QFile::WriteOnly|QFile::Truncate))//Append//以只写、附加方式打开Text文件
    {
        textStream1.setDevice(&recordFile1);//设定文本流对象
        //写文件时，使用QString::fromUtf8()将中文包起来可避免乱码
        for(auto frame:vframe)
        {
            QString content = frame.originData;
            textStream1<<content<<endl;
            textStream1.flush();//写入硬盘
        }

    }
    recordFile1.close();//关闭文件//可多次关闭


}



/**
 * @brief MainWindow::on_btnClear_clicked
 * 清除窗口
 * Done
 */
void MainWindow::on_btnClear_clicked()
{
    qDebug()<<"on_btnClear_clicked:";
    ui->plainTextEdit->clear();
}


/**
 * @brief MainWindow::on_btnSend_clicked
 * 发送按钮
 * Done
 */
void MainWindow::on_btnSend_clicked()
{
    QString data = ui->lintSendData->text();
    qDebug()<<"on_btnSend_clicked:"<<data;
    QByteArray array = data.toLatin1();
    port->sendPort(array);
}

/**
 * @brief MainWindow::on_ckbHexDisplay_clicked
 * @param 十六进制显示
 * unDone
 */
void MainWindow::on_ckbHexDisplay_clicked(bool checked)
{
    qDebug()<<"on_ckbHexDisplay_clicked";
    QString info;
    QTextDocument* doc = ui->plainTextEdit->document();//获取文本对象
    int cnt = doc->blockCount();
    if(0 == cnt)
        return;
    QString line;
    QString strTime;

    if(checked)//如果处于选中状态，则将当前数据转换成十六进制字符串并显示
    {
        for(int i=0; i<cnt; i++)//由于有可能有时间字段，所以要分段落处理数据
        {
            line = doc->findBlockByNumber(i).text();//获取一段数据
            if(line.indexOf('[') == 0)//带有时间标签，需要去除
            {
                strTime = line.mid(0,10);//时间标签为10位字符
                line = line.mid(10);//[12:20:02]
            }

            info += strTime;//继续保留时间标签
            info += line.toLocal8Bit().toHex(' ');//将数据部分转换成十六进制字符串
            if(i!=(cnt-1))//不是最后一行时，需要添加原有的换行符
                info += QString('\n');//原先就应该带有换行符，findBlockByNumber.text()可能把它去掉了，这里把它加上去。
        }
    }else//如果处于取消选中，则将恢复原有数据
    {
        QByteArray bytes;
        for(int i=0; i<cnt; i++)
        {
            line = doc->findBlockByNumber(i).text();
            if(line.indexOf('[') == 0)//带有时间标签，需要去除
            {
                strTime = line.mid(0,10);//获取时间标签，为10位字符
                line = line.mid(10);//[12:20:02]
            }

            info += strTime;//先添加时间标签
            bytes = bytes.fromHex(line.toLatin1());//调用fromHex函数，从十六进制字符串中恢复显示
            info+=bytes;

            if(i!=(cnt-1))//不是最后一行时，需要添加原有的换行符
                info += QString('\n');//添加换行符
        }
    }

    ui->plainTextEdit->setPlainText(info);
}

/**
 * @brief 解析消息
 * @param recvData 接收数据
 */
void MainWindow::parseMsg(QByteArray recvData)
{

    Frame frametemp;


    //赋值时间
    frametemp.originBuffer = recvData;
    frametemp.sysTimeMs =getTimeStamp();
    frametemp.currentTime = getCurrentTime();
    frametemp.frameTimeMs = getFrameTimeStamp();

    //数据分析并打包
    dataReceiveAnl(frametemp);

    //在ui中打印数据信息
    if(frametemp.FrameID %100 ==0)
    {
        QString recvStr(frametemp.formData);
        ui->plainTextEdit->moveCursor(QTextCursor::End);//将光标移动到末尾
        recvStr+='\n';
        ui->plainTextEdit->insertPlainText(recvStr);//使用append会导致换行
    }


    //将当前帧保存到Vector中
    vframe.push_back(frametemp);

//    QString recvStr(recvData);//将接收到的ByteArray转换成字符串，以便输出
//    QString recvInfo;

//    if(ui->ckbShowTime->isChecked())//是否选中了“显示时间”选项
//    {
//        QTime now = QTime::currentTime();//获取当前时间
//        recvInfo = "["+now.toString("hh:mm:ss")+"]";//转换成长度为10的字符串
//    }else
//    {
//        recvInfo="";
//    }

//    if(ui->ckbHexDisplay->isChecked())//是否选中了“十六进制显示”选项
//    {
//        recvInfo += recvData.toHex(' ');//转换成十六进制字符串
//        recvInfo += QString(' ');//后面追加一个空格
//    }else
//    {
//        recvInfo.append(recvData);//未选中时，直接输出字符
//    }

//    if(ui->ckbShowTime->isChecked())//如果选中了“显示时间”选项，则需要追加一个换行
//    {
//        recvInfo+='\n';
//    }

//    ui->plainTextEdit->moveCursor(QTextCursor::End);//将光标移动到末尾
//    //不使用换行
//    ui->plainTextEdit->insertPlainText(recvInfo);//使用append会导致换行


    /* 默认换行 */
//    if(ui->ckbAutoLine->isChecked())//是否选中了“自动换行”选项
//    {
//        ui->plainTextEdit->append(recvInfo);//使用append会导致换行
//    }else
//    {
//        ui->plainTextEdit->insertPlainText(recvInfo);//使用插入文本不会导致换行
//    }


}



