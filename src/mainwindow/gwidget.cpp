/*!
* \file gwidget.cpp
* \brief ARM板软件主界面头cpp文件
*
*ARM板软件主界面程序框架的实现。界面大小800*600pixel
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-17 13:20
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gwidget.h"

#if defined(DEVICE_TYPE_TL22)
#include "ui_gwidget.h"
#endif

#include "gglobal.h"
#include "gdatapool.h"

#include "goverview.h"
#include "gruneditor.h"
#include "grawdata.h"
#include "gutilities.h"

#include "gtransportunlockwizard.h"
#include "gpushbutton.h"
#include "qlightboxwidget.h"

#include <QDir>
#include <QMenu>
#include <QSettings>
#include <QCloseEvent>
#include <QTextBlock>
#include <QScrollBar>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>

#include "mymessagebox.h"

#include "gcircularwidget.h"

//-----------------------------------------------------------------------------
//private data class declare
//-----------------------------------------------------------------------------

/*!
* \class PrivateData
* \brief 类GWidget内部的私有数据类
*
* 用于统一管理私有数据
*/
class GWidget::PrivateData
{
public:
    PrivateData(){
        prevTabIndex = 0;
        usbStickIn = false;

        timeoutId = 0;
        timeoutcnt = 0;
        verfalg =0;
        commId = 0;
        timerId = 0;
        hotlidId = 0;
        hotcover = 0.0;
        sysState = 0;

        actionShutdown = NULL;
        actionReboot = NULL;

        lightbox = 0;
    }
    ~PrivateData(){
        foreach(int key, errorInfos.keys()){
            qDeleteAll(errorInfos.value(key)->values());
            errorInfos.value(key)->clear();
        }
        qDeleteAll(errorInfos.values());
        errorInfos.clear();

        foreach(int key, warnInfos.keys()){
            qDeleteAll(warnInfos.value(key)->values());
            warnInfos.value(key)->clear();
        }
        qDeleteAll(warnInfos.values());
        warnInfos.clear();

        if(lightbox){
            delete lightbox;
            lightbox = 0;
        }
    }

    int     prevTabIndex;                   ///< 上次Tab的序号
    QString openFile;                       ///< 当前打开的文件名
    QString file_edited_time;               ///< 当前打开文件的最后修改时间
    bool    usbStickIn;                     ///< usb设备插入
    int     timeoutId;                      ///< 命令发送超时时钟Id
    int     timeoutcnt;                     ///< 命令发送超时次数
    int     verfalg;
    int     commId;                       ///< 软件启动时读取仪器状态和温度Id
    int     timerId;                        ///< 系统时钟Id
    int     hotlidId;                       ///< 热盖图标闪烁时钟Id
    double  hotcover;                       ///< 热盖温度
    int     sysState;                       ///< 系统状态，1初始化，2自检，3复位，4待命(唯一可运行实验的状态), 5实验, 6暂停, 7故障, 8调试, 9运输锁定, 10关机

    QAction *actionShutdown;
    QAction *actionReboot;

    QLightBoxWidget *lightbox;

    QMap<int, QMap<int, QMap<int, QString>* > * > errorInfos; ///< 故障描述信息 QMap<语言，QMap<故障类型, QMap<故障代码，故障描述>>>
    QMap<int, QMap<int, QMap<int, QString>* > * > warnInfos;  ///< 警告描述信息 QMap<语言，QMap<警告类型, QMap<警告代码，警告描述>>>
};

/*!
* \brief 类GWidget的构造函数
* \param parent = NULL
* \return 无
*/
GWidget::GWidget(const QRect &desktopRect, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GWidget),
    m_d(new PrivateData),
    d_pool(new GDataPool(desktopRect.width(), desktopRect.height()))
{
    //使用Designer设计的UI界面
    ui->setupUi(this);
    d_pool->desktop = this;

    //设置窗口类型
    setWindowFlags(Qt::CustomizeWindowHint);
    //初始化变量
    initVariable();
    //初始化UI界面
    initUi();
    //检查实验文件个数是否超出
    QTimer::singleShot(500, this, SLOT(g_fileCountCheck()));
}

/*!
* \brief 类GWidget的析构函数
* \param 无
* \return 无
*/
GWidget::~GWidget()
{    
    delete d_pool;

    if(m_d->hotlidId){
        killTimer(m_d->hotlidId);
        m_d->hotlidId = 0;
    }

    if(m_d->timerId){
        killTimer(m_d->timerId);
        m_d->timerId = 0;
    }
    if(m_d->commId){
        killTimer(m_d->commId);
        m_d->commId = 0;
    }
    delete m_d;

    delete ui;
    qDebug() << "delete GWidget";
}

/*!
* \brief 类GWidget的公共函数
* \param 无
* \return 返回当前设置的语言
*/
int GWidget::defaultLanguageId()
{
    return d_pool->current_lan;
}

/*!
* \brief 类GWidget的公共槽函数，实现程序语言的变化
* \param lan 中文0，英文1
* \return 无
*/
void GWidget::g_changeLanguage(int lan)
{
    if(lan == 0){
        qApp->removeTranslator(&translatorQt);
        qApp->removeTranslator(&translator);
    }else{
        translatorQt.load(":/file/qt_zh_CN");
        qApp->installTranslator(&translatorQt);

        QString path = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_LANGUAGE + QDir::separator());
        translator.load(path + "armui_zh");
        qApp->installTranslator(&translator);
    }

    d_pool->current_lan = lan;

    qDebug() << "language changed to" << lan;
}

/*!
* \brief 类GWidget的公共槽函数，显示当前网络连接图标
* \param connect 是否连接
* \return 无
*/
void GWidget::g_networkState(bool connect)
{
    ui->labelWeb->setPixmap(connect ? QPixmap(":/png/network_connect") : QPixmap(":/png/network_disconnect"));
}

/*!
* \brief 类GWidget的公共槽函数，显示当前的实验文件
* \param info 系统信息
* \return 无
*/
void GWidget::setFileInfomation(const QString &info)
{
    ui->labelFile->setText(info);
}

/*!
* \brief 类GWidget的继承事件，用于翻译
* \param 无
* \return 无
*/

void GWidget::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);//在此处刷新语言的

        int tab_index_ = 0;
        ui->tabWidgetMain->setTabText(tab_index_++, tr("Experiment File"));
        ui->tabWidgetMain->setTabText(tab_index_++, tr("Run Setting"));
        ui->tabWidgetMain->setTabText(tab_index_++, tr("Run Monitoring"));
        ui->tabWidgetMain->setTabText(tab_index_++, tr("General Setting"));

        if(!d_pool->expFile->fileName().isEmpty()){
            QFileInfo fileInfo(d_pool->expFile->fileName());
            setFileInfomation(fileInfo.completeBaseName());
        }

        m_d->actionShutdown->setText(tr("Shutdown"));
        m_d->actionReboot->setText(tr("Restart"));
    }
    QWidget::changeEvent(e);
}

/*!
* \brief 类GWidget的继承事件，用于时间控制时的操作
* \param e 计时事件
* \return 无
*/
void GWidget::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    //如果是命令超时,重发命令 //
    if(id == m_d->timeoutId){
        killTimer(m_d->timeoutId);
        m_d->timeoutId = 0;

        //判断重发次数
        if(m_d->timeoutcnt >= 3){
            if(d_pool->circular->isShowing()){
                d_pool->circular->hideProcess();
                this->setProperty("message_is_showing", false);
            }

            d_pool->commandVule = -1;
            m_d->timeoutcnt = 0;

            _PCR_RUN_CTRL_INFO pcrInfo;
            d_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
            d_pool->mutex.unlock();

            buttonRefresh(pcrInfo);

            My_MessageBox mb;
            mb.gwarning(d_pool, NULL, tr("Warning"), tr("Request fail!"));
            return;
        }else{
            m_d->timeoutcnt ++;
        }

        qDebug()<<"如果是命令超时,重发命令:" << d_pool->commandType;
        switch(d_pool->commandType){
        case 2:{
            QString fn = d_pool->expFile->fileName().trimmed();
            if(fn.isEmpty()){
                break;
            }

            //如果当前文件是tlpd文件,提示不能运行
            QFileInfo fileInfo(fn);
            quint16 currentCmd = 1;
            QByteArray dat;
            dat.resize(sizeof(quint16));
            memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));
            dat.append(fileInfo.fileName().toUtf8());
            sendControlCMD(2, 45, dat);
            if(d_pool->commandType!=2){
                break;
            }

            break;
        }
        case 43:{
            QByteArray dat;
            dat.resize(sizeof(quint16));

            quint16 currentCmd = 2;
            memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));
            sendControlCMD(3, 45, dat);
            break;
        }
        case 44:{
            QByteArray dat;
            dat.resize(sizeof(quint16));
            quint16 currentCmd = 0;
            memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));
            sendControlCMD(1, 45, dat);
            break;
        }
        default:break;
        }
    }else if(id == m_d->commId){  //定时读取设备信息
        d_pool->devicecheck();

        int cmdId = this->property("commandId").toInt();
        switch(cmdId){
        case 0:{
            if(d_pool->isInitialzied()){
                d_pool->WriteData(1, 14);
            }
            qDebug() << QTime::currentTime().toString("hh:mm:ss") << "reading instrumnet info";
            break;
        }
        case 1:
            if(d_pool->isInitialzied())
                d_pool->WriteData(1, 12);
            QTimer::singleShot(3000, this, SLOT(g_getMaxSpeedAgain()));
            qDebug() << QTime::currentTime().toString("hh:mm:ss") << "reading max speed data";
            break;
        case 2:
            if(d_pool->isInitialzied())
                d_pool->WriteData(1, 13);
            qDebug() << QTime::currentTime().toString("hh:mm:ss") << "reading instrumentTypeName data";
            break;
        default:
            //查询状态信息
            if(d_pool->isInitialzied())
                d_pool->WriteData(1, 1);
            qDebug() << QTime::currentTime().toString("hh:mm:ss") << "reading run info";
            break;
        }
    }else if(id == m_d->timerId){
        QDateTime current = QDateTime::currentDateTime();

        //显示系统时钟
        QString dateTimeFormat = d_pool->is24HourFormat ? "hh:mm" : "hh:mm AP";
        dateTimeFormat += "\n" + d_pool->dateFormat;
        ui->labelTime->setText(current.toString(dateTimeFormat));

        //判断超过1小时即记录仪器运行时间
        qint64 secs = openDateTime.secsTo(current);
        if(secs >= 59){
            //更新开机时间
            setOpenDateTime();

            if(d_pool->selfCheckRptFile){
                d_pool->selfCheckRptFile->runMinutes++;
                d_pool->selfCheckRptFile->saveReport();

                int tab_index_ = sw_Utilities;
                GUtilities *us = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
                if(us) us->updateSelfTest();
            }
        }
    }else if(id == m_d->hotlidId){
        twinkleHotlidIcon();
    }
}

/*!
* \brief 初始化变量
* \param 无
* \return 无
*/
void GWidget::initVariable()
{
    qDebug() << tr("Open Software");

    //读取软件配置信息
    readConfInfo();

    //添加重启和关机菜单
    QMenu *menu = new QMenu(this);
    m_d->actionShutdown = new QAction(tr("Shutdown"), menu);
    menu->addAction(m_d->actionShutdown);
    connect(m_d->actionShutdown, SIGNAL(triggered()), this, SLOT(g_sysShutdown()));
    menu->addSeparator();
    m_d->actionReboot = new QAction(tr("Restart"), menu);
    menu->addAction(m_d->actionReboot);
    connect(m_d->actionReboot, SIGNAL(triggered()), this, SLOT(g_sysReboot()));
#ifdef DEVICE_TYPE_TL22
    ui->buttonExit->setShowMode(GPushButton::UpRight);
    ui->buttonExit->setMargin(9,50,9,50);
#endif
    ui->buttonExit->setMenu(menu);

    //添加故障和警告描述信息变量, Arm-0, Optical-1, Thermal-2, Driver-3
    for(int i=0; i<4; i++){
        QMap<int, QMap<int,QString>* > *type = new QMap<int, QMap<int,QString>* >();

        QMap<int,QString> *info = new QMap<int,QString>();
        type->insert(KEY_ARM, info);
        info = new QMap<int,QString>();
        type->insert(KEY_OPTICAL, info);
        info = new QMap<int,QString>();
        type->insert(KEY_THERMAL, info);
        info = new QMap<int,QString>();
        type->insert(KEY_DRIVER, info);

        switch(i){
        case 0:m_d->errorInfos.insert(LANG_CHINESE, type);break;
        case 1:m_d->errorInfos.insert(LANG_ENGLISH, type);break;
        case 2:m_d->warnInfos.insert(LANG_CHINESE, type);break;
        case 3:m_d->warnInfos.insert(LANG_ENGLISH, type);break;
        default:break;
        }
    }

    //读取故障及警告描述信息
    readErrorInfo();
    //读取ARM系统版本
    QProcess process;
    process.start("cat /etc/sysversion");
    process.waitForFinished();
    d_pool->systemVersion = QString(process.readAllStandardOutput()).trimmed();

    //初始化通讯
    connect(d_pool, SIGNAL(sig_dealReadData(quint8,quint16,QByteArray)), this, SLOT(g_dealReadData(quint8,quint16,QByteArray)));

    connect(this, SIGNAL(operatorLog(QString)), SLOT(slot_operatorLog(QString)));

    //初始化U盘监控
    connect(d_pool, SIGNAL(usbChanged()), this, SLOT(slot_usbDeviceChanged()));

    this->setProperty("reboot", false);         ///< 是否断电重启
    this->setProperty("showMessageBox", false); ///< 在通讯中是否显示提示框
    this->setProperty("commandId", 0);          ///< 发送的命令ID
    this->setProperty("inEditting", false);     ///< UI处于Editing模式

    this->setProperty("message_is_showing", false);   ///< 点击时是否正在显示提示框

    //时钟,1s
    m_d->timerId = this->startTimer(1000);
}

