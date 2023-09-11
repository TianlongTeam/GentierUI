#include "gdatapool.h"

#include "gexperimentfile.h"
#include "gselfcheckrptfile.h"
#include "gwarningfile.h"
#include "widgetkeyboard.h"
#include "gcircularwidget.h"

#include "beepthread.h"

#include <QProcess>
#include <QDateTime>
#include <QTimer>
#include <QDir>

#ifdef Q_OS_LINUX
#include <sys/ioctl.h>
#endif

#include <sys/fcntl.h>
#include <sys/unistd.h>

QByteArray GDataPool::adminPassword = "gentier96";

GDataPool::GDataPool(int width, int height) : \
    keyboard(new WidgetKeyboard),
    expFile(new GExperimentFile),
    selfCheckRptFile(new GSelfCheckRptFile),
    warnFile(new GWarningFile),
    beepThread(NULL),
    circular(NULL),
    current_lan(0),
    powerOnSecs(0),
    brightness(5),
    screenTouchSound(false),
    is24HourFormat(true),
    desktopWidth(width),
    desktopHeight(height),
    desktop(NULL),
    run_method_edited(false),
    commandType(0),
    commandVule(-1),
    info_refresh_flag(0),
    readingFluor(false),
    infinite_stage_no(-1),
    infinite_step_no(-1),
    fopt(true),
    fdir(true),
    fthe(true),
    fileOpType(0)
{
    for(int i=0; i<2; i++)
        maxSpeed[i] = MAX_RAMP;

    dateFormat = "yyyy-MM-dd";

    //操作日志文件
    QString logDir= QDir::toNativeSeparators(qApp->applicationDirPath()+ QDir::separator() + SYS_DIR_LOG + QDir::separator());
    operatorLogFile = logDir + QDateTime::currentDateTime().toString("op-yyyyMMddhhmmss.log");
    //删除一个月前的操作日志
    QDateTime datetime = QDateTime::currentDateTime().addDays(-30);
    QString prevfn = datetime.toString("op-yyyyMMddhhmmss.log");
    foreach(const QString &fn, QDir(logDir).entryList(QStringList() << "op-*", QDir::Files|QDir::NoDotAndDotDot)){
        if(fn < prevfn){
            QProcess::execute(QString("rm ")+logDir+fn);
        }
    }

    //告警信息文件
    warnFile->openFile();

    //清空仪器状态存储区
    runCtrlInfo.clear();
    usbMaps.clear();

    //新建处理中窗口
    circular = new GCircularWidget(QSize(width,height));

    //蜂鸣器
    beepThread = new BeepThread;
    connect(beepThread, SIGNAL(finished()), this, SLOT(auto_end_sound()));

    QProcess::execute("/etc/init.d/udev restart");

    //初始化
    m_serialPort.setPortName("ttyO1");
    qDebug() << "ttyO0 open......";
    if (m_serialPort.open(QIODevice::ReadWrite))
    {
        bool bRet = true;
        bRet &= m_serialPort.setBaudRate(57600);
        bRet &= m_serialPort.setDataBits(QSerialPort::Data8);
        bRet &= m_serialPort.setStopBits(QSerialPort::OneStop);
        bRet &= m_serialPort.setParity(QSerialPort::NoParity);
        bRet &= m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "--------------------------->";
        qDebug() << "--------->open serial port " << m_serialPort.portName() << bRet;
        qDebug() << "--------------------------->";
        connect(&m_serialPort, &QSerialPort::readyRead, this, &GDataPool::readData, Qt::DirectConnection);
    }else{
        qDebug() << "---->Failed to open serial port" << m_serialPort.portName();
    }
}

GDataPool::~GDataPool()
{
    if(keyboard){
        delete keyboard;
        keyboard = NULL;
    }

    if(expFile){
        delete expFile;
        expFile = NULL;
    }

    if(selfCheckRptFile){
        delete selfCheckRptFile;
        selfCheckRptFile = NULL;
    }

    if(warnFile){
        delete warnFile;
        warnFile = NULL;
    }


    if(circular){
        delete circular;
        circular = NULL;
    }

    if(beepThread){
        if(beepThread->isRunning()){
            beepThread->quit();
            beepThread->wait();
        }
        delete beepThread;
        beepThread = NULL;
    }
}