/*!
* \brief 私有槽函数，根据仪器状态更新按键显示
* \param state 仪器状态
* \return 无
*/
void GWidget::buttonRefresh(const _PCR_RUN_CTRL_INFO &pcr_info, bool unuse_sync_flag)
{
    int sync_flag_ = unuse_sync_flag ? 0 : pcr_info.State.SyncFlag;

    switch(pcr_info.State.ExpState){
    case 0:
    {
        updateStateInfo(4);

        if(!ui->buttonStart_Pause->isEnabled())
            ui->buttonStart_Pause->setEnabled(pcr_info.State.DrvDrawerPos && sync_flag_==0);

#if (!defined DEVICE_TYPE_TL12)
        ui->buttonStart_Pause->setIcon(QIcon(":/png/run"));
#endif
#ifdef DEVICE_TYPE_TL22
        if(!ui->buttonEject->isEnabled())
            ui->buttonEject->setEnabled(sync_flag_ != 1);
#endif
        if(ui->buttonStop->isEnabled())
            ui->buttonStop->setEnabled(false);
        if(!ui->buttonExit->isEnabled())
            ui->buttonExit->setEnabled(true);
        break;}
    case 1:
        updateStateInfo(5);
#if (!defined DEVICE_TYPE_TL12)
        ui->buttonStart_Pause->setIcon(QIcon(":/png/pause"));
#endif
        ui->buttonStart_Pause->setEnabled(pcr_info.State.DrvDrawerPos && !pcr_info.State.DrvMotoBusy && pcr_info.State.ThFlag && sync_flag_==0);

        if(!ui->buttonStop->isEnabled())
            ui->buttonStop->setEnabled(sync_flag_==0);

#ifdef DEVICE_TYPE_TL22
        if(ui->buttonEject->isEnabled())
            ui->buttonEject->setEnabled(false);
#endif        

        if(ui->buttonExit->isEnabled())
            ui->buttonExit->setEnabled(false);
        break;
    case 2:
        updateStateInfo(6);
#if (!defined DEVICE_TYPE_TL12)
        ui->buttonStart_Pause->setIcon(QIcon(":/png/run"));
#endif
#ifdef DEVICE_TYPE_TL22        
        if(pcr_info.State.DrvDrawerPos && pcr_info.State.DrvLidPos){
            if(!ui->buttonStart_Pause->isEnabled())
                ui->buttonStart_Pause->setEnabled(pcr_info.State.DrvDrawerPos && sync_flag_==0);
            if(!ui->buttonStop->isEnabled())
                ui->buttonStop->setEnabled(sync_flag_ == 0);
        }else{
            if(ui->buttonStart_Pause->isEnabled())
                ui->buttonStart_Pause->setEnabled(false);
            if(ui->buttonStop->isEnabled())
                ui->buttonStop->setEnabled(false);
        }

        ui->buttonEject->setEnabled(sync_flag_ != 1);
#else
        ui->buttonStart_Pause->setEnabled(pcr_info.State.DrvDrawerPos && sync_flag_==0);

        if(!ui->buttonStop->isEnabled())
            ui->buttonStop->setEnabled(sync_flag_ == 0);
#endif

        if(ui->buttonExit->isEnabled())
            ui->buttonExit->setEnabled(false);
        break;
    default:break;
    }
}

/*!
* \brief 初始化designer设计的界面
* \param 无
* \enum GWidget::SubWidget
* \return 无
*/
void GWidget::initUi()
{
    ui->labelHotlid->setProperty("using", false);
    ui->labelHotlid->setVisible(false);
    ui->labelUsb->setVisible(false);

    ui->buttonAlarm->setEnabled(false);
    ui->buttonStop->setEnabled(false);

    ui->buttonStart_Pause->setEnabled(false);

#ifdef DEVICE_TYPE_TL22
    ui->labelError->setVisible(false);   
    ui->buttonEject->setEnabled(false);
#else

    bool printer_visible_ = false;
    if ( 6 == GHelper::total_instrument_id )
    {
        printer_visible_ = QFile(PRINTER_ADDR).exists();
    }
    ui->labelPrint->setVisible( printer_visible_ );

#if (defined DEVICE_TYPE_TL12)
    ui->labellidopen->setPixmap(QPixmap(":/png/door"));
#else
    ui->labellidopen->setPixmap(QPixmap(":/png/hotlid"));
#endif

    ui->labellidopen->setVisible(false);
//    ui->buttonStart_Pause->setEnabled(true);
#endif
    ui->buttonExit->setEnabled(false);

    QString pixmap_file;
    if(d_pool->current_lan == 1){
        pixmap_file = QStringLiteral("./logo1");
    }else{
        pixmap_file = QStringLiteral("./logo");
    }

    ui->labelLogo->setPixmap(QPixmap(pixmap_file).scaled(110,36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));


    //添加4个子窗口

    //1. 文件管理子窗口
    GOverView *ov = new GOverView(d_pool);
    //    ui->tabWidgetMain->addTab(ov, QIcon(":/png/tab1"), tr("Overview"));
    ui->tabWidgetMain->addTab(ov, tr("Experiment File"));
    connect(ov, SIGNAL(expFileFocus(bool)), this, SLOT(slot_fileFocus(bool)));
    connect(ov, SIGNAL(expFileChanged(QString,int)), this, SLOT(slot_fileChanged(QString,int)));
    connect(ov, SIGNAL(next()), this, SLOT(slot_nextWidget()));
    connect(ov, SIGNAL(operatorLog(QString)), this, SLOT(slot_operatorLog(QString)));
    connect(d_pool, SIGNAL(usbChanged()), ov, SLOT(slot_usbDeviceChanged()));

    qDebug() << tr("Add Overview widget");
    //2. 运行设置子窗口
    GRunEditor *re = NULL;
    if(d_pool->expFile)
        re = new GRunEditor(d_pool);
    else
        re = new GRunEditor(NULL);
    //    ui->tabWidgetMain->addTab(re, QIcon(":/png/tab2"), tr("Run Editor"));
    ui->tabWidgetMain->addTab(re, tr("Run Setting"));
    connect(re, SIGNAL(editting2(bool)), this, SLOT(slot_editting(bool)));
    connect(re, SIGNAL(editting(bool)), this, SLOT(slot_runeditor_tabDisabled(bool)));
    connect(re, SIGNAL(methodEdited()), this, SLOT(slot_runMethod_edited()));
    connect(this, SIGNAL(updateExp()), re, SLOT(updateConfig()));

    connect(re, SIGNAL(operatorLog(QString)), this, SLOT(slot_operatorLog(QString)));
    connect(this, SIGNAL(experimentCtrl(int)), re, SLOT(experimentState(int)));
    qDebug() << tr("Add Run Setting widget");

    //3. 运行监控子窗口
    GRawData *rd = new GRawData(d_pool);
    //    ui->tabWidgetMain->addTab(rd, QIcon(":/png/tab3"), tr("Raw Data"));
    ui->tabWidgetMain->addTab(rd, tr("Run Monitoring"));

    connect(this, SIGNAL(fluor_scan_info(QByteArray)), rd, SLOT(slot_fluor_scan_info(QByteArray)));

    connect(re, SIGNAL(methodChanged()), rd, SLOT(updateTempDisplay()));
    connect(this, SIGNAL(updateExp()), rd, SLOT(updateFluorDisplay()));

    connect(rd, SIGNAL(operatorLog(QString)), this, SLOT(slot_operatorLog(QString)));
    connect(this, SIGNAL(experimentCtrl(int)), rd, SLOT(monitorCtrl(int)));
    qDebug() << tr("Add Run Monitoring widget");

    //5. 通用设置子窗口
    GUtilities *us = new GUtilities(d_pool);
    //    ui->tabWidgetMain->addTab(us, QIcon(":/png/tab4"), tr("Utilities"));
    ui->tabWidgetMain->addTab(us, tr("General Setting"));
    connect(us, SIGNAL(changeLanguage(int)), this, SLOT(g_changeLanguage(int)));
    connect(us, SIGNAL(networkState(bool)), this, SLOT(g_networkState(bool)));
    connect(us, SIGNAL(editting2(bool,bool)), this, SLOT(slot_editting(bool,bool)));
    connect(us, SIGNAL(editting(bool)), this, SLOT(slot_utilities_tabDisabled(bool)));

    connect(d_pool, SIGNAL(usbChanged()), us, SLOT(slot_usbDeviceChanged()));
    connect(us, SIGNAL(operatorLog(QString)), this, SLOT(slot_operatorLog(QString)));

    connect(us, SIGNAL(configChanged()), this, SLOT(saveConfInfo()));
    connect(us, SIGNAL(transportLocking(bool)), this, SLOT(g_transportLocking(bool)));

    connect(ov, SIGNAL(fileOperator(bool)), us, SLOT(slot_fileOperator(bool)));

    //清除存储区时清除当前选择的文件
    connect(us, SIGNAL(clearCurrent()), ov, SLOT(slot_clearCurrentFile()));
    us->initLanguage(d_pool->current_lan);
    us->updateSelfTest();
    us->updateCurrentDate();

    qDebug() << tr("Add General Setting widget");

    //设置初始Tab序号
    ui->tabWidgetMain->setCurrentIndex(sw_OverView);
    ui->tabWidgetMain->setStyleSheet("QTabBar::tab {height:43px; width:180px;}  QTabBar::tab:selected{border-image: url(:/png/BodyClicked);}\nQTabBar::tab:!enabled{border-image: url(:/png/BodyDisabled);} \nQTabBar::tab{border-image: url(:/png/Body);}");

    //设置信号槽连接
    connect(re, SIGNAL(channelModeChanged(QByteArray)), rd, SLOT(slot_channelModeChanged(QByteArray)));
    connect(re, SIGNAL(channelItemChanged(int,bool,QByteArray)), rd, SLOT(slot_channelItemChanged(int,bool,QByteArray)));
    connect(re, SIGNAL(sampleChanged()), rd, SLOT(slot_sampleChanged()));

    connect(ui->tabWidgetMain, SIGNAL(tabBarClicked(int)), this, SLOT(slot_tabBarClicked(int)));
    connect(ui->tabWidgetMain, SIGNAL(currentChanged(int)), this, SLOT(slot_currentChanged(int)));

    slot_usbDeviceChanged();

    //Show Status : Initialize
    updateStateInfo(1);

    ui->tabWidgetMain->setEnabled(false);
    ui->tabWidgetMain->clearFocus();

    //读取仪器信息
    m_d->commId = startTimer(SOCKET_INTERVAL);
}