void GDataPool::set_ui_busy_state(quint16 flag)
{
    if(isInitialzied()){
        QByteArray dat;
        dat.resize(sizeof(quint16));
        memcpy((void*)dat.data(), (const void*)&flag, sizeof(quint16));

        WriteData(3, 65, dat);
    }
}

void GDataPool::key_sound()
{
    if(beepThread){
        if(beepThread->property("ring").toBool()) return;
        beepThread->setProperty("ring", true);
        beepThread->setProperty("autofinished", false);
        beepThread->on_StartBeep(BeepThread::KeySound);
    }
}

void GDataPool::general_sound()
{
    if(beepThread){
        if(beepThread->property("ring").toBool()) return;
        beepThread->setProperty("ring", true);
        beepThread->setProperty("autofinished", true);
        beepThread->on_StartBeep(BeepThread::GeneralSound);
    }
}

void GDataPool::alarm_sound()
{
    if(beepThread){
        if(beepThread->property("ring").toBool()) return;
        beepThread->setProperty("ring", true);
        beepThread->setProperty("autofinished", true);
        beepThread->on_StartBeep(BeepThread::AlarmSound);
    }
}

void GDataPool::screen_sound()
{
    if(screenTouchSound && beepThread){        
        if(beepThread->property("ring").toBool()) return;
        beepThread->setProperty("ring", true);
        beepThread->setProperty("autofinished", true);
        beepThread->on_StartBeep(BeepThread::KeySound);
    }
}

void GDataPool::auto_end_sound()
{
    if(beepThread){
        if(beepThread->property("autofinished").toBool()){
            if(beepThread->property("ring").toBool())
                beepThread->setProperty("ring", false);
        }
    }
}

void GDataPool::end_sound()
{
    if(beepThread){
        if(beepThread->property("ring").toBool())
            beepThread->setProperty("ring", false);
    }
}

bool GDataPool::isInitialzied()
{
    return m_serialPort.isOpen();
}

void GDataPool::WriteData(int type1, int type2)
{
    QByteArray writeData(6, '-');
    writeData[0] = type1;
    writeData[1] = type2;
    qint64 ret = m_serialPort.write(writeData);
    if(writeData.size() != ret)
    {
        qDebug() << "err,Failed to write: " << writeData << ret;
    }
}

void GDataPool::WriteData(int type1, int type2, const QByteArray &data)
{
    QByteArray hex = data.toHex();
    int len = 6 + hex.size();
    QByteArray writeData(len, '-');
    writeData[0] = type1;
    writeData[1] = type2;
    memcpy(writeData.data()+2, hex.data(), hex.size());
    qint64 ret = m_serialPort.write(writeData);
    if(writeData.size() != ret)
    {
        qDebug() << "err,Failed to write: " << writeData << ret;
    }
}

void GDataPool::readData()
{
    m_readData += m_serialPort.readAll();
    if(m_readData.size() < 6){
        return;
    }
    int idx = m_readData.indexOf("----", 0);

    int type1 = m_readData[0];
    int type2 = m_readData[1];

    QByteArray data = m_readData.mid(2, idx-2);
    m_readData.remove(0,idx+4);

    emit sig_dealReadData(type1, type2, data);
}

void GDataPool::devicecheck()
{
    if(QFile("/dev/sda1").exists()){
        if(usbMaps.contains("/media/sda1/")){
            usbMaps.insert("/media/sda1/", "");
            emit usbChanged();
        }
    }else{
        if(usbMaps.contains("/media/sda1/")){
            usbMaps.remove("/media/sda1/");
            emit usbChanged();
        }
    }
    if(QFile("/dev/sda2").exists()){
        if(usbMaps.contains("/media/sda2/")){
            usbMaps.insert("/media/sda2/", "");
            emit usbChanged();
        }
    }else{
        if(usbMaps.contains("/media/sda2/")){
            usbMaps.remove("/media/sda2/");
            emit usbChanged();
        }
    }
    if(QFile("/dev/sda3").exists()){
        if(usbMaps.contains("/media/sda3/")){
            usbMaps.insert("/media/sda3/", "");
            emit usbChanged();
        }
    }else{
        if(usbMaps.contains("/media/sda3/")){
            usbMaps.remove("/media/sda3/");
            emit usbChanged();
        }
    }
}