void GWidget::version()
{
    if(d_pool->isInitialzied())
        d_pool->WriteData(1, 15);
    QFile file("~/log/version.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out<< " SystemVersion                  : " <<d_pool->systemVersion<< "\n";
    out<< " ApplicationVersion             : "<<qApp->applicationVersion()+"("+ tr("%1.%2.%3").arg(d_pool->ArmVer.Bits.VerMajor).arg(d_pool->ArmVer.Bits.VerMinor).arg(QString::number(d_pool->ArmVer.Bits.VerRevision).rightJustified(3,'0',true))+")"<< "\n";
    out<< " Optical Module                 : "<< tr("%1.%2.%3").arg(d_pool->OptVer.Bits.VerMajor).arg(d_pool->OptVer.Bits.VerMinor).arg(QString::number(d_pool->OptVer.Bits.VerRevision).rightJustified(3,'0',true))<< "\n";
    out<< " Thermal Cycle Module           : "<< tr("%1.%2.%3").arg(d_pool->TherVer.Bits.VerMajor).arg(d_pool->TherVer.Bits.VerMinor).arg(QString::number(d_pool->TherVer.Bits.VerRevision).rightJustified(3,'0',true))<< "\n";
    out<< " Driver Module                  : "<< tr("%1.%2.%3").arg(d_pool->DrvVer.Bits.VerMajor).arg(d_pool->DrvVer.Bits.VerMinor).arg(QString::number(d_pool->DrvVer.Bits.VerRevision).rightJustified(3,'0',true))<< "\n";
}

/*!
* \brief 私有函数，alarm3Sound()
* \param 无
* \return 无
*/
void GWidget::alarm3Sound()
{
    d_pool->alarm_sound();
    QThread::sleep(1);
    d_pool->alarm_sound();
    QThread::sleep(1);
    d_pool->alarm_sound();
}

/*!
* \brief 私有函数，更新仪器状态信息
* \param 无
* \return 无
*/
void GWidget::updateStateInfo(int state)
{
    m_d->sysState = state;

    QString stateStr;
    switch(m_d->sysState){
    case 1:stateStr = tr("Initializing ...");break;
    case 2:stateStr = tr("System Self Inspecting");break;
    case 3:stateStr = QString::null;break;
    case 4:stateStr = tr("Ready");break;
    case 5:stateStr = tr("Running");break;
    case 6:stateStr = tr("Pause");break;
    case 7:stateStr = tr("Error");break;
    case 8:stateStr = tr("Debugging");break;
    case 9:stateStr = tr("Transport Locked");break;
    case 10:stateStr = tr("Shutting Down");break;
    default:stateStr = QString::null;break;
    }
    ui->labelStatus->setText(stateStr);
}

/*!
* \brief 私有槽函数，发送命令，同时开启定时器，如果规定时间没有收到响应
* \param 无
* \return 无
*/
void GWidget::sendControlCMD(int commandId, quint16 ctrl, const QByteArray& dat)
{    
    //如果在等待控制命令的响应,则退出
    if(m_d->timeoutId != 0) return;
    d_pool->mutex.lock();
    d_pool->commandType = commandId;
    d_pool->mutex.unlock();
    qDebug() << "send command id:" << commandId;

    if(d_pool->isInitialzied()){
        d_pool->WriteData(3, ctrl, dat);
    }

    m_d->timeoutId = startTimer(910);
}

/*!
* \brief 私有槽函数，读取软件的配置信息
* \param 无
* \return 无
*/
void GWidget::readConfInfo()
{
    QString fn = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG + QDir::separator() + "config.ini");

    //如果存在lock文件，则删除lock文件读取带后缀的时间最近的文件配置
    bool exist_lock_file_ = false;
    bool not_first_file_ = false;
    QString lockfile = "config.ini.*";
    QString last_conf_file_;
    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        if(fileInfo.suffix() == QStringLiteral("lock")){
            exist_lock_file_ = true;
        }else if(not_first_file_){
            if(fileInfo.lastModified() > QFileInfo(last_conf_file_).lastModified()){
                last_conf_file_ = fileInfo.filePath();
            }
        }else{
            not_first_file_ = true;
            last_conf_file_ = fileInfo.filePath();
        }
    }

    if(exist_lock_file_ && !last_conf_file_.isEmpty()){
        QFile(fn).remove();
        QFile(last_conf_file_).rename(fn);
    }

    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        QFile(fileInfo.filePath()).remove();
    }

    //读取配置文件，设置配置变量
    QSettings fileSetting(fn, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");

    QStringList groupList = fileSetting.childGroups();

    //设置信息
    if(groupList.contains(QLatin1String("Device"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("Device", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));

        d_pool->brightness = fileSetting.value("Brightness", 100).toInt();
        d_pool->screenTouchSound = fileSetting.value("ScreenTouchSound", false).toBool();
        d_pool->is24HourFormat = fileSetting.value("Hour24Format", true).toBool();
        d_pool->dateFormat = fileSetting.value("DateFormat", "yyyy-MM-dd").toString().trimmed();
        d_pool->adminPassword = fileSetting.value("AdminPassword", "gentier96").toByteArray();
        fileSetting.endGroup();
    }
    //语言设置
    d_pool->current_lan = 0;
    if(groupList.contains(tr("Language"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("Language", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));
        d_pool->current_lan = fileSetting.value("Default").toInt();
        fileSetting.endGroup();
    }
}

/*!
* \brief 私有槽函数，读取故障及警告描述信息
* \param 无
* \return 无
*/
void GWidget::readErrorInfo()
{
    QString fn = ERR_WARN_INFO_FILE;

    if(!QFile::exists(fn)) return;

    QSettings fileSetting(fn, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");

    QStringList groupList = fileSetting.childGroups();

    //ARM信息描述
    if(groupList.contains(tr("arm"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("arm", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));

        //故障信息
        int itemSize = fileSetting.beginReadArray("error");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->errorInfos.contains(LANG_CHINESE) && m_d->errorInfos.value(LANG_CHINESE)->contains(KEY_ARM))
                m_d->errorInfos.value(LANG_CHINESE)->value(KEY_ARM)->insert(i+1, chStr);
            if(m_d->errorInfos.contains(LANG_ENGLISH) && m_d->errorInfos.value(LANG_ENGLISH)->contains(KEY_ARM))
                m_d->errorInfos.value(LANG_ENGLISH)->value(KEY_ARM)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        //警告信息
        itemSize = fileSetting.beginReadArray("warn");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->warnInfos.contains(LANG_CHINESE) && m_d->warnInfos.value(LANG_CHINESE)->contains(KEY_ARM))
                m_d->warnInfos.value(LANG_CHINESE)->value(KEY_ARM)->insert(i+1, chStr);
            if(m_d->warnInfos.contains(LANG_ENGLISH)&& m_d->warnInfos.value(LANG_ENGLISH)->contains(KEY_ARM))
                m_d->warnInfos.value(LANG_ENGLISH)->value(KEY_ARM)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        fileSetting.endGroup();
    }
    //光学信息描述
    if(groupList.contains(tr("optical"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("optical", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));

        //故障信息
        int itemSize = fileSetting.beginReadArray("error");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->errorInfos.contains(LANG_CHINESE) && m_d->errorInfos.value(LANG_CHINESE)->contains(KEY_OPTICAL))
                m_d->errorInfos.value(LANG_CHINESE)->value(KEY_OPTICAL)->insert(i+1, chStr);
            if(m_d->errorInfos.contains(LANG_ENGLISH) && m_d->errorInfos.value(LANG_ENGLISH)->contains(KEY_OPTICAL))
                m_d->errorInfos.value(LANG_ENGLISH)->value(KEY_OPTICAL)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        //警告信息
        itemSize = fileSetting.beginReadArray("warn");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->warnInfos.contains(LANG_CHINESE)&& m_d->warnInfos.value(LANG_CHINESE)->contains(KEY_OPTICAL))
                m_d->warnInfos.value(LANG_CHINESE)->value(KEY_OPTICAL)->insert(i+1, chStr);
            if(m_d->warnInfos.contains(LANG_ENGLISH) && m_d->warnInfos.value(LANG_ENGLISH)->contains(KEY_OPTICAL))
                m_d->warnInfos.value(LANG_ENGLISH)->value(KEY_OPTICAL)->insert(i+1, enStr);
        }
        fileSetting.endArray();
        fileSetting.endGroup();
    }
    //热学信息描述
    if(groupList.contains(tr("thermal"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("thermal", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));

        //故障信息
        int itemSize = fileSetting.beginReadArray("error");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->errorInfos.contains(LANG_CHINESE) && m_d->errorInfos.value(LANG_CHINESE)->contains(KEY_THERMAL))
                m_d->errorInfos.value(LANG_CHINESE)->value(KEY_THERMAL)->insert(i+1, chStr);
            if(m_d->errorInfos.contains(LANG_ENGLISH) && m_d->errorInfos.value(LANG_ENGLISH)->contains(KEY_THERMAL))
                m_d->errorInfos.value(LANG_ENGLISH)->value(KEY_THERMAL)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        //警告信息
        itemSize = fileSetting.beginReadArray("warn");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->warnInfos.contains(LANG_CHINESE) && m_d->warnInfos.value(LANG_CHINESE)->contains(KEY_THERMAL))
                m_d->warnInfos.value(LANG_CHINESE)->value(KEY_THERMAL)->insert(i+1, chStr);
            if(m_d->warnInfos.contains(LANG_ENGLISH) && m_d->warnInfos.value(LANG_ENGLISH)->contains(KEY_THERMAL))
                m_d->warnInfos.value(LANG_ENGLISH)->value(KEY_THERMAL)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        fileSetting.endGroup();
    }
    //驱动板信息描述
    if(groupList.contains(tr("driver"), Qt::CaseInsensitive)){
        int pos = groupList.indexOf(QRegExp("driver", Qt::CaseInsensitive));
        fileSetting.beginGroup(groupList.at(pos));

        //故障信息
        int itemSize = fileSetting.beginReadArray("error");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->errorInfos.contains(LANG_CHINESE) && m_d->errorInfos.value(LANG_CHINESE)->contains(KEY_DRIVER))
                m_d->errorInfos.value(LANG_CHINESE)->value(KEY_DRIVER)->insert(i+1, chStr);
            if(m_d->errorInfos.contains(LANG_ENGLISH) && m_d->errorInfos.value(LANG_ENGLISH)->contains(KEY_DRIVER))
                m_d->errorInfos.value(LANG_ENGLISH)->value(KEY_DRIVER)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        //警告信息
        itemSize = fileSetting.beginReadArray("warn");
        for(int i=0; i<itemSize; i++){
            fileSetting.setArrayIndex(i);
            QString chStr = fileSetting.value("ch").toString();
            QString enStr = fileSetting.value("en").toString();
            if(m_d->warnInfos.contains(LANG_CHINESE) && m_d->warnInfos.value(LANG_CHINESE)->contains(KEY_DRIVER))
                m_d->warnInfos.value(LANG_CHINESE)->value(KEY_DRIVER)->insert(i+1, chStr);
            if(m_d->warnInfos.contains(LANG_ENGLISH) && m_d->warnInfos.value(LANG_ENGLISH)->contains(KEY_DRIVER))
                m_d->warnInfos.value(LANG_ENGLISH)->value(KEY_DRIVER)->insert(i+1, enStr);
        }
        fileSetting.endArray();

        fileSetting.endGroup();
    }
}

/*!
* \brief 私有槽函数，保存软件的配置信息
* \param 无
* \return 无
*/
void GWidget::saveConfInfo()
{
    qDebug() << Q_FUNC_INFO;
    QString lockfile = "config.ini.*";
    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        QFile(fileInfo.filePath()).remove();
    }

    //保存配置文件
    QString fn = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG + QDir::separator() + "config.ini");    

    QSettings fileSetting(fn, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");
    fileSetting.clear();

    //设备信息
    fileSetting.beginGroup("Device");
    fileSetting.setValue("Brightness", d_pool->brightness);
    fileSetting.setValue("ScreenTouchSound", d_pool->screenTouchSound);
    fileSetting.setValue("Hour24Format", d_pool->is24HourFormat);
    fileSetting.setValue("DateFormat", d_pool->dateFormat);
    fileSetting.endGroup();

    //语言设置
    fileSetting.beginGroup("Language");
    fileSetting.setValue("Default", d_pool->current_lan);

    QStringList langNames;
    langNames << "English" << "简体中文";
    fileSetting.beginWriteArray("Item");
    for(int i=0; i<langNames.count(); i++){
        fileSetting.setArrayIndex(i);
        fileSetting.setValue("Id", i);
        fileSetting.setValue("Title", langNames.at(i));
    }
    fileSetting.endArray();
    fileSetting.endGroup();

    qDebug() << "保存配置文件。";
}

/*!
* \brief 私有槽函数，设置软件处于运输锁定操作过程中
* \param 无
* \return 无
*/
void GWidget::g_transportLocking(bool locking)
{
    if(locking){
        killTimer(m_d->commId);
        m_d->commId = 0;
    }else{
        m_d->commId = startTimer(SOCKET_INTERVAL);
    }
}

/*!
* \brief 私有槽函数，系统关机
* \param 无
* \return 无
*/
void GWidget::g_sysShutdown()
{
    d_pool->key_sound();

    m_d->lightbox = new QLightBoxWidget(d_pool, tr("Please turn off the power\nafter the progress bar is cleared"), this);
    connect(m_d->lightbox, SIGNAL(signal_ok()), this, SLOT(g_shutdown_halt()));
    connect(m_d->lightbox, SIGNAL(signal_cancel()), this, SLOT(g_shutdown_cancel()));
    m_d->lightbox->show();
}

/*!
* \brief 私有槽函数，系统重启
* \param 无
* \return 无
*/
void GWidget::g_sysReboot()
{
    d_pool->key_sound();

    m_d->lightbox = new QLightBoxWidget(d_pool, tr("Restart the instrument, sure?"), this);
    connect(m_d->lightbox, SIGNAL(signal_ok()), this, SLOT(g_shutdown_reboot()));
    connect(m_d->lightbox, SIGNAL(signal_cancel()), this, SLOT(g_shutdown_cancel()));
    m_d->lightbox->show();
}

void GWidget::g_shutdown_halt()
{
    emit operatorLog(tr("Shutdown"));
    shutdown_save_infos();
    QProcess::execute("shutdown -h now");
}

void GWidget::g_shutdown_reboot()
{
    emit operatorLog(tr("Restart"));
    shutdown_save_infos();
    QProcess::execute("shutdown -r now");
}

void GWidget::g_shutdown_cancel()
{
    if(m_d->lightbox){
        m_d->lightbox->hide();
        m_d->lightbox->disconnect(this);
        delete m_d->lightbox;
        m_d->lightbox = 0;
    }
}

void GWidget::g_fileCountCheck()
{
    //判断当前数据目录中的文件个数是否超出最大设定值，显示提示
#ifdef Q_OS_LINUX
    QString sysDir = SYS_DIR_FILE;
#else
    QString sysDir = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_FILE);
#endif
    int currentFileCount = QDir(sysDir).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();

#ifdef DEVICE_TYPE_TL13
    foreach(const QFileInfo &fileInfo, QDir(sysDir).entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs)){
        currentFileCount += QDir(fileInfo.filePath()).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
    }
#endif

    if(currentFileCount >= EXPERIMENT_TOTAL_COUNT){
        My_MessageBox mb;        
        mb.ginformation(d_pool, NULL, tr("Prompt"), tr("Too many experiment files, please backup and delete useless files."));
    }
}

void GWidget::g_getMaxSpeedAgain()
{
    if(d_pool->isInitialzied())
        d_pool->WriteData(1, 78);
    qDebug() << QTime::currentTime().toString("hh:mm:ss") << "reading max speed data";
}

/*!
* \brief 私有槽函数，清除打开文件时新建的临时目录
* \param 无
* \return 无
*/
void GWidget::clearTmpDirectory()
{
    QString tmpPath = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + TMP_DIRCTORY + QDir::separator());

    removeDirWithContent(tmpPath);
}

/*!
* \brief 私有槽函数，递归删除文件夹,即使里面有内容
* \param dirName 文件夹的名字,绝对路径和相对路径均可
* \return 成功后返回true;否则返回false
*/
bool GWidget::removeDirWithContent(const QString &dirName, bool removeDir)
{
    static QVector<QString> dirNames;

    QDir dir;
    QFileInfoList filst;
    QFileInfoList::iterator curFi;
    //初始化
    dirNames.clear();
    if(dir.exists()){
        dirNames<<dirName;
    }
    else{
        return true;
    }
    //遍历各级文件夹，并将这些文件夹中的文件删除
    for(int i=0;i<dirNames.size();++i){
        dir.setPath(dirNames[i]);
        filst=dir.entryInfoList(QDir::Dirs|QDir::Files
                                |QDir::Readable|QDir::Writable
                                |QDir::Hidden|QDir::NoDotAndDotDot
                                ,QDir::Name);
        if(filst.size()>0){
            curFi=filst.begin();
            while(curFi!=filst.end()){
                //遇到文件夹,则添加至文件夹列表dirs尾部
                if(curFi->isDir()){
                    dirNames.push_back(curFi->filePath());
                }else if(curFi->isFile()){
                    //遇到文件,则删除之
                    if(!dir.remove(curFi->fileName())){
                        return false;
                    }
                }
                curFi++;
            }//end of while
        }
    }
    //删除文件夹
    for(int i=dirNames.size()-1;i>0;--i){
        if(!dir.rmdir(dirNames[i])){
            return false;
        }
    }

    if(removeDir)
        dir.rmdir(dirNames[0]);

    return true;
}

/*!
* \brief 私有函数，实现热盖图标的闪烁，表示实验正在运行中
* \param 无
* \return 无
*/
void GWidget::twinkleHotlidIcon()
{
    int val = ui->labelHotlid->property("count").toInt() + 1;
    ui->labelHotlid->setProperty("count", val);
    ui->labelHotlid->setVisible(val & 0x01);
}

/*!
* \brief 私有函数，重启或关机时保存配置
* \param 无
* \return 无
*/
void GWidget::shutdown_save_infos()
{
    //判断是否温控设置已修改
    if(d_pool->fileOpType==GOverView::FOT_Open && d_pool->run_method_edited){
        My_MessageBox mb;
        connect(&mb, SIGNAL(finished(int)), d_pool, SLOT(screen_sound()));
        d_pool->set_ui_busy_state(1);
        if(mb.gquestion(NULL, NULL, tr("Prompt"), tr("The run settings have been modified, do you want to save?")) == 0){
            d_pool->expFile->save();
        }
        d_pool->run_method_edited = false;
        mb.disconnect(d_pool);
    }

    saveConfInfo();
    clearTmpDirectory();
}

/*!
* \brief 私有槽函数，实现实验的启动和暂停
* \param 无
* \return 无
*/
void GWidget::on_buttonStart_Pause_clicked()
{
    d_pool->key_sound();

    _PCR_RUN_CTRL_INFO pcrInfo;
    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();    

    qDebug() << Q_FUNC_INFO << "当前仪器状态：" << pcrInfo.State.ExpState << ", 样本仓位置：" << pcrInfo.State.DrvDrawerPos << ", 热盖位置：" << pcrInfo.State.DrvLidPos << ", 电机运动状态：" << pcrInfo.State.DrvMotoBusy;
    if(pcrInfo.State.DrvMotoBusy) return;
    qDebug() << Q_FUNC_INFO << "启动/暂停";

#if (!defined DEVICE_TYPE_TL22)
    if(pcrInfo.State.ExpState != 1){
        QString promptMsg;
#if (defined DEVICE_TYPE_TL12)
        bool open_pos =pcrInfo.State.DrvDrawerPos;
        promptMsg = tr("Please close the drawer !");
#else
        bool open_pos =pcrInfo.State.DrvLidPos;
        promptMsg = tr("Please closed heated lid !");
#endif

        if(!open_pos){
            alarm3Sound();
            My_MessageBox mb;            
            mb.gwarning(d_pool, NULL, tr("Warning"), promptMsg, tr("Ok"));
            return;
        }
    }
#endif

    ui->buttonStart_Pause->setEnabled(false);
    ui->buttonStop->setEnabled(false);
#ifdef DEVICE_TYPE_TL22
    ui->buttonEject->setEnabled(false);
#endif
    ui->buttonExit->setEnabled(false);

    QByteArray dat;
    dat.resize(sizeof(quint16));
    quint16 currentCmd = 0;
    if(pcrInfo.State.ExpState == 1){
        //提示是否暂停实验
        this->setProperty("message_is_showing", true);
        My_MessageBox mb;       
        if(mb.gquestion(d_pool, NULL, tr("Inquiry"), tr("Are you sure to pause the experiment?"), tr("Pause"), tr("Cancel")) == 1){
            this->setProperty("message_is_showing", false);
            ui->buttonStart_Pause->setEnabled(true);
            ui->buttonStop->setEnabled(true);
            return;
        }

        _PCR_RUN_CTRL_INFO pcrInfo;
        d_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        d_pool->mutex.unlock();
        qDebug() << "-----00----pcrInfo.State.AmpFlag--  //暂停实验---"<< d_pool->expFile->m_expRunMethod.at(pcrInfo.SizeInfo.No.Stage)->Property;
        if(d_pool->expFile->m_expRunMethod.at(pcrInfo.SizeInfo.No.Stage)->Property!=0) {
            ui->buttonStop->setEnabled(true);
            return;
        }
        if(pcrInfo.State.ExpState != 1) return;
        ui->buttonStop->setEnabled(false);
        //暂停实验
        currentCmd = 2;
        memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));

        sendControlCMD(3, 1, dat);
        emit operatorLog(tr("Pause Experiment"));
        d_pool->circular->showProcess(tr("Pausing"));
    }else{
        currentCmd = 1;
        memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));

        if(pcrInfo.State.ExpState == 0){
            //如果当前文件名为空,退出
            QString fn = d_pool->expFile->fileName().trimmed();
            if(fn.isEmpty()){
                My_MessageBox mb;              
                mb.ginformation(d_pool, NULL, tr("Prompt"), tr("Select a file first."));
                ui->buttonStart_Pause->setEnabled(true);
#ifdef DEVICE_TYPE_TL22
                ui->buttonEject->setEnabled(true);
#endif
                ui->buttonExit->setEnabled(true);
                return;
            }
            //如果当前文件是tlpd文件,提示不能运行
            QFileInfo fileInfo(fn);
            if(fileInfo.suffix().toLower() == tr("tlpd")){
                My_MessageBox mb;               
                mb.gwarning(d_pool, NULL, tr("Warning"), tr("Cannot run file of finished ."));
                ui->buttonStart_Pause->setEnabled(true);
#ifdef DEVICE_TYPE_TL22
                ui->buttonEject->setEnabled(true);
#endif
                ui->buttonExit->setEnabled(true);
                return;
            }

#ifndef DEVICE_TYPE_TL13
            dat.append(fileInfo.fileName().toUtf8());
#else
            QString pathName = fileInfo.filePath();
            pathName.remove(0, QString(SYS_DIR_FILE).size());
            dat.append(pathName.toUtf8());
#endif
        }
        qDebug()<<"22222222222222222222";
        sendControlCMD(2, 1, dat);
        emit operatorLog(tr("Start Experiment"));

        d_pool->circular->showProcess(tr("Starting"));
    }

}

/*!
* \brief 私有槽函数，实现实验的停止
* \param 无
* \return 无
*/
void GWidget::on_buttonStop_clicked()
{
    d_pool->key_sound();
    ui->buttonStop->setEnabled(false);
    ui->buttonStart_Pause->setEnabled(false);
#ifdef DEVICE_TYPE_TL22
    ui->buttonEject->setEnabled(false);
#endif
    //提示是否关闭实验
    this->setProperty("message_is_showing", true);
    My_MessageBox mb;  
    if(mb.gquestion(d_pool, NULL, tr("Inquiry"), tr("Are you sure to stop the experiment?"), tr("Stop"), tr("Cancel")) == 1){
        this->setProperty("message_is_showing", false);

        _PCR_RUN_CTRL_INFO pcrInfo;
        d_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        d_pool->mutex.unlock();

        buttonRefresh(pcrInfo, true);

//        ui->buttonStop->setEnabled(true);
//        ui->buttonStart_Pause->setEnabled(d_pool->expFile->m_expRunMethod.at(pcrInfo.SizeInfo.No.Stage)->Property==0);

//#ifdef DEVICE_TYPE_TL22
//        if( pcrInfo.State.ExpState == 2)
//            ui->buttonEject->setEnabled(true);
//#endif
        return;
    }

    _PCR_RUN_CTRL_INFO pcrInfo;
    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();
    if( pcrInfo.State.ExpState == 0)  return;
    qDebug()<<" Stopping -------------------------------------------";
    //关闭实验
    QByteArray dat;
    dat.resize(sizeof(quint16));
    quint16 currentCmd = 0;
    memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));
    sendControlCMD(1, 1, dat);
    emit operatorLog(tr("Stop Experiment"));
    d_pool->circular->showProcess(tr("Stopping"));
}

#ifdef DEVICE_TYPE_TL22
/*!
* \brief 私有槽函数，实现仓门的打开操作
* \param 无
* \return 无
*/
void GWidget::on_buttonEject_clicked()
{
    d_pool->key_sound();

    _PCR_RUN_CTRL_INFO pcrInfo;
    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();

    if(pcrInfo.State.ExpState == 1 ) return;

    ui->buttonStart_Pause->setEnabled(false);
    ui->buttonStop->setEnabled(false);
#ifdef DEVICE_TYPE_TL22
    ui->buttonEject->setEnabled(false);
#endif

    quint16 driverCtrl = pcrInfo.State.DrvDrawerPos ? 0 : 1;

    QByteArray dat;
    dat.resize(2);
    memcpy((void*)dat.data(), (const void*)&driverCtrl, 2);
    sendControlCMD(4, 10, dat);
}
#endif

/*!
* \brief 私有槽函数，显示最后一次故障或警告信息
* \param 无
* \return 无
*/
void GWidget::on_buttonAlarm_clicked()
{
    d_pool->key_sound();

    if(d_pool->warnFile->infoList.isEmpty()) return;

    My_MessageBox mb;
    GWarningItem item = d_pool->warnFile->infoList.last();
    QString warnTitle = item.type != 0 ? tr("Error") : tr("Warning");
    warnTitle += tr(" information");

    QDateTime dateTime = QDateTime::fromString(item.dateTime,"yyyy_MM_dd-hh_mm_ss");

    QString warnText = tr("Date: %1").arg(dateTime.toString(d_pool->dateFormat+" "+(d_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP")))+"\n";
    warnText += tr("Code: %1").arg(item.code)+"\n";
    warnText += tr("Source: %1").arg(item.src)+"\n";
    warnText += tr("Description: %1").arg(item.describe);

    mb.gwarning(d_pool, NULL, warnTitle, warnText);
}

/*!
* \brief 私有槽函数，实现文件选中或没选中时的状态变化
* \param focus 得到焦点
* \return 无
*/
void GWidget::slot_fileFocus(bool focus)
{
    _PCR_RUN_CTRL_INFO pcrInfo;

    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();

    if(pcrInfo.State.ExpState != 0) return;

    qDebug() << "buttonStart_Pause set" << focus;
    ui->buttonStart_Pause->setEnabled(pcrInfo.State.DrvDrawerPos && !pcrInfo.State.DrvMotoBusy);
}

/*!
* \brief 私有槽函数，实现文件在编辑状态时启动按键的状态变化
* \param edit 是否在编辑温控
* \return 无
*/
void GWidget::slot_editting(bool edit, bool all)
{
    _PCR_RUN_CTRL_INFO pcrInfo;
    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();

    qDebug() << Q_FUNC_INFO << pcrInfo.State.ExpState << edit;
    this->setProperty("inEditting", edit);
    if(edit){
        ui->buttonStart_Pause->setEnabled(false);
        ui->buttonStop->setEnabled(false);
#ifdef DEVICE_TYPE_TL22
        ui->buttonEject->setEnabled(false);
#endif

        if(!d_pool->run_method_edited) d_pool->set_ui_busy_state(1);
    }else{
#if (defined DEVICE_TYPE_TL22)
        if(pcrInfo.State.DrvDrawerPos){
            if(pcrInfo.State.ExpState == 0){
                ui->buttonStart_Pause->setEnabled(true);
                ui->buttonEject->setEnabled(true);
            }else if(!d_pool->circular->isShowing()){
                ui->buttonStart_Pause->setEnabled(pcrInfo.State.ThFlag);
                ui->buttonEject->setEnabled(pcrInfo.State.ExpState == 2);
                ui->buttonStop->setEnabled(true);
            }
        }else if(!d_pool->circular->isShowing()){
            ui->buttonStart_Pause->setEnabled(false);
            ui->buttonStop->setEnabled(false);
            ui->buttonEject->setEnabled(true);
        }
#elif (defined DEVICE_TYPE_TL12)
        if(pcrInfo.State.DrvDrawerPos){
            if(pcrInfo.State.ExpState == 0){
                ui->buttonStart_Pause->setEnabled(true);
            }else if(!d_pool->circular->isShowing()){
                ui->buttonStart_Pause->setEnabled(false);
                ui->buttonStop->setEnabled(true);
            }
        }else if(!d_pool->circular->isShowing()){
            ui->buttonStart_Pause->setEnabled(true);
            ui->buttonStop->setEnabled(false);
        }
#else
        if(pcrInfo.State.ExpState == 0){
            ui->buttonStart_Pause->setEnabled(true);
        }else if(!d_pool->circular->isShowing()){
            ui->buttonStart_Pause->setEnabled(pcrInfo.State.ThFlag);
            ui->buttonStop->setEnabled(true);
        }
#endif
    }

    if(all){
#ifdef DEVICE_TYPE_TL22
        ui->buttonEject->setEnabled(!edit);
#endif
        ui->buttonExit->setEnabled(!edit);
    }
    qDebug() << Q_FUNC_INFO << "end";
}

/*!
* \brief 私有槽函数，实现文件选中后数据池的处理
* \param fileName 当前选择的实验文件名
* \return 无
*/
void GWidget::slot_fileChanged(const QString &fileName, int type){
    qDebug() << "File changed :" << fileName << type;
    qDebug() << "File changed --------000000000-----------:"<< d_pool->expFile->fileName()<<m_d->openFile;
    //如果没有实验文件,退出
    if(d_pool->expFile == NULL) return;

    //如果实验正在运行,退出
    _PCR_RUN_CTRL_INFO pcrInfo;

    d_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    d_pool->mutex.unlock();

    if(pcrInfo.State.ExpState != 0) return;

    //如果是启动时,要等到延时后再继续运行
    while(d_pool->powerOnSecs < 6){
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
    };
    //      m_d->openFile.clear();
    //如果是删除文件,清空文件存储区,退出
    if(fileName.isEmpty() || QFileInfo(fileName).fileName()==QString("..")){
        d_pool->expFile->clear();
        m_d->openFile.clear();
        m_d->file_edited_time.clear();
        QString titleShow;
#ifdef DEVICE_TYPE_TL13
        //如果是在子目录中
        if(d_pool->filePath.size() > QString(SYS_DIR_FILE).size()){
            titleShow = d_pool->filePath.right(d_pool->filePath.size()-QString(SYS_DIR_FILE).size());
        }
#endif
        setFileInfomation(titleShow);
        emit updateExp();
        d_pool->run_method_edited = false;
        d_pool->fileOpType = 0;
        return;
    }

    if(type > GOverView::FOT_Clear){
        d_pool->fileOpType = type;
    }
#ifdef DEVICE_TYPE_TL13
    QString pathName = fileName;
    pathName.remove(0, QString(SYS_DIR_FILE).size());

    QFileInfo fileInfo(fileName);
    if(fileInfo.isDir()){
        setFileInfomation(QFileInfo(pathName).fileName() + QString("/") );

        d_pool->expFile->clear();
        m_d->openFile.clear();
        m_d->file_edited_time.clear();
        d_pool->run_method_edited = false;
        d_pool->fileOpType = 0;
    }else{
        QFileInfo fi(pathName);
        if(fi.path() == QString(".")){
            setFileInfomation(fi.completeBaseName());
        }else{
            setFileInfomation(fi.path() + QString("/") + fi.completeBaseName());
        }
        d_pool->expFile->setFile(fileName);
    }
#else
    QFileInfo fileInfo(fileName);
    setFileInfomation(fileInfo.completeBaseName());
    d_pool->expFile->setFile(fileName);
#endif    
}

/*!
* \brief 私有槽函数，新建文件后自动转到运行设置界面
* \param 无
* \return 无
*/
void GWidget::slot_nextWidget()
{
    ui->tabWidgetMain->setCurrentIndex(sw_RunEditor);

    GRunEditor *re = (GRunEditor *)(ui->tabWidgetMain->widget(sw_RunEditor));
    re->showFirstTab();
}

/*!
* \brief 私有槽函数，编辑运行界面设置时，使Tab无效
* \param enable 是否有效
* \return 无
*/
void GWidget::slot_runeditor_tabDisabled(bool disable)
{
    for(int i=0; i<ui->tabWidgetMain->count(); i++){
        if(i != sw_RunEditor)
            ui->tabWidgetMain->setTabEnabled(i, !disable);
    }

    slot_editting(disable);
}

/*!
* \brief 私有槽函数，运行设置界面的内容已修改
* \param 无
* \return 无
*/
void GWidget::slot_runMethod_edited()
{
    if(!d_pool->run_method_edited) d_pool->set_ui_busy_state(1);
    d_pool->run_method_edited = true;
}

/*!
* \brief 私有槽函数，通用设置界面设置时，使Tab无效
* \param enable 是否有效
* \return 无
*/
void GWidget::slot_utilities_tabDisabled(bool disable)
{
    int tab_index_ = sw_Utilities;

    for(int i=0; i<ui->tabWidgetMain->count(); i++){
        if(i != tab_index_)
            ui->tabWidgetMain->setTabEnabled(i, !disable);
    }
    slot_editting(disable);
}

/*!
* \brief 私有槽函数，通用设置界面USB设备插拔时的操作
* \param 无
* \return 无
*/
void GWidget::slot_usbDeviceChanged()
{
    ui->labelUsb->setVisible(d_pool->usbMaps.count() > 0);
}

/*!
* \brief 私有槽函数，校准触摸屏后刷新显示界面
* \param log 操作记录
* \return 无
*/
void GWidget::slot_refreshScreen()
{
    this->repaint();
}

/*!
* \brief 私有槽函数，通用设置界面左下角操作记录框
* \param log 操作记录
* \return 无
*/
void GWidget::slot_operatorLog(const QString& log, int colorId)
{
    //去掉顶部的信息
#ifdef DEVICE_TYPE_TL22
    if(ui->textEditOperate->document()->lineCount() >= 3){
        QTextCursor txtcur= ui->textEditOperate->textCursor();
        txtcur.setPosition(0);
        txtcur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
        txtcur.movePosition(QTextCursor::Down,QTextCursor::KeepAnchor);
        txtcur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
        txtcur.removeSelectedText();
    }
#endif
    QString txt = "> " + log;

    qDebug()<<"--------文件中添加操作信息-------"<<d_pool->operatorLogFile;
    //向文件中添加操作信息
    QFile file(d_pool->operatorLogFile);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") << txt << "\r\n";
        file.close();
    }

    switch(colorId){
    case 1:txt = "<font color=red>" + txt + "</font>";break;
    case 2:txt = "<font color=blue>" + txt + "</font>";break;
    default:txt = "<font color=#383e83>" + txt + "</font>";break;
    }
#ifdef DEVICE_TYPE_TL22
    ui->textEditOperate->append(txt);
#endif
}

/*!
* \brief 私有槽函数，读取数据
* \param 无
* \return 无
*/
void GWidget::g_dealReadData(quint8 type1, quint16 type2, const QByteArray &dat)
{
    if(m_d->verfalg<2){
        version();
        m_d->verfalg++;
    }
    if(this->property("showMessageBox").toBool()) return;
    qDebug() << "-----" << QTime::currentTime().toString("hh:mm:ss") << "get packet data:" << type1 << type2 << GHelper::byteArrayToHexStr(dat);

    switch(type1){
    case 1:   {
        switch(type2)
        {
        case 1:
        {
            _PCR_RUN_CTRL_INFO pcrInfo, prevInfo;
            int len = pcrInfo._size();
            memcpy( (void*)(&pcrInfo), (void*)(dat.data()), len);

            qDebug() << "Instrument run state:" << pcrInfo.State.ExpState << pcrInfo.State.DrvMotoBusy << pcrInfo.State.ThFlag << pcrInfo.SizeInfo.No.Stage << pcrInfo.SizeInfo.No.Step;

            bool readFluor = false;
            bool stateChanged = false;
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
            bool drawerMoved = false;
#endif
            bool isFirst = false;
#if (!defined DEVICE_TYPE_TL22) && (!defined DEVICE_TYPE_TL12)
            bool hotlibopen = false;
#endif
            if(pcrInfo.State.DrvLockSoft == 0 && pcrInfo.State.DrvLockMechanical == 0)
            {
                //如果仪器已解锁
                //初始化延时2s
                if(pcrInfo.State.ExpState==0){
                    if(d_pool->powerOnSecs < 6){
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
                        if((pcrInfo.State.OpHeadMoveState==0) && (pcrInfo.State.DrvMotoBusy==0)){
#else
                        if((pcrInfo.State.OpHeadMoveState==0)){
#endif
                            d_pool->powerOnSecs ++;
                        }
                        return;
                    }
                }else{
                    d_pool->powerOnSecs = 6;
                }

                if(d_pool->powerOnSecs <= 6){
                    d_pool->powerOnSecs = 6 + 1;

                    if(!ui->tabWidgetMain->isEnabled())
                        ui->tabWidgetMain->setEnabled(true);

#ifndef DEVICE_TYPE_TL22
                    if(!ui->buttonStart_Pause->isEnabled())
                        ui->tabWidgetMain->setEnabled(true);
#endif
                }

                d_pool->mutex.lock();
                if(d_pool->runCtrlInfo.isEmpty()){
                    //如果是第一次得到仪器信息,申请空间,初始化清零
                    isFirst = true;
                    d_pool->runCtrlInfo.resize(len);
                    memset((void*)d_pool->runCtrlInfo.data(), '\0', len);
                }else if(d_pool->runCtrlInfo.count() != len){
                    //如果空间大小不对,保存此次状态,退出
                    d_pool->runCtrlInfo.resize(len);
                    memcpy((void*)d_pool->runCtrlInfo.data(), (const void*)(dat.data()), len);
                    return;
                }

                //上次保存的数据
                memcpy((void*)&prevInfo, (const void*)(d_pool->runCtrlInfo.data()), len);

                qDebug() << "prev run state :" << prevInfo.State.ExpState << prevInfo.State.DrvLidPos << prevInfo.State.OpHeadMoveState;

                //如果有故障
                if(pcrInfo.State.Error){
                    qDebug() << "*************Instrument failure****************";

                    //显示仪器状态
                    updateStateInfo(7);
                    ui->buttonAlarm->setIcon(QIcon(":/png/status3"));
                    if(!ui->buttonAlarm->isEnabled())
                        ui->buttonAlarm->setEnabled(true);

                    if(prevInfo.State.Error == 0){
                        QString errTxt = tr("Error:");
                        int errSize = errTxt.size();
                        if(pcrInfo.ErrorInfo.Arm.Errorstate){
                            errTxt += tr("%1(Arm)").arg(pcrInfo.ErrorInfo.Arm.ErrorCode);
                            QString errInfo = QString::number(pcrInfo.ErrorInfo.Arm.ErrorCode);
                            if(m_d->errorInfos.contains(d_pool->current_lan) && m_d->errorInfos.value(d_pool->current_lan)->contains(KEY_ARM)){
                                if(m_d->errorInfos.value(d_pool->current_lan)->value(KEY_ARM)->contains(pcrInfo.ErrorInfo.Arm.ErrorCode)){
                                    errInfo = m_d->errorInfos.value(d_pool->current_lan)->value(KEY_ARM)->value(pcrInfo.ErrorInfo.Arm.ErrorCode);
                                }
                            }
                            errTxt += ", " + errInfo;

                            GWarningItem item;
                            item.type = 1;
                            item.src = tr("Arm");
                            item.code = QString::number(pcrInfo.ErrorInfo.Arm.ErrorCode);
                            item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                            item.describe = errInfo;
                            d_pool->warnFile->append(item);
                        }
                        if(pcrInfo.ErrorInfo.Optical.Errorstate){
                            if(errTxt.size() > errSize) errTxt += tr(", ");
                            errTxt += tr("%1(Optical)").arg(pcrInfo.ErrorInfo.Optical.ErrorCode);
                            QString errInfo = QString::number(pcrInfo.ErrorInfo.Optical.ErrorCode);
                            if(m_d->errorInfos.contains(d_pool->current_lan) && m_d->errorInfos.value(d_pool->current_lan)->contains(KEY_OPTICAL)){
                                if(m_d->errorInfos.value(d_pool->current_lan)->value(KEY_OPTICAL)->contains(pcrInfo.ErrorInfo.Optical.ErrorCode)){
                                    errInfo = m_d->errorInfos.value(d_pool->current_lan)->value(KEY_OPTICAL)->value(pcrInfo.ErrorInfo.Optical.ErrorCode);
                                }
                            }
                            errTxt += ", " + errInfo;

                            GWarningItem item;
                            item.type = 1;
                            item.src = tr("Optical");
                            item.code = QString::number(pcrInfo.ErrorInfo.Optical.ErrorCode);
                            item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                            item.describe = errInfo;
                            d_pool->warnFile->append(item);
                        }
                        if(pcrInfo.ErrorInfo.Thermal.Errorstate){
                            if(errTxt.size() > errSize) errTxt += tr(", ");
                            errTxt += tr("%1(Thermal)").arg(pcrInfo.ErrorInfo.Thermal.ErrorCode);
                            QString errInfo = QString::number(pcrInfo.ErrorInfo.Thermal.ErrorCode);
                            if(m_d->errorInfos.contains(d_pool->current_lan) && m_d->errorInfos.value(d_pool->current_lan)->contains(KEY_THERMAL)){
                                if(m_d->errorInfos.value(d_pool->current_lan)->value(KEY_THERMAL)->contains(pcrInfo.ErrorInfo.Thermal.ErrorCode)){
                                    errInfo = m_d->errorInfos.value(d_pool->current_lan)->value(KEY_THERMAL)->value(pcrInfo.ErrorInfo.Thermal.ErrorCode);
                                }
                            }
                            errTxt += ", " + errInfo;

                            GWarningItem item;
                            item.type = 1;
                            item.src = tr("Thermal");
                            item.code = QString::number(pcrInfo.ErrorInfo.Thermal.ErrorCode);
                            item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                            item.describe = errInfo;
                            d_pool->warnFile->append(item);
                        }
                        if(pcrInfo.ErrorInfo.Driver.Errorstate){
                            if(errTxt.size() > errSize) errTxt += tr(", ");
                            errTxt += tr("%1(Driver)").arg(pcrInfo.ErrorInfo.Driver.ErrorCode);
                            QString errInfo = QString::number(pcrInfo.ErrorInfo.Driver.ErrorCode);
                            if(m_d->errorInfos.contains(d_pool->current_lan) && m_d->errorInfos.value(d_pool->current_lan)->contains(KEY_DRIVER)){
                                if(m_d->errorInfos.value(d_pool->current_lan)->value(KEY_DRIVER)->contains(pcrInfo.ErrorInfo.Driver.ErrorCode)){
                                    errInfo = m_d->errorInfos.value(d_pool->current_lan)->value(KEY_DRIVER)->value(pcrInfo.ErrorInfo.Driver.ErrorCode);
                                }
                            }
                            errTxt += ", " + errInfo;

                            GWarningItem item;
                            item.type = 1;
                            item.src = tr("Driver");
                            item.code = QString::number(pcrInfo.ErrorInfo.Driver.ErrorCode);
                            item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                            item.describe = errInfo;
                            d_pool->warnFile->append(item);
                        }

                        slot_operatorLog(errTxt, 1);
                        d_pool->alarm_sound();
                    }
#ifdef DEVICE_TYPE_TL22
                    bool visible = ui->labelError->isVisible();
                    ui->labelError->setVisible(!visible);
#endif
                }else{
                    //显示仪器状态
                    updateStateInfo(4);
                    if(!pcrInfo.State.Warning){
                        ui->buttonAlarm->setIcon(QIcon(":/png/status1"));
                        if(ui->buttonAlarm->isEnabled())
                            ui->buttonAlarm->setEnabled(false);
                    }

                    if(prevInfo.State.Error){
                        slot_operatorLog(tr("Trouble shooting"), 2);
                        d_pool->end_sound();
                    }
#ifdef DEVICE_TYPE_TL22
                    if(ui->labelError->isVisible())
                        ui->labelError->setVisible(false);
#endif
                }

                //如果有警告
                if(pcrInfo.State.Warning && prevInfo.State.Warning == 0){
                    qDebug() << "*************Instrument warning****************";

                    ui->buttonAlarm->setIcon(QIcon(":/png/status2"));
                    if(!ui->buttonAlarm->isEnabled())
                        ui->buttonAlarm->setEnabled(true);

                    QString warnTxt = tr("Warning:");
                    int warnSize = warnTxt.size();
                    if(pcrInfo.ErrorInfo.Arm.WarningState){
                        warnTxt += tr("%1(Arm)").arg(pcrInfo.ErrorInfo.Arm.WarningCode);
                        QString warnInfo = QString::number(pcrInfo.ErrorInfo.Arm.WarningCode);
                        if(m_d->warnInfos.contains(d_pool->current_lan) && m_d->warnInfos.value(d_pool->current_lan)->contains(KEY_ARM)){
                            if(m_d->warnInfos.value(d_pool->current_lan)->value(KEY_ARM)->contains(pcrInfo.ErrorInfo.Arm.WarningCode)){
                                warnInfo = m_d->warnInfos.value(d_pool->current_lan)->value(KEY_ARM)->value(pcrInfo.ErrorInfo.Arm.WarningCode);
                            }
                        }
                        warnTxt += ", " + warnInfo;

                        GWarningItem item;
                        item.type = 0;
                        item.src = tr("Arm");
                        item.code = QString::number(pcrInfo.ErrorInfo.Arm.WarningCode);
                        item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                        item.describe = warnInfo;
                        d_pool->warnFile->append(item);
                    }
                    if(pcrInfo.ErrorInfo.Optical.WarningState){
                        if(warnTxt.size() > warnSize) warnTxt += tr(", ");
                        warnTxt += tr("%1(Optical)").arg(pcrInfo.ErrorInfo.Optical.WarningCode);
                        QString warnInfo = QString::number(pcrInfo.ErrorInfo.Optical.WarningCode);
                        if(m_d->warnInfos.contains(d_pool->current_lan) && m_d->warnInfos.value(d_pool->current_lan)->contains(KEY_OPTICAL)){
                            if(m_d->warnInfos.value(d_pool->current_lan)->value(KEY_OPTICAL)->contains(pcrInfo.ErrorInfo.Optical.WarningCode)){
                                warnInfo = m_d->warnInfos.value(d_pool->current_lan)->value(KEY_OPTICAL)->value(pcrInfo.ErrorInfo.Optical.WarningCode);
                            }
                        }
                        warnTxt += ", " + warnInfo;

                        GWarningItem item;
                        item.type = 0;
                        item.src = tr("Optical");
                        item.code = QString::number(pcrInfo.ErrorInfo.Optical.WarningCode);
                        item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                        item.describe = warnInfo;
                        d_pool->warnFile->append(item);
                    }
                    if(pcrInfo.ErrorInfo.Thermal.WarningState){
                        if(warnTxt.size() > warnSize) warnTxt += tr(", ");
                        warnTxt += tr("%1(Thermal)").arg(pcrInfo.ErrorInfo.Thermal.WarningCode);
                        QString warnInfo = QString::number(pcrInfo.ErrorInfo.Thermal.WarningCode);
                        if(m_d->warnInfos.contains(d_pool->current_lan) && m_d->warnInfos.value(d_pool->current_lan)->contains(KEY_THERMAL)){
                            if(m_d->warnInfos.value(d_pool->current_lan)->value(KEY_THERMAL)->contains(pcrInfo.ErrorInfo.Thermal.WarningCode)){
                                warnInfo = m_d->warnInfos.value(d_pool->current_lan)->value(KEY_THERMAL)->value(pcrInfo.ErrorInfo.Thermal.WarningCode);
                            }
                        }
                        warnTxt += ", " + warnInfo;

                        GWarningItem item;
                        item.type = 0;
                        item.src = tr("Thermal");
                        item.code = QString::number(pcrInfo.ErrorInfo.Thermal.WarningCode);
                        item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                        item.describe = warnInfo;
                        d_pool->warnFile->append(item);
                    }
                    if(pcrInfo.ErrorInfo.Driver.WarningState){
                        if(warnTxt.size() > warnSize) warnTxt += tr(", ");
                        warnTxt += tr("%1(Driver)").arg(pcrInfo.ErrorInfo.Driver.WarningCode);
                        QString warnInfo = QString::number(pcrInfo.ErrorInfo.Driver.WarningCode);
                        if(m_d->warnInfos.contains(d_pool->current_lan) && m_d->warnInfos.value(d_pool->current_lan)->contains(KEY_DRIVER)){
                            if(m_d->warnInfos.value(d_pool->current_lan)->value(KEY_DRIVER)->contains(pcrInfo.ErrorInfo.Driver.WarningCode)){
                                warnInfo = m_d->warnInfos.value(d_pool->current_lan)->value(KEY_DRIVER)->value(pcrInfo.ErrorInfo.Driver.WarningCode);
                            }
                        }
                        warnTxt += ", " + warnInfo;

                        GWarningItem item;
                        item.type = 0;
                        item.src = tr("Driver");
                        item.code = QString::number(pcrInfo.ErrorInfo.Driver.WarningCode);
                        item.dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                        item.describe = warnInfo;
                        d_pool->warnFile->append(item);
                    }

                    slot_operatorLog(warnTxt, 1);
                }
                //得到是否读荧光值信息
                readFluor = prevInfo.State.OpDataReady != pcrInfo.State.OpDataReady;
                //得到仪器状态变化的信息
                stateChanged = prevInfo.State.ExpState != pcrInfo.State.ExpState;
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
                //得到样本仓运动信息
                drawerMoved = prevInfo.State.DrvDrawerPos != pcrInfo.State.DrvDrawerPos;
#else
                hotlibopen = !pcrInfo.State.DrvLidPos;
#endif                
                //保存此次状态
                memcpy((void*)d_pool->runCtrlInfo.data(), (const void*)(dat.data()), len);
                d_pool->info_refresh_flag = 1;

                d_pool->mutex.unlock();

                m_d->hotcover = pcrInfo.Temp[4]/100.0;
            }else{      //如果没有解锁，提示

                qDebug() << "-- instrument lock" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << pcrInfo.State.DrvLockSoft << pcrInfo.State.DrvLockMechanical;

                //关闭读取仪器状态命令
                killTimer(m_d->commId);
                m_d->commId = 0;
                this->setProperty("showMessageBox", true);

                //显示仪器状态
                updateStateInfo(9);

                GTransportUnlockWizard wizard(d_pool, this);
                wizard.exec();

                this->setProperty("showMessageBox", false);
                m_d->commId = startTimer(SOCKET_INTERVAL);
                return;
            }

            //更新系统信息
            if(pcrInfo.State.Error){
                if(m_d->sysState != 7){
                    updateStateInfo(7);
                }

                //记录自检结果
                if(d_pool->selfCheckRptFile && d_pool->selfCheckRptFile->checkResult){
                    d_pool->selfCheckRptFile->checkResult = false;
                    d_pool->selfCheckRptFile->dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                    //                    d_pool->selfCheckRptFile->analytical = true;
                    d_pool->selfCheckRptFile->thermodynamics = pcrInfo.ErrorInfo.Thermal.WarningState;
                    d_pool->selfCheckRptFile->coverDrive = pcrInfo.ErrorInfo.Driver.WarningState;
                    d_pool->selfCheckRptFile->photometer = pcrInfo.ErrorInfo.Optical.WarningState;
                    d_pool->selfCheckRptFile->saveReport();

                    int tab_index_ = sw_Utilities;
                    GUtilities *us = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
                    if(us) us->updateSelfTest();
                }
            }else{
                if(pcrInfo.State.ExpState==0 && m_d->sysState!=4){
                    updateStateInfo(4);
                }else if(pcrInfo.State.ExpState==1 && m_d->sysState!=5){
                    updateStateInfo(5);
                }else if(pcrInfo.State.ExpState==2 && m_d->sysState!=6){
                    updateStateInfo(6);
                }else if(pcrInfo.State.ExpState== 3 && m_d->sysState!=5){
                    updateStateInfo(5);
                }

                //记录自检结果
                if(d_pool->selfCheckRptFile && !d_pool->selfCheckRptFile->checkResult){
                    d_pool->selfCheckRptFile->checkResult = true;
                    d_pool->selfCheckRptFile->dateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
                    //                    d_pool->selfCheckRptFile->analytical = true;
                    d_pool->selfCheckRptFile->thermodynamics = true;
                    d_pool->selfCheckRptFile->coverDrive = true;
                    d_pool->selfCheckRptFile->photometer = true;
                    d_pool->selfCheckRptFile->saveReport();

                    int tab_index_ = sw_Utilities;
                    GUtilities *us = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
                    if(us) us->updateSelfTest();
                }
            }

            //如果是第一次执行
            if(isFirst){
                //按键初始化
                buttonRefresh(pcrInfo);
                //如果仪器正在运行，显示当前运行的文件名
                if(pcrInfo.State.ExpState != 0){
                    if(ui->tabWidgetMain->isEnabled())
                        ui->tabWidgetMain->setEnabled(false);
                    this->setProperty("reboot", true);
                    //查询当前运行的文件名
                    if(d_pool->isInitialzied()){
                        d_pool->WriteData(1, 14);
                    }
                    qDebug()<<"--- 启动时仪器正在运行，显示断电前正在运行的文件名 ---";
                    if(!ui->tabWidgetMain->isEnabled())
                        ui->tabWidgetMain->setEnabled(true);
                }
                return;
            }
#ifdef DEVICE_TYPE_TL22
            if(pcrInfo.State.ExpState==1 && ui->buttonStop->isEnabled()){
                ui->buttonStart_Pause->setEnabled(pcrInfo.State.ThFlag);
            }

            if(pcrInfo.State.DrvMotoBusy==1 && pcrInfo.State.ExpState!=1){
                ui->buttonStart_Pause->setEnabled(false);
                ui->buttonStop->setEnabled(false);
                ui->buttonEject->setEnabled(false);
            }else if(pcrInfo.State.ExpState==0 && pcrInfo.State.DrvMotoBusy==0&& !this->property("inEditting").toBool()){
                ui->buttonStart_Pause->setEnabled(true);
                ui->buttonEject->setEnabled(true);
            }

            qDebug() << "pcrInfo.State.SyncFlag------"<<pcrInfo.State.SyncFlag;
            //更新样本仓信息1
            if(drawerMoved){
                if(!this->property("WaitForDrawerClose").toBool()){
                    if(ui->buttonEject->isEnabled())
                        ui->buttonEject->setEnabled(false);
                }
                if(pcrInfo.State.DrvDrawerPos){
                    qDebug() << "--- 抽屉已关闭 ---"<<pcrInfo.State.DrvDrawerPos;
                    this->setProperty("WaitForDrawerClose", true);
                    if (pcrInfo.State.ExpState==0){
                        ui->buttonStart_Pause->setEnabled(true);
                        ui->buttonStop->setEnabled(false);
                        if(!ui->buttonEject->isEnabled())
                            ui->buttonEject->setEnabled(true);
                    }else if (pcrInfo.State.ExpState==2&&pcrInfo.State.DrvMotoBusy==0){
                        ui->buttonStart_Pause->setEnabled(true);
                        ui->buttonStop->setEnabled(true);
                        if(!ui->buttonEject->isEnabled())
                            ui->buttonEject->setEnabled(true);
                    }
                }else{
                    qDebug() << "--- 抽屉已打开 ---";
                    if(!ui->buttonEject->isEnabled())
                        ui->buttonEject->setEnabled(true);
                    emit operatorLog(tr("%1 Loading Platform").arg(pcrInfo.State.DrvDrawerPos?tr("Close"):tr("Open")));
                }
            }
            qDebug() << "---------- 1 -------------------";
            //更新样本仓信息2
            if(this->property("WaitForDrawerClose").toBool() && pcrInfo.State.DrvLidPos){
                this->setProperty("WaitForDrawerClose", false);
                qDebug() << "drawer moved 2";
                if(!ui->buttonEject->isEnabled())
                    ui->buttonEject->setEnabled(true);
                if(pcrInfo.State.DrvDrawerPos){
                    if(pcrInfo.State.ExpState==2){
                        if(!ui->buttonStart_Pause->isEnabled())
                            ui->buttonStart_Pause->setEnabled(true);
                        ui->buttonStop->setEnabled(true);
                    }
                }
                emit operatorLog(tr("%1 Loading Platform").arg(pcrInfo.State.DrvDrawerPos?tr("Close"):tr("Open")));
            }
            qDebug() << "---------- 2 -------------------";
#elif (defined DEVICE_TYPE_TL12)
            if(pcrInfo.State.ExpState==1 && ui->buttonStop->isEnabled()){
                ui->buttonStart_Pause->setEnabled(pcrInfo.State.ThFlag);
            }

            if(pcrInfo.State.DrvMotoBusy==1 && pcrInfo.State.ExpState!=1){
                ui->buttonStart_Pause->setEnabled(false);
                ui->buttonStop->setEnabled(false);
            }

            //更新样本仓信息1
            if(drawerMoved){
                ui->labellidopen->setVisible(!pcrInfo.State.DrvDrawerPos);

                if(pcrInfo.State.DrvMotoBusy==1){
                    if(pcrInfo.State.DrvDrawerPos){
                        this->setProperty("WaitForDrawerClose", true);
                        if (pcrInfo.State.ExpState==0){
                            ui->buttonStart_Pause->setEnabled(true);
                            ui->buttonStop->setEnabled(false);
                        }
                    }else{
                        emit operatorLog(tr("%1 Loading Platform").arg(pcrInfo.State.DrvDrawerPos?tr("Close"):tr("Open")));
                    }
                }
            }
            //更新样本仓信息2
           if(this->property("WaitForDrawerClose").toBool() && pcrInfo.State.DrvLidPos){
                this->setProperty("WaitForDrawerClose", false);
                if(pcrInfo.State.DrvDrawerPos){
                    if(pcrInfo.State.ExpState==2){
                        if(!ui->buttonStart_Pause->isEnabled())
                            ui->buttonStart_Pause->setEnabled(true);
                        ui->buttonStop->setEnabled(true);
                    }
                }

                emit operatorLog(tr("%1 Loading Platform").arg(pcrInfo.State.DrvDrawerPos?tr("Close"):tr("Open")));
            }
#else
            if(pcrInfo.State.ExpState!=0 && hotlibopen) d_pool->general_sound();
            ui->labellidopen->setVisible(hotlibopen);
#endif
            //如果荧光值数据准备好
            if(readFluor && d_pool->isInitialzied()){
                d_pool->WriteData(1, 12);
                d_pool->readingFluor = true;
            }
            qDebug() << "---------- 3 -------------------" << stateChanged << d_pool->commandVule;

            //如果仪器状态发生变化
            if(stateChanged && (d_pool->commandVule<0 || d_pool->commandVule==pcrInfo.State.ExpState/* || !ui->buttonStop->isEnabled()*/)){
                d_pool->commandVule = -1;
                qDebug() << "state changed";
                //刷新按键显示，如果点击停止实验，但是热盖还在关闭状态，则等待热盖打开后再改变按键状态
                if(pcrInfo.State.ExpState==0 && pcrInfo.State.DrvLidPos){
                    this->setProperty("WaitForLidOpen", true);
                    if(ui->buttonStart_Pause->isEnabled())
                        ui->buttonStart_Pause->setEnabled(false);
                    if(ui->buttonStop->isEnabled())
                        ui->buttonStop->setEnabled(false);
                }else if(pcrInfo.State.ExpState==1 && pcrInfo.State.DrvLidPos==0){
                    this->setProperty("WaitForLidClose", true);
                    if(ui->buttonStart_Pause->isEnabled())
                        ui->buttonStart_Pause->setEnabled(false);
                    if(ui->buttonStop->isEnabled())
                        ui->buttonStop->setEnabled(false);
                }else{
                    //如果使用了提示，恢复按键可按状态
                    if(d_pool->circular->isShowing()){
                        d_pool->circular->hideProcess();
                        switch(d_pool->commandType){
                        case 3:
                            this->setProperty("message_is_showing", false);
                        case 2:
                            if(!ui->buttonStart_Pause->isEnabled())
                                ui->buttonStart_Pause->setEnabled(true);
                        case 1 :
                            this->setProperty("message_is_showing", false);
                            if(ui->buttonStart_Pause->isEnabled())
                                ui->buttonStart_Pause->setEnabled(false);                           
                            break;
                        default:break;
                        }
                    }
                    buttonRefresh(pcrInfo);
                }

                qDebug() << "---------- 4 -------------------";
                if(pcrInfo.State.ExpState == 1){
                    if(d_pool->isInitialzied()){
                        d_pool->WriteData(1, 14);
                    }
                }else{
                    if(pcrInfo.State.ExpState == 0){

                        if(ui->buttonStart_Pause->isEnabled())
                            ui->buttonStart_Pause->setEnabled(false);
                        if(ui->buttonStop->isEnabled())
                            ui->buttonStop->setEnabled(false);

                        d_pool->fileOpType = GOverView::FOT_Open;

                        qDebug()<< " //如果是实验结束，刷新实验文件信息   is   ----------------------ok?";
                        //如果是实验结束，刷新实验文件信息
                        pcrInfo.SizeInfo.No.Cycle=0;
                        pcrInfo.SizeInfo.No.Stage=0;
                        pcrInfo.SizeInfo.No.Step=0;
                        pcrInfo.SizeInfo.ExpTime.CurrentTimes=0;
                        qDebug() << "------------实验结束---------";
                        GOverView *ov = (GOverView *)(ui->tabWidgetMain->widget(sw_OverView));
                        ov->experimentState(false);

                        int tab_index_ = sw_Utilities;
                        GUtilities *us = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
                        us->experimentState(false);
                        //
                        if(!d_pool->expFile->fileName().isEmpty()){
                            QFileInfo fileInfo(d_pool->expFile->fileName());
                            if(fileInfo.suffix().toLower() == tr("tlpp") && !QFile::exists(fileInfo.absoluteFilePath())){
                                QString fn = QDir::toNativeSeparators(fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".tlpd");
                                if(QFile::exists(fn)){
                                    d_pool->expFile->setFile(fn);
                                    m_d->openFile = fn;
                                    m_d->file_edited_time = QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss");
                                    setFileInfomation(QFileInfo(fn).completeBaseName());
                                    qDebug() << "experiment end, and file change to" << fn;
                                }
                            }
                        }
                        alarm3Sound();
                    }

                    //发送状态变化信号
                    emit experimentCtrl(pcrInfo.State.ExpState);
                }
            }

            qDebug() << "---------- 5 -------------------";
            //如果有等待热盖运动
            if(this->property("WaitForLidOpen").toBool() && pcrInfo.State.DrvLidPos == 0){
                this->setProperty("WaitForLidOpen", false);
                //如果使用了提示，恢复按键可按状态
                if(d_pool->circular->isShowing()){
                    d_pool->circular->hideProcess();
                    this->setProperty("message_is_showing", false);
                }
                buttonRefresh(pcrInfo);
            }else if(this->property("WaitForLidClose").toBool() && pcrInfo.State.DrvLidPos){
                this->setProperty("WaitForLidClose", false);
                //如果使用了提示，恢复按键可按状态
                if(d_pool->circular->isShowing()){
                    d_pool->circular->hideProcess();
                    this->setProperty("message_is_showing", false);
                }
                buttonRefresh(pcrInfo);
            }

            qDebug() << "---------- 6 -------------------";
            //热盖图标，判断是否使用热盖，如果使用，显示热盖并开始闪烁
            if(pcrInfo.State.ExpState != 0){
                if(d_pool->expFile->m_expRunInfo.lidStatus==0 || (pcrInfo.State.DrvLidTCtrl==0)){
                    if(m_d->hotlidId){
                        killTimer(m_d->hotlidId);
                        m_d->hotlidId = 0;
                    }

                    //显示关闭热盖
                    if(ui->labelHotlid->property("using").toBool()){
                        ui->labelHotlid->setPixmap(QPixmap(":/png/hotlid_unused"));
                        ui->labelHotlid->setProperty("using",false);
                    }
                    if(!ui->labelHotlid->isVisible())
                        ui->labelHotlid->setVisible(true);
                }else if (pcrInfo.SizeInfo.No.Cycle==0||pcrInfo.State.DrvLidTCtrl==1){
                    if(!ui->labelHotlid->property("using").toBool()){
                        ui->labelHotlid->setPixmap(QPixmap(":/png/hotlid_using"));
                        ui->labelHotlid->setProperty("using",true);
                    }

                    if(pcrInfo.State.DrvLidKeep){
                        if(m_d->hotlidId){
                            killTimer(m_d->hotlidId);
                            m_d->hotlidId = 0;
                        }
                        if(!ui->labelHotlid->isVisible())
                            ui->labelHotlid->setVisible(true);
                    }else{
                        if(m_d->hotlidId == 0){
                            ui->labelHotlid->setProperty("count", 1);

                            m_d->hotlidId = startTimer(1010);
                        }
                    }
                }

                if(pcrInfo.SizeInfo.No.Stage==d_pool->infinite_stage_no && pcrInfo.SizeInfo.No.Step==d_pool->infinite_step_no){
                    if(ui->buttonStart_Pause->isEnabled())
                        ui->buttonStart_Pause->setEnabled(false);
                    if(!ui->buttonStop->isEnabled())
                        ui->buttonStop->setEnabled(true);
                }
            }else{
                if(m_d->hotlidId){
                    killTimer(m_d->hotlidId);
                    m_d->hotlidId = 0;
                }
                if(ui->labelHotlid->isVisible())
                    ui->labelHotlid->setVisible(false);
            }
            qDebug() << "---------- 7 -------------------";
            break;
        }
        case 12:{
            if(dat.size() == sizeof(_FLUOR_SCAN_INFO)){
                qDebug() << "-- get fluor scan data --";
#ifndef DEVICE_TYPE_TL13
                emit fluor_scan_info(dat);
#endif
            }else{
                qDebug() << "-- get fluor data size error --" << dat.size();
            }
            d_pool->readingFluor = false;
            break;
        }
        case 13:{
            QByteArray expname(dat.size(),0);
            memcpy((void*)expname.data(), (const void*)(dat.data()), dat.size());
            const QString fn = QString::fromUtf8(expname).trimmed();
            //判断是否温控设置已修改FOT_New          

            if(d_pool->fileOpType==GOverView::FOT_Open && d_pool->run_method_edited){
                My_MessageBox mb;
                connect(&mb, SIGNAL(finished(int)), d_pool, SLOT(screen_sound()));
                d_pool->set_ui_busy_state(1);
                if(mb.gquestion(d_pool, NULL, tr("Prompt"), tr("The run settings have been modified, do you want to save?")) == 0){
                    d_pool->expFile->save();
                }

                d_pool->run_method_edited = false;
                mb.disconnect(d_pool);
            }

            GOverView *ov = (GOverView *)(ui->tabWidgetMain->widget(sw_OverView));
            int ret = ov->setSelectFile(fn);
            qDebug() << "Run experiment name:" << fn << dat.size() << ret;
            if(ret != 0) break;
            ov->experimentState(true);
#ifdef DEVICE_TYPE_TL13
            if(QFileInfo(fn).path() ==".") {
                setFileInfomation(QFileInfo(fn).completeBaseName());
            }else{
                setFileInfomation(QFileInfo(fn).path() + QString("/") + QFileInfo(fn).completeBaseName());
            }
#else
            setFileInfomation(QFileInfo(fn).completeBaseName());
#endif
            int tab_index_ = sw_Utilities;
            GUtilities *us = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
            us->experimentState(true);            

            qDebug() << "Run step1: 更新按键状态";
            _PCR_RUN_CTRL_INFO pcrInfo;
            d_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)(d_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
            d_pool->mutex.unlock();

            qDebug() << "获得运行实验名时的状态：" << pcrInfo.State.DrvMotoBusy << pcrInfo.State.DrvDrawerPos << pcrInfo.State.DrvLidPos;

            updateStateInfo(5);

            bool reboot_and_motobusy_and_lidpos = this->property("reboot").toBool() && (pcrInfo.State.DrvMotoBusy || pcrInfo.State.DrvLidPos==0);

#if (defined DEVICE_TYPE_TL12)
            if(ui->buttonStart_Pause->isEnabled())
                ui->buttonStart_Pause->setEnabled(false);
#else
            ui->buttonStart_Pause->setIcon(pcrInfo.State.ExpState==1?QIcon(":/png/pause"):QIcon(":/png/run"));

            if(reboot_and_motobusy_and_lidpos){
                ui->buttonStart_Pause->setEnabled(false);
            }else{
                ui->buttonStart_Pause->setEnabled(!pcrInfo.State.DrvMotoBusy && pcrInfo.State.DrvDrawerPos && pcrInfo.State.ThFlag);
            }
#endif
#if (defined DEVICE_TYPE_TL22)
            ui->buttonEject->setEnabled(pcrInfo.State.ExpState == 2);

            if(reboot_and_motobusy_and_lidpos){
                ui->buttonStop->setEnabled(false);
                this->setProperty("WaitForLidClose", true);
            }else{
                ui->buttonStop->setEnabled(!pcrInfo.State.DrvMotoBusy && pcrInfo.State.DrvDrawerPos && pcrInfo.State.DrvLidPos);
            }
#elif (defined DEVICE_TYPE_TL12)
            if(reboot_and_motobusy_and_lidpos){
                ui->buttonStop->setEnabled(false);
                this->setProperty("WaitForLidClose", true);
            }else{
                ui->buttonStop->setEnabled(!pcrInfo.State.DrvMotoBusy && pcrInfo.State.DrvDrawerPos && pcrInfo.State.DrvLidPos);
            }
#else
            if(!ui->buttonStop->isEnabled())
                ui->buttonStop->setEnabled(true);
#endif

            if(ui->buttonExit->isEnabled())
                ui->buttonExit->setEnabled(false);

            qDebug() << "Run step2:" << ui->tabWidgetMain->currentIndex();

            //跳转到监控界面，开始监控实验，定位实验运行的位置
            if(ui->tabWidgetMain->currentIndex() != sw_RawData){
                ui->tabWidgetMain->setCurrentIndex(sw_RawData);
            }else{
                qDebug() << "Run step3:" << m_d->openFile << d_pool->expFile->fileName() << m_d->file_edited_time << QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss");
                if(m_d->openFile.isEmpty() || d_pool->expFile->fileName() != m_d->openFile || m_d->file_edited_time != QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss")){
                    bool not_del_dat_ = false;
                    if(d_pool->expFile->fileName() == m_d->openFile){
                        not_del_dat_ = pcrInfo.State.ExpState!=0;
                    }
                    //打开新文件
                    int ret = d_pool->expFile->open(d_pool->maxSpeed, not_del_dat_);
                    if(ret == 0){
                        m_d->openFile = d_pool->expFile->fileName();
                        m_d->file_edited_time = QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss");
                        emit updateExp();
                    }else{
                        m_d->openFile.clear();

                        //清空选择
                        GOverView *ov = (GOverView *)(ui->tabWidgetMain->widget(sw_OverView));
                        ov->clearSelection();

                        qDebug() << "Experiment file open error :" << ret;
                        My_MessageBox mb;                        
                        mb.gwarning(d_pool, NULL, tr("Warning"), tr("Experiment file open error : %1").arg(ret));
                        return;
                    }
                }
            }

            if(this->property("reboot").toBool()){
                //如果是断电重启时读取断电前的数据

                qDebug()<<"----------------- //如果是断电重启时读取断电前的数据-------------------------";
                GRawData *rd = (GRawData *)(ui->tabWidgetMain->widget(sw_RawData));
                rd->read_prev_data_of_shutdown();
            }

            //添加实验运行累计次数
            if(d_pool->selfCheckRptFile){
                d_pool->selfCheckRptFile->runCounter ++;
                d_pool->selfCheckRptFile->saveReport();

                us->updateSelfTest();
            }

            //发送状态变化信号
            emit experimentCtrl(1);

            this->setProperty("reboot", false);
            break;
        }

        case 15:
        {
            _PCR_INSTRUMENT_INFO instrumentInfo;
            instrumentInfo._memset();
            memcpy( (void*)(&instrumentInfo), (void*)(dat.data()), instrumentInfo._size() );

            d_pool->mutex.lock();
            qDebug()<<"----------PCR_type2_DeviceInfo-------deviceInfo.InstruSN-----"<<instrumentInfo.InstruSN<<GHelper::deviceSerial;
            GHelper::deviceName = QString::fromUtf8((char*)instrumentInfo.InstruName);
            GHelper::deviceSerial = QString::fromUtf8((char*)instrumentInfo.InstruSN);
            memcpy((void*)&d_pool->ArmVer, (const void*)&instrumentInfo.ArmVer.Value, sizeof(_DEVICE_VERSION));
            memcpy((void*)&d_pool->TherVer, (const void*)&instrumentInfo.TherVer.Value, sizeof(_DEVICE_VERSION));
            memcpy((void*)&d_pool->OptVer, (const void*)&instrumentInfo.OptVer.Value, sizeof(_DEVICE_VERSION));
            memcpy((void*)&d_pool->DrvVer, (const void*)&instrumentInfo.DrvVer.Value, sizeof(_DEVICE_VERSION));
            d_pool->mutex.unlock();

            int tab_index_ = sw_Utilities;
            GUtilities *uw = (GUtilities *)(ui->tabWidgetMain->widget(tab_index_));
            uw->updateDeviceInfo();

            QString msg;
            msg.append( QString("InstrySN[%1] \n InstruName[%2] \n ArmVer[%3] \n OptVer[%4] \n ThermalVer[%5] \n DrvVer[%6] \n")
                        .arg(QString( QByteArray( (char*)(instrumentInfo.InstruSN), sizeof(instrumentInfo.InstruSN) ) ))
                        .arg(QString( QByteArray( (char*)(instrumentInfo.InstruName), sizeof(instrumentInfo.InstruName) ) ))
                        .arg(QString::number(instrumentInfo.ArmVer.Value))
                        .arg(QString::number(instrumentInfo.TherVer.Value))
                        .arg(QString::number(instrumentInfo.OptVer.Value))
                        .arg(QString::number(instrumentInfo.DrvVer.Value))
                        );
            //
            qDebug() << "-------deviceInfo.InstruSN-----------------------"<<instrumentInfo.InstruSN;
            qDebug() <<  qPrintable(msg) ;

            //改为查询温控速率最大值
            this->setProperty("commandId", 1);

            break;
        }

        case 96:
        {
            quint32 maxSpeed[2] = {0};
            memcpy( (void*)maxSpeed, (void*)(dat.data()), 2*sizeof(quint32) );
            d_pool->maxSpeed[0] = (double)maxSpeed[0] / 100.0;
            d_pool->maxSpeed[1] = (double)maxSpeed[1] / 100.0;

            qDebug() << "Max speed:" << dat.size() << maxSpeed[0] << maxSpeed[1];

            //            for(int i=0; i<2; i++){
            //                if(d_pool->maxSpeed[i] < MIN_RAMP || d_pool->maxSpeed[i] > MAX_RAMP)
            //                    d_pool->maxSpeed[i] = MAX_RAMP;
            //            }
            //改为查询状态信息
            qDebug() <<" //改为查询状态信息  -----------------------";
            this->setProperty("commandId", 2);
            break;
        }
        case 14:
        {
            QByteArray expname(dat.size(),0);
            memcpy((void*)expname.data(), (const void*)(dat.data()), dat.size());
            QString fn = QString::fromUtf8(expname).trimmed();

            GHelper::deviceTypeName =fn ;
            this->setProperty("commandId", 3);
            break;
        }
        default:break;
        }
        break;
    }
    case 2:{
        qDebug()<<"---------------------2------------------------------------";
        break;
    }
    case 3:
    {
        //得到命令的响应,关闭计时
        if(m_d->timeoutId)
        {
            killTimer(m_d->timeoutId);
            m_d->timeoutId = 0;
            m_d->timeoutcnt = 0;
        }

        qDebug()<<"---------------------3------------------------------------" << type2;
        if(type2 == 1 || type2 == 2){
            // 只有在运行控制及样本仓控制时
            int len = sizeof(_PCR_RUN_CTRL_INFO);
            _PCR_RUN_CTRL_INFO pcrInfo;
            memset((void*)&pcrInfo, '\0', len);
            memcpy((void*)&pcrInfo, (const void*)(dat.data()), len);

            d_pool->mutex.lock();
            int sendCmdId = d_pool->commandType;
            if(sendCmdId != 0){
                d_pool->commandType = 0;
                d_pool->commandVule = sendCmdId>=4 ? -1 : (sendCmdId-1);
            }
            d_pool->mutex.unlock();

            qDebug() << "packet command response" << sendCmdId << d_pool->commandVule;

            //无效按键
            switch(sendCmdId)
            {
            case 2:
                if(ui->buttonStart_Pause->isEnabled())
                    ui->buttonStart_Pause->setEnabled(false);
                break;
            case 3:
                if(pcrInfo.State.ExpState==0)
                {
                    //                if(!ui->buttonStart_Pause->isEnabled())
                    //                    ui->buttonStart_Pause->setEnabled(true);
                }else{
                    if(ui->buttonStart_Pause->isEnabled())
                        ui->buttonStart_Pause->setEnabled(false);
                }
                break;
            case 1:
            {
                if(ui->buttonStop->isEnabled())
                    ui->buttonStop->setEnabled(false);
                break;
            }
#ifdef DEVICE_TYPE_TL22
            case 4:
            {
                if(ui->buttonEject->isEnabled())
                    ui->buttonEject->setEnabled(false);
                break;
            }
#endif
            default:break;
            }
        }
        break;
    }
    default:break;
    }
}

void GWidget::slot_tabBarClicked(int index){
    if(ui->tabWidgetMain->currentIndex() == 0 && index != 0 && (d_pool->commandVule<0 || d_pool->commandVule!=0) && (d_pool->expFile->fileName() != m_d->openFile || m_d->file_edited_time != QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss"))){
        d_pool->circular->showProcess(tr("Opening the experimental file"));

        QElapsedTimer time;
        time.start();
        while(time.elapsed() < 100){
            QCoreApplication::processEvents();
        }
    }
}

void GWidget::slot_currentChanged(int index)
{
    d_pool->screen_sound();

    //Tab改变时,判断当前文件打开,如果没有则打开
    //    if(index != 0){
    if(m_d->prevTabIndex != index){
        if(m_d->prevTabIndex == 1){
            if(d_pool->fileOpType==GOverView::FOT_Open && d_pool->run_method_edited){
                My_MessageBox mb;
                connect(&mb, SIGNAL(finished(int)), d_pool, SLOT(screen_sound()));
                d_pool->set_ui_busy_state(1);
                if(mb.gquestion(d_pool, NULL, tr("Prompt"), tr("The run settings have been modified, do you want to save?")) == 0){
                    m_d->file_edited_time = QFileInfo(d_pool->expFile->fileName()).lastModified().toString("yyyy_MM_dd-hh_mm_ss");
                    int ret = d_pool->expFile->save();
                    if(ret != 0){
                        qDebug() << "Experiment file save error :" << ret;
                    }
                }else{
                    d_pool->expFile->open(d_pool->maxSpeed);
                }
                d_pool->run_method_edited = false;
                mb.disconnect(d_pool);
                emit updateExp();
            }
        }

        qDebug() << Q_FUNC_INFO << d_pool->commandVule << m_d->openFile << d_pool->expFile->fileName() << m_d->file_edited_time << QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss") << ui->buttonStop->isEnabled();

        //按动停止键时不打开文件
        if((d_pool->commandVule<0 || d_pool->commandVule!=0) && d_pool->expFile->isValid() && (d_pool->expFile->fileName()!=m_d->openFile || m_d->file_edited_time!=QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss"))){
            //如果是运行中PC端更新样本设置，重新打开文件时不删除当前的荧光数据
            bool not_del_dat_ = false;
            if(d_pool->expFile->fileName() == m_d->openFile){
                _PCR_RUN_CTRL_INFO pcrInfo;
                d_pool->mutex.lock();
                memcpy((void*)&pcrInfo, (const void*)d_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
                d_pool->mutex.unlock();

                not_del_dat_ = pcrInfo.State.ExpState!=0;
            }
            qDebug() << Q_FUNC_INFO << "是否删除当前的荧光数据：" << !not_del_dat_;
            //打开新文件
            int ret = d_pool->expFile->open(d_pool->maxSpeed, not_del_dat_);

            //去掉打开文件的提示
            if(d_pool->circular->isShowing()){
                d_pool->circular->hideProcess();
                this->setProperty("message_is_showing", false);
            }

            if(ret == 0){
                m_d->openFile = d_pool->expFile->fileName();
                m_d->file_edited_time = QFileInfo(m_d->openFile).lastModified().toString("yyyy_MM_dd-hh_mm_ss");
                emit updateExp();
            }else{
                m_d->openFile.clear();
                //清空选择
                GOverView *ov = (GOverView *)(ui->tabWidgetMain->widget(sw_OverView));
                ov->clearSelection();

                qDebug() << "Experiment file open error :" << ret;
                My_MessageBox mb;               
                mb.gwarning(d_pool, NULL, tr("Warning"), tr("Experiment file open error : %1").arg(ret));
            }
        }
    }

    ui->tabWidgetMain->widget(index)->clearFocus();
    m_d->prevTabIndex = index;
    //    emit updateExp();

    qDebug() << Q_FUNC_INFO << "end";
}

