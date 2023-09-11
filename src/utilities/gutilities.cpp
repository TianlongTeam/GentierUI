/*!
* \file gutilities.cpp
* \brief ARM板软件中通用设置界面cpp文件
*
*实现了ARM板软件中仪器的各种配置功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:38
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gutilities.h"
#include "ui_gutilities.h"
#include "gglobal.h"
#include "gdatapool.h"

#include "ginputdialog.h"

#include "gdatainputdialog.h"
#include "gversiondetail.h"
#include "gcalendar.h"
#include "gupgradeselectdialog.h"
#include "gusbselectdialog.h"
#include "gtransportlockwizard.h"
#include "gcircularwidget.h"
#include "gexperimentfile.h"

#include "JlCompress.h"

#include <QStandardItemModel>
#include <QNetworkInterface>
#include <QFileDialog>
#include <QFileInfo>
#include <QScrollBar>
#include "mymessagebox.h"
#include <QDateTime>
#include <QInputDialog>
#include <QProcess>
#include <QHostInfo>
#include <QSettings>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QProgressBar>

#ifdef Q_OS_LINUX
#include <arpa/inet.h>
#endif

//-----------------------------------------------------------------------------
//define declare
//-----------------------------------------------------------------------------
#ifdef DEVICE_TYPE_TL22
#define SELFTEST_TABLEVIEW_WIDTH   764
#define SELFTEST_TABLEVIEW_HEIGHT  305
#else
#define SELFTEST_TABLEVIEW_WIDTH   699
#define SELFTEST_TABLEVIEW_HEIGHT  303
#endif

#define SELFTEST_ROW_HEIGHT        35
#define SELFTEST_COL_ITEM          100
#define SELFTEST_COL_RESULT        (SELFTEST_TABLEVIEW_WIDTH-SELFTEST_COL_ITEM)

//#define ALARM_TABLEVIEW_WIDTH   764
//#define ALARM_TABLEVIEW_HEIGHT  228
//#define ALARM_ROW_HEIGHT        38
#define ALARM_COL_TYPE          90
#define ALARM_COL_SOURCE        90
#define ALARM_COL_CODE          50
#define ALARM_COL_TIME          220

//#define ALARM_TABLEVIEW_WIDTH   764

#ifndef DEVICE_TYPE_TL22
#define ALARM_TABLEVIEW_HEIGHT  230
#define ALARM_ROW_HEIGHT   45
#else
#define ALARM_TABLEVIEW_HEIGHT  228
#define ALARM_ROW_HEIGHT      38
#endif
#define ALARM_COL_TYPE          90
#define ALARM_COL_SOURCE        90
#define ALARM_COL_CODE          50
#define ALARM_COL_TIME          220

#ifdef DEVICE_TYPE_TL22
#define LOG_TABLEVIEW_WIDTH   700
#define LOG_COL_TIME          220
#define LOG_COL_DESCRIBE      (LOG_TABLEVIEW_WIDTH-LOG_COL_TIME)
#else
#define LOG_TABLEVIEW_WIDTH  634
#define LOG_COL_TIME     220
#define LOG_COL_DESCRIBE    380
#endif

#define ROOT_PATH    "~/log"
#define LOG_PATH    "~/log/DebugLog"
#define CONTROL_LOG_PATH  "~/app/control/DebugLog"
#define UI_LOG_PATH  "~/app/ui/expdebuglog"

//#define LOG_COL_DESCRIBE      (LOG_TABLEVIEW_WIDTH-LOG_COL_TIME)
//-----------------------------------------------------------------------------
//private data class declare
//-----------------------------------------------------------------------------
/*!
* \class PrivateData
* \brief 类GUtilities内部的私有数据类
*
* 用于统一管理私有数据
*/
class GUtilities::PrivateData
{
public:
    PrivateData() :\
        calendar(NULL),
        testModel(NULL),
        warnModel(NULL),
        logModel(NULL),
        selectedIndex(0),
        timeId(0),
        networkId(0),
        hasNetworkConf(false),
        isConnected(false),
        isUpdate(false)
    {}

    ~PrivateData(){
        if(testModel) delete testModel;
        if(warnModel) delete warnModel;
        if(logModel) delete logModel;
    }

    GCalendar          *calendar;       ///< 日期设置控件
    QStandardItemModel *testModel;      ///< 自检信息
    QStandardItemModel *warnModel;      ///< 告警信息
    QStandardItemModel *logModel;       ///< 操作信息
    int     selectedIndex;              ///< 设置network时XXX.XXX.XXX.XXX的序号
    int     timeId;                     ///< 日期更新的时钟
    int     networkId;                  ///< 搜索Network的时钟
    bool    hasNetworkConf;             ///< 是否有网络配置文件
    bool    isConnected;                ///< 是否网络连接

    bool    isUpdate;                   ///< 仪器信息是否更新

    struct{
        //        QString deviceName;
        QString mac;
        QString ip;
        QString mask;
        QString gateway;
        QString dns;
    } localNetwork;

    QStringList languageList;
};

//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------
/*!
* \class GUtilities
* \brief ARM板通用设置界面类
*
* 仪器的各种设置功能
*/

/*!
* \brief 类GUtilities的构造函数
* \param parent = NULL
* \return 无
*/
GUtilities::GUtilities(GDataPool *pool, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GUtilities),
    m_d(new GUtilities::PrivateData),
    m_pool(pool)
{
    ui->setupUi(this);

    initVariables();
    initUi();
    this->clearFocus();
}

/*!
* \brief 类GUtilities的析构函数
* \param 无
* \return 无
*/
GUtilities::~GUtilities()
{
    if(m_d->timeId){
        killTimer(m_d->timeId);
        m_d->timeId = 0;
    }

    if(m_d->networkId){
        killTimer(m_d->networkId);
        m_d->networkId = 0;
    }

    delete ui;
    delete m_d;
    m_pool = NULL;

    qDebug() << "delete GUtilities widget";
}

/*!
* \brief 类GUtilities的公共函数，设置语言选项
* \param languages 软件中使用的语言列表
* \return 无
*/
void GUtilities::initLanguage(int lan)
{
    ui->radioButtonLangEn->blockSignals(true);
    ui->radioButtonLangEn->setChecked(lan==0);
    ui->radioButtonLangEn->blockSignals(false);

    ui->radioButtonLangZh->blockSignals(true);
    ui->radioButtonLangZh->setChecked(lan!=0);
    ui->radioButtonLangZh->blockSignals(false);
}

/*!
* \brief 类GUtilities的公共事件，更新自检报告的显示
* \param 无
* \return 无
*/
void GUtilities::updateSelfTest()
{
    //自检报告信息
    //    if(m_pool->selfCheckRptFile){
    //        ui->labelSelfTestResult->setText(m_pool->selfCheckRptFile->checkResult ? tr("successful") : tr("failing"));
    //        ui->labelNumOfRun->setText(QString::number(m_pool->selfCheckRptFile->runCounter));
    //        int hours = m_pool->selfCheckRptFile->runMinutes / 60;
    //        int minutes = m_pool->selfCheckRptFile->runMinutes % 60;
    //        if(hours == 0){
    //            ui->labelRunHour->setText(tr("%1min").arg(QString::number(minutes).leftJustified(2,' ',true)));
    //        }else{
    //            ui->labelRunHour->setText(tr("%1h %2min").arg(QString::number(hours)).arg(QString::number(minutes).leftJustified(2,' ',true)));
    //        }
    //    }else{
    //        ui->labelSelfTestResult->setText(tr("NULL"));
    //        ui->labelNumOfRun->setText(tr("NULL"));
    //        ui->labelRunHour->setText(tr("NULL"));
    //    }
    //    this->update();

    initSelfTestContent();
}

/*!
* \brief 类GUtilities的公共事件，更新日期时间的显示
* \param 无
* \return 无
*/
void GUtilities::updateCurrentDate()
{    
    //    ui->label_Version->setText(qApp->applicationVersion()+ "("+tr("%1.%2.%3").arg(m_pool->ArmVer.Bits.VerMajor).arg(m_pool->ArmVer.Bits.VerMinor).arg(QString::number(m_pool->ArmVer.Bits.VerRevision).rightJustified(3,'0',true))+")");
    ui->labelDate->setText(QDateTime::currentDateTime().toString(m_pool->dateFormat));
    ui->labelTime->setText(QDateTime::currentDateTime().toString(m_pool->is24HourFormat?"hh:mm":"hh:mm AP"));
}

/*!
* \brief 类GUtilities的公共事件，设置当前屏幕亮度
* \param val 亮度等级 [1~5]
* \return 无
*/
void GUtilities::setCurrentBrightness(int val)
{
    m_pool->brightness = val < 1 ? 1 : (val > 5 ? 5 : val);
    ui->lineEditBrightness->setText(QString::number(m_pool->brightness));
}

/*!
* \brief 类GUtilities的公共事件，用于启动或停止实验时使按键无效有效
* \param run 是否运行实验
* \return 无
*/
void GUtilities::experimentState(bool run)
{
    ui->buttonUpgrade->setEnabled(!run && (m_pool->usbMaps.count()>0));
    ui->buttonAlarmExportUSB->setEnabled(!run && (m_pool->usbMaps.count()>0));
    ui->buttonGetlog->setEnabled(!run && (m_pool->usbMaps.count()>0));
    ui->buttonDateTimeSet->setEnabled(!run);

    ui->buttonDeviceNameEdit->setEnabled(!run);
    ui->buttonNetworkEdit->setEnabled(!run);
#ifdef DEVICE_TYPE_TL22
    ui->buttonTransportLock->setEnabled(!run);
#endif
    ui->buttonEmptyStorage->setEnabled(!run);
}

/*!
* \brief 类GUtilities的公共事件，更新设备信息
* \param run 是否运行实验
* \return 无
*/
void GUtilities::updateDeviceInfo()
{
    ui->labelDeviceName->setText(GHelper::deviceName);
    ui->labelSerialNum->setText(GHelper::deviceSerial);
    m_d->isUpdate = false;
}

/*!
* \brief 公共槽函数，通用设置界面USB设备插拔时的操作
* \param on USB设备是否插入
* \param disk USB设备的盘符
* \return 无
*/
void GUtilities::slot_usbDeviceChanged()
{
    bool hasUsb = m_pool->usbMaps.count() > 0;
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState==0){
        ui->buttonUpgrade->setEnabled(hasUsb);
        ui->buttonAlarmExportUSB->setEnabled(hasUsb);
        ui->buttonGetlog->setEnabled(hasUsb);
    }
}
/*!
* \brief 类GUtilities的继承事件，用于动态切换语言
* \param 无
* \return 无
*/
void GUtilities::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);//在此处刷新语言的

        ui->labelSelfTestTitle->setText(tr("Self Inspection Result"));

        if(m_d->warnModel){
            QStringList headers;
            headers << tr("Type") << tr("Source") << tr("Code") << tr("Date/Time") << tr("Description");
            m_d->warnModel->setHorizontalHeaderLabels(headers);
        }
        ui->labelAlarmDetails->setText(tr("Alarm Details"));

        if(m_d->logModel){
            QStringList headers;
            headers << tr("Date/Time") << tr("Description");
            m_d->logModel->setHorizontalHeaderLabels(headers);
        }
        ui->labelOperatorTitle->setText(tr("Operation Log"));

        ui->labelMAC->setText(m_d->localNetwork.mac);
        ui->labelIP->setText(m_d->localNetwork.ip);
        ui->labelNetworkAddress->setText(m_d->localNetwork.ip);
        ui->labelMAC->setText(m_d->localNetwork.mac);
        ui->labelSubnetMask->setText(m_d->localNetwork.mask);
        ui->labelGateway->setText(m_d->localNetwork.gateway);

        ui->buttonSound->setText(m_pool->screenTouchSound ? tr("Close") : tr("Open"));

        initSelfTestContent();

        updateDeviceInfo();
        updateSelfTest();
        updateCurrentDate();

        slot_dateChanged();
        slot_timeChanged();
    }
    QWidget::changeEvent(e);
}

/*!
* \brief 类GUtilities的继承事件
* \param obj QObject对象指针
* \param e QEvent对象指针
* \return bool
*/
bool GUtilities::eventFilter(QObject *obj, QEvent *e)
{
    if(obj == ui->lineEditNetItem1){
        if(e->type() == QEvent::FocusIn){
            m_d->selectedIndex = 0;
        }
    }else if(obj == ui->lineEditNetItem2){
        if(e->type() == QEvent::FocusIn){
            m_d->selectedIndex = 1;
        }
    }else if(obj == ui->lineEditNetItem3){
        if(e->type() == QEvent::FocusIn){
            m_d->selectedIndex = 2;
        }
    }else if(obj == ui->lineEditNetItem4){
        if(e->type() == QEvent::FocusIn){
            m_d->selectedIndex = 3;
        }
    }else if(obj == ui->lineEditBrightness){
        if(e->type() == QEvent::MouseMove)
            return true;
    }else if(obj == ui->labelHour){
        if(e->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            QRegExp rx("(\\d+)");
            QString str = ui->labelHour->text();
            str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;
            GDataInputDialog dialog(tr("Hour"), str, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(0, 23);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 0){
                    val = 0;
                }else if(val > 23){
                    val = 23;
                }
                ui->labelHour->setText(QString::number(val));
            }
            return true;
        }
    }else if(obj == ui->labelMinute){
        if(e->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            QRegExp rx("(\\d+)");
            QString str = ui->labelMinute->text();
            str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;
            GDataInputDialog dialog(tr("Minute"), str, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(0, 59);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 0){
                    val = 0;
                }else if(val > 59){
                    val = 59;
                }
                ui->labelMinute->setText(QString::number(val));
            }
            return true;
        }
    }else if(obj == ui->labelSecond){
        if(e->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            QRegExp rx("(\\d+)");
            QString str = ui->labelSecond->text();
            str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;
            GDataInputDialog dialog(tr("Minute"), str, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(0, 59);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 0){
                    val = 0;
                }else if(val > 59){
                    val = 59;
                }
                ui->labelSecond->setText(QString::number(val));
            }
            return true;
        }
    }else if(obj == ui->comboBoxDateFormat->lineEdit()){
        if(e->type() == QEvent::MouseButtonPress){
            ui->comboBoxDateFormat->showPopup();
            return true;
        }
    }

    return QWidget::eventFilter(obj, e);
}

/*!
* \brief 类GUtilities的继承事件
* \param e QEvent对象指针
* \return 无
*/
void GUtilities::timerEvent(QTimerEvent *e)
{
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    int id = e->timerId();
    if(id == m_d->timeId){
        updateCurrentDate();
    }else if(id == m_d->networkId){
        bool isOk = scanNetWork();

        if(isOk != m_d->isConnected){
            m_d->isConnected = isOk;
            emit networkState(isOk);
        }
        if(!isOk){
            qDebug() << "restart networking";
            //            QProcess::execute("/etc/init.d/networking restart");

            killTimer(m_d->networkId);
            m_d->networkId = 0;

            QProcess process;
            process.start("/etc/init.d/networking restart");
            process.waitForFinished();

            m_d->networkId = startTimer(3030);
        }
    }

    ui->label_Instrutype->setText(GHelper::deviceTypeName);
    ui->labelSerialNum->setText(GHelper::deviceSerial);

    //    if(pcrInfo.State.ExpState==0)
    //        ui->buttonGetlog->setEnabled(m_pool->usbMaps.count());
    //    else  ui->buttonGetlog->setEnabled(false);

    QWidget::timerEvent(e);
}

/*!
* \brief 类GUtilities的私有槽函数，实现固件版本的升级
* \param 无
* \return 无
*/
void GUtilities::on_buttonUpgrade_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Firmware upgrade";
    emit editting2(true,true);   //无效启动按键
    setTabsEnabled(0, true);
    ui->buttonDeviceNameEdit->setEnabled(false);
    ui->buttonSelfTestDetail->setEnabled(false);
    ui->buttonAlarmDetail->setEnabled(false);
    ui->buttonOperatorDetail->setEnabled(false);
    ui->buttonBuild->setEnabled(false);
    ui->buttonUpgrade->setEnabled(false);

    m_pool->set_ui_busy_state(3);

    QString devStr;
    if(m_pool->usbMaps.count() < 1){
        //无U盘
    }else if(m_pool->usbMaps.count() == 1){
        devStr = m_pool->usbMaps.firstKey();
    }else{
        GUsbSelectDialog dialog(m_pool->usbMaps, this);
        dialog.show();
        int xx = (m_pool->desktopWidth - dialog.width())/2;
        int yy = (m_pool->desktopHeight - dialog.height())/2;
        dialog.move(xx,yy);

        if(dialog.exec() == QDialog::Accepted){
            devStr = dialog.currentDev();
        }
    }

    if(devStr.isEmpty()){
        emit editting2(false,true);
        return;
    }

    //读取当前版本号
    m_d->isUpdate = true;
    if(m_pool->isInitialzied())
        m_pool->WriteData(1, 15);

    QElapsedTimer time;
    time.start();
    while(time.elapsed() < 500){
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if(!m_d->isUpdate) break;
    }

    QStringList versionList;
    //光学版本
#ifndef DEVICE_TYPE_TL13
    versionList << tr("%1.%2.%3").arg(m_pool->OptVer.Bits.VerMajor).arg(m_pool->OptVer.Bits.VerMinor).arg(QString::number(m_pool->OptVer.Bits.VerRevision).rightJustified(3,'0',true));
#endif
    //热学版本
    versionList << tr("%1.%2.%3").arg(m_pool->TherVer.Bits.VerMajor).arg(m_pool->TherVer.Bits.VerMinor).arg(QString::number(m_pool->TherVer.Bits.VerRevision).rightJustified(3,'0',true));
    //驱动版本
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    versionList << tr("%1.%2.%3").arg(m_pool->DrvVer.Bits.VerMajor).arg(m_pool->DrvVer.Bits.VerMinor).arg(QString::number(m_pool->DrvVer.Bits.VerRevision).rightJustified(3,'0',true));
#endif
    //UI版本
    versionList << qApp->applicationVersion();
    //控制版本
    versionList << tr("%1.%2.%3").arg(m_pool->ArmVer.Bits.VerMajor).arg(m_pool->ArmVer.Bits.VerMinor).arg(QString::number(m_pool->ArmVer.Bits.VerRevision).rightJustified(3,'0',true));
    //系统版本
    versionList << m_pool->systemVersion;

    GUpgradeSelectDialog dialog(m_pool, devStr, versionList);

    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    int yy = (m_pool->desktopHeight - dialog.height())/2;
    dialog.move(xx,yy);

    dialog.exec();

    ui->buttonDeviceNameEdit->setEnabled(true);
    ui->buttonSelfTestDetail->setEnabled(true);
    ui->buttonAlarmDetail->setEnabled(true);
    ui->buttonOperatorDetail->setEnabled(true);
    ui->buttonBuild->setEnabled(true);
    ui->buttonUpgrade->setEnabled(m_pool->usbMaps.count() > 0);

    setTabsEnabled(0, false);
    emit editting2(false,true);
}

/*!
* \brief 类GUtilities的私有槽函数，实现自检信息的查看
* \param 无
* \return 无
*/
void GUtilities::on_buttonSelfTestDetail_clicked()
{
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键

    qDebug() << "Self Inspection Detial Information Show";
    ui->stackedWidgetDevice->setCurrentIndex(1);

    setTabsEnabled(0, true);

    emit operatorLog(tr("Show self inspection report"));
}

/*!
* \brief 类GUtilities的私有槽函数，退出自检信息查看界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonSelfTestBack_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Back Self Inspection Detial Information Show";
    ui->stackedWidgetDevice->setCurrentIndex(0);
    setTabsEnabled(0, false);

    this->clearFocus();

    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，实现告警信息的查看
* \param 无
* \return 无
*/
void GUtilities::on_buttonAlarmDetail_clicked()
{
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键

    qDebug() << "Alarm Detial Information Show";
    ui->stackedWidgetDevice->setCurrentIndex(2);

    setTabsEnabled(0, true);

    //重生刷新所有告警信息
    m_pool->warnFile->openFile();
    m_d->warnModel->removeRows(0,m_d->warnModel->rowCount());
    for(int i=0; i<m_pool->warnFile->infoList.count(); i++){
        //        m_d->warnModel->appendRow(new QStandardItem);
        //        QString txt = m_pool->warnFile->infoList.at(i).type != 0 ? tr("Error") : tr("Warning");
        //        m_d->warnModel->setData(m_d->warnModel->index(i,0), txt, Qt::DisplayRole);
        //        m_d->warnModel->setData(m_d->warnModel->index(i,1), m_pool->warnFile->infoList.at(i).src, Qt::DisplayRole);
        //        m_d->warnModel->setData(m_d->warnModel->index(i,2), m_pool->warnFile->infoList.at(i).code, Qt::DisplayRole);
        //        QDateTime dateTime = QDateTime::fromString(m_pool->warnFile->infoList.at(i).dateTime, "yyyy_MM_dd-hh_mm_ss");
        //        m_d->warnModel->setData(m_d->warnModel->index(i,3), dateTime.toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP")), Qt::DisplayRole);
        //        m_d->warnModel->setData(m_d->warnModel->index(i,4), m_pool->warnFile->infoList.at(i).describe, Qt::DisplayRole);

        m_d->warnModel->insertRow(0, new QStandardItem);
        QString txt = m_pool->warnFile->infoList.at(i).type != 0 ? tr("Error") : tr("Warning");
        m_d->warnModel->setData(m_d->warnModel->index(0,0), txt, Qt::DisplayRole);
        m_d->warnModel->setData(m_d->warnModel->index(0,1), m_pool->warnFile->infoList.at(i).src, Qt::DisplayRole);
        m_d->warnModel->setData(m_d->warnModel->index(0,2), m_pool->warnFile->infoList.at(i).code, Qt::DisplayRole);
        QDateTime dateTime = QDateTime::fromString(m_pool->warnFile->infoList.at(i).dateTime, "yyyy_MM_dd-hh_mm_ss");
        m_d->warnModel->setData(m_d->warnModel->index(0,3), dateTime.toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP")), Qt::DisplayRole);
        m_d->warnModel->setData(m_d->warnModel->index(0,4), m_pool->warnFile->infoList.at(i).describe, Qt::DisplayRole);

    }

    emit operatorLog(tr("Show alarm information"));
}

/*!
* \brief 类GUtilities的私有槽函数，实现导出告警信息到USB设备
* \param 无
* \return 无
*/
void GUtilities::on_buttonAlarmExportUSB_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Export Alarm Detial Information to USB Device";

    QString devStr;
    if(m_pool->usbMaps.count() < 1){
        //无U盘
    }else if(m_pool->usbMaps.count() == 1){
        devStr = m_pool->usbMaps.firstKey();
    }else{
        GUsbSelectDialog dialog(m_pool->usbMaps, this);
        //            QPoint pos = this->mapToGlobal(this->geometry().center());
        //            dialog.move(pos.x()-dialog.width()/2, pos.y()-dialog.height()/2);
        dialog.show();
        int xx = (m_pool->desktopWidth - dialog.width())/2;
        int yy = (m_pool->desktopHeight - dialog.height())/2;
        dialog.move(xx,yy);

        if(dialog.exec() == QDialog::Accepted){
            devStr = dialog.currentDev();
        }
    }

    if(devStr.isEmpty()){
        return;
    }
    qDebug() << "select Usb device:" << devStr;

    QFileInfo fileInfo(m_pool->warnFile->fileName);

    QString fn = QDir::toNativeSeparators(devStr + QDir::separator() + fileInfo.fileName());
    int count = 1;
    while(QFile::exists(fn)){
        fn = QDir::toNativeSeparators(devStr + QDir::separator() + fileInfo.completeBaseName() + tr("[%1].").arg(count) + fileInfo.suffix());
        count++;
    }
    QProcess process;
    process.start(tr("cp -a %1 %2").arg(m_pool->warnFile->fileName).arg(fn));
    bool isOk = process.waitForFinished();
    if(isOk){
        process.start("sync");
        isOk = process.waitForFinished();
    }
    QElapsedTimer time;
    time.start();
    while(time.elapsed() < 500){
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    //        bool isOk = QFile::copy(fileName, fn);

    if(isOk){
        emit operatorLog(tr("Export alarm file to USB device"));
    }else{
        emit operatorLog(tr("Export alarm file to USB device failed"));
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Alarm file export failed."));
        return;
    }

    My_MessageBox mb;
    mb.gwarning(m_pool, NULL, tr("Prompt"), tr("Complete"));
}

/*!
* \brief 类GUtilities的私有槽函数，退出告警信息查看界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonAlarmDetailBack_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Back Alarm Detial Information Show";
    ui->stackedWidgetDevice->setCurrentIndex(0);
    setTabsEnabled(0, false);

    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，进入操作信息查看界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonOperatorDetail_clicked()
{
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键

    qDebug() << "Operator Information Show";
    m_d->logModel->removeRows(0, m_d->logModel->rowCount());
    //    int row = 0;

    QFile file(m_pool->operatorLogFile);
    if(file.exists()){
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            QString line = in.readLine();
            while (!line.isNull()) {
                int pos = line.indexOf('>');
                if(pos > 0){
                    //                    m_d->logModel->appendRow(new QStandardItem);
                    //                    QString txt = line.left(pos).trimmed();
                    //                    QDateTime dateTime = QDateTime::fromString(txt, "yyyy_MM_dd-hh_mm_ss");
                    //                    m_d->logModel->setData(m_d->logModel->index(row, 0), dateTime.toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP")), Qt::DisplayRole);
                    //                    txt = line.right(line.length()-pos-1).trimmed();
                    //                    m_d->logModel->setData(m_d->logModel->index(row, 1), txt, Qt::DisplayRole);
                    //                    row++;

                    m_d->logModel->insertRow(0, new QStandardItem);
                    QString txt = line.left(pos).trimmed();
                    QDateTime dateTime = QDateTime::fromString(txt, "yyyy_MM_dd-hh_mm_ss");
                    m_d->logModel->setData(m_d->logModel->index(0, 0), dateTime.toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP")), Qt::DisplayRole);
                    txt = line.right(line.length()-pos-1).trimmed();
                    m_d->logModel->setData(m_d->logModel->index(0, 1), txt, Qt::DisplayRole);
                }

                line = in.readLine();
            }
        }
    }


    ui->stackedWidgetDevice->setCurrentIndex(3);

    setTabsEnabled(0, true);

    emit operatorLog(tr("Show operation information"));
}

/*!
* \brief 类GUtilities的私有槽函数，退出操作信息查看界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonOperatorLogBack_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Back Operator Information Show";
    m_d->logModel->removeRows(0, m_d->logModel->rowCount());

    ui->stackedWidgetDevice->setCurrentIndex(0);
    setTabsEnabled(0, false);

    emit editting2(false);
}

void GUtilities::on_buttonHourAdd_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelHour->text().toInt();
    current = (current < 23) ? current+1 : 0;
    ui->labelHour->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_buttonHourReduce_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelHour->text().toInt();
    current = (current > 0) ? current-1 : 23;
    ui->labelHour->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_buttonMinuteAdd_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelMinute->text().toInt();
    current = (current < 59) ? current+1 : 0;
    ui->labelMinute->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_buttonMinuteReduce_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelMinute->text().toInt();
    current = (current > 0) ? current-1 : 59;
    ui->labelMinute->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_buttonSecondAdd_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelSecond->text().toInt();
    current = (current < 59) ? current+1 : 0;
    ui->labelSecond->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_buttonSecondReduce_clicked()
{
    m_pool->screen_sound();

    int current = ui->labelSecond->text().toInt();
    current = (current > 0) ? current-1 : 59;
    ui->labelSecond->setText(QString::number(current));

    slot_timeChanged();
}

void GUtilities::on_comboBoxDateFormat_currentIndexChanged(int /*index*/)
{
    m_pool->screen_sound();
    ui->comboBoxDateFormat->clearFocus();

    QString format = ui->comboBoxDateFormat->currentText().trimmed();
    ui->groupBoxDate->setTitle(m_d->calendar->currentDate().toString(format));
}

void GUtilities::on_radioButton24_toggled(bool checked)
{
    if(!checked) return;

    m_pool->screen_sound();
    slot_timeChanged();
}

void GUtilities::on_radioButton12_toggled(bool checked)
{
    if(!checked) return;

    m_pool->screen_sound();
    slot_timeChanged();
}

/*!
* \brief 类GUtilities的私有槽函数，实现仪器时间的设置
* \param 无
* \return 无
*/
void GUtilities::on_buttonDateTimeSet_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Set Instrument DateTime";

    emit editting2(true);   //无效启动按键

    QDate date = QDate::currentDate();
    m_d->calendar->setCurrentDate(date);
    ui->groupBoxDate->setTitle(date.toString(ui->comboBoxDateFormat->currentText().trimmed()));

    QTime time = QTime::currentTime();
    ui->labelHour->setText(QString::number(time.hour()));
    ui->labelMinute->setText(QString::number(time.minute()));
    ui->labelSecond->setText(QString::number(time.second()));
    ui->groupBoxTime->setTitle(time.toString(ui->radioButton24->isChecked()?"hh:mm:ss":"hh:mm:ss AP"));

    //    ui->stackedWidgetDevice->setCurrentIndex(4);
    ui->stackedWidgetConfig->setCurrentIndex(3);
    //    setTabsEnabled(0, true);
    setTabsEnabled(1, true);

    emit operatorLog(tr("Set Date/Time"));
}

/*!
* \brief 类GUtilities的私有槽函数，实现仪器时间的设置
* \param 无
* \return 无
*/
void GUtilities::on_buttonDateTimeConfirm_clicked()
{
    m_pool->screen_sound();

    ui->stackedWidgetConfig->setCurrentIndex(0);
    setTabsEnabled(1, false);

    QProcess process;
    process.start(tr("date %1%2%3%4%5.%6")
                  .arg(QString::number(m_d->calendar->currentDate().month()).rightJustified(2,'0',true))
                  .arg(QString::number(m_d->calendar->currentDate().day()).rightJustified(2,'0',true))
                  .arg(QString::number(ui->labelHour->text().toInt()).rightJustified(2,'0',true))
                  .arg(QString::number(ui->labelMinute->text().toInt()).rightJustified(2,'0',true))
                  .arg(QString::number(m_d->calendar->currentDate().year()).rightJustified(4,'0',true))
                  .arg(QString::number(ui->labelSecond->text().toInt()).rightJustified(2,'0',true)));
    process.waitForFinished();
    process.start("hwclock -w");
    process.waitForFinished();
    process.start("/etc/init.d/save-rtc.sh");
    process.waitForFinished();
    m_pool->dateFormat = ui->comboBoxDateFormat->currentText().trimmed();
    m_pool->is24HourFormat = ui->radioButton24->isChecked();

    emit configChanged();

    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，退出时间设置界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonDateTimeBack_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Back DateTime Config Widget";

    ui->comboBoxDateFormat->blockSignals(true);
    ui->comboBoxDateFormat->setCurrentText(m_pool->dateFormat);
    ui->comboBoxDateFormat->blockSignals(false);
    QRadioButton *radioButton = m_pool->is24HourFormat ? ui->radioButton24 : ui->radioButton12;
    radioButton->blockSignals(true);
    radioButton->setChecked(true);
    radioButton->blockSignals(false);

    //    ui->stackedWidgetDevice->setCurrentIndex(0);
    //    setTabsEnabled(0, false);
    ui->stackedWidgetConfig->setCurrentIndex(0);
    setTabsEnabled(1, false);
    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，设置仪器名称
* \param 无
* \return 无
*/
void GUtilities::on_buttonDeviceNameEdit_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Set Instrument Name";

    emit editting2(true);   //无效启动按键

    GInputDialog dialog(QString(), QString(), ui->labelDeviceName->text(), GInputDialog::IT_Name, this);
    dialog.setMaxCharLength(20);
    //    if(ui->labelDeviceName->text().size() >20)
    //    {
    //        My_MessageBox mb;
    //       mb.gwarning(m_pool, NULL, tr("Warning"), tr("The number of bytes entered is full[0~%1]!").arg(20));
    //    }
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));

    dialog.setTitle(tr("Instrument Name:"));
    dialog.setMaxCharLength(20);
    //    QPoint pos = this->mapToGlobal(this->geometry().center());
    //    dialog.move(pos.x()-352, pos.y()-167);

    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    QPoint point = this->rect().topLeft();
    point.setY(this->rect().height() - dialog.height());
    dialog.move(xx,this->mapToGlobal(point).y());

    m_pool->set_ui_busy_state(1);
    int ret_ = dialog.exec();

    if(ret_ == QDialog::Accepted){
//        QFileInfo fileInfo(dialog.input());

        ui->labelDeviceName->setText(dialog.input());

        QByteArray ba = ui->labelDeviceName->text().toUtf8();
        ba.resize(32);

        if(m_pool->isInitialzied())
            m_pool->WriteData(2, 14, ba);
    }

    emit operatorLog(tr("Set instrument name"));

    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，设置网络
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkEdit_clicked()
{
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键
    qDebug() << "Set Network";
    ui->buttonNetworkEdit->setProperty("ItemIndex", 0);
    ui->stackedWidgetConfig->setCurrentIndex(1);

    setTabsEnabled(1, true);

    emit operatorLog(tr("Set Network Information"));
}

/*!
* \brief 类GUtilities的私有槽函数，设置网络IP
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkAddress_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Set Network IP Address";
    ui->buttonNetworkEdit->setProperty("ItemIndex", 1);
    ui->labelSetNetworkTitle->setText(tr("Network Address"));
    analysisNetwork(ui->labelNetworkAddress->text().trimmed());
    m_d->selectedIndex = 0;
    ui->lineEditNetItem1->selectAll();
    ui->lineEditNetItem1->setFocus();

    //判断确定键是否有效
    QLineEdit *edit[4];
    edit[0] = ui->lineEditNetItem1;
    edit[1] = ui->lineEditNetItem2;
    edit[2] = ui->lineEditNetItem3;
    edit[3] = ui->lineEditNetItem4;
    bool isOk = false;
    for(int i=0; i<4; i++){
        int val = edit[i]->text().toInt(&isOk);
        if(!isOk) break;
        if(val>255){
            isOk = false;
            break;
        }
    }
    ui->buttonNetworkSetOk->setEnabled(isOk);

    ui->stackedWidgetConfig->setCurrentIndex(2);
}

/*!
* \brief 类GUtilities的私有槽函数，设置网络子网掩码
* \param 无
* \return 无
*/
void GUtilities::on_buttonSubnetMask_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Set Network Subnet Mask";
    ui->buttonNetworkEdit->setProperty("ItemIndex", 2);
    ui->labelSetNetworkTitle->setText(tr("Subnet Mask"));
    analysisNetwork(ui->labelSubnetMask->text().trimmed());
    m_d->selectedIndex = 0;
    ui->lineEditNetItem1->selectAll();
    ui->lineEditNetItem1->setFocus();

    //判断确定键是否有效
    QLineEdit *edit[4];
    edit[0] = ui->lineEditNetItem1;
    edit[1] = ui->lineEditNetItem2;
    edit[2] = ui->lineEditNetItem3;
    edit[3] = ui->lineEditNetItem4;
    bool isOk = false;
    for(int i=0; i<4; i++){
        int val = edit[i]->text().toInt(&isOk);
        if(!isOk) break;
        if(val>255){
            isOk = false;
            break;
        }
    }
    ui->buttonNetworkSetOk->setEnabled(isOk);

    ui->stackedWidgetConfig->setCurrentIndex(2);
}

/*!
* \brief 类GUtilities的私有槽函数，设置网络网关
* \param 无
* \return 无
*/
void GUtilities::on_buttonGateway_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Set Network Gateway";
    ui->buttonNetworkEdit->setProperty("ItemIndex", 3);
    ui->labelSetNetworkTitle->setText(tr("Gateway"));
    analysisNetwork(ui->labelGateway->text().trimmed());
    m_d->selectedIndex = 0;
    ui->lineEditNetItem1->selectAll();
    ui->lineEditNetItem1->setFocus();

    //判断确定键是否有效
    QLineEdit *edit[4];
    edit[0] = ui->lineEditNetItem1;
    edit[1] = ui->lineEditNetItem2;
    edit[2] = ui->lineEditNetItem3;
    edit[3] = ui->lineEditNetItem4;
    bool isOk = false;
    for(int i=0; i<4; i++){
        int val = edit[i]->text().toInt(&isOk);
        if(!isOk) break;
        if(val>255){
            isOk = false;
            break;
        }
    }
    ui->buttonNetworkSetOk->setEnabled(isOk);

    ui->stackedWidgetConfig->setCurrentIndex(2);
}

/*!
* \brief 类GUtilities的私有槽函数，恢复网络缺省IP
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkDefault_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Set default Network";
#if defined(DEVICE_TYPE_TL22)
    m_d->localNetwork.ip = "192.168.22.10";
    m_d->localNetwork.mask = "255.255.255.0";
    m_d->localNetwork.gateway = "192.168.22.0";
#elif (DEVICE_TYPE_TL23)
    m_d->localNetwork.ip = "192.168.23.10";
    m_d->localNetwork.mask = "255.255.255.0";
    m_d->localNetwork.gateway = "192.168.23.0";
#elif(DEVICE_TYPE_TL13)
    m_d->localNetwork.ip = "192.168.13.10";
    m_d->localNetwork.mask = "255.255.255.0";
    m_d->localNetwork.gateway = "192.168.13.0";
#elif (DEVICE_TYPE_TL12)
    m_d->localNetwork.ip = "192.168.12.10";
    m_d->localNetwork.mask = "255.255.255.0";
    m_d->localNetwork.gateway = "192.168.12.0";
#endif
    ui->labelNetworkAddress->setText(m_d->localNetwork.ip);
    ui->labelSubnetMask->setText(m_d->localNetwork.mask);
    ui->labelGateway->setText(m_d->localNetwork.gateway);
    setNetworkConfig();
}

/*!
* \brief 类GUtilities的私有槽函数，返回配置界面
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkBack_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Set Network Ok";
    ui->labelIP->setText(ui->labelNetworkAddress->text().trimmed());
    ui->stackedWidgetConfig->setCurrentIndex(0);

    setTabsEnabled(1, false);

    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，网络设置确定
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkSetOk_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Network Set Ok";
    QString txt = tr("%1.%2.%3.%4")
            .arg(QString::number(ui->lineEditNetItem1->text().toInt()))
            .arg(QString::number(ui->lineEditNetItem2->text().toInt()))
            .arg(QString::number(ui->lineEditNetItem3->text().toInt()))
            .arg(QString::number(ui->lineEditNetItem4->text().toInt()));

    int itemIndex = ui->buttonNetworkEdit->property("ItemIndex").toInt();
    switch(itemIndex){
    case 1:
        m_d->localNetwork.ip = txt;
        ui->labelNetworkAddress->setText(txt);
        break;
    case 2:
        m_d->localNetwork.mask = txt;
        ui->labelSubnetMask->setText(txt);
        break;
    case 3:
        m_d->localNetwork.gateway = txt;
        ui->labelGateway->setText(txt);
        break;
    default:break;
    }

    ui->stackedWidgetConfig->setCurrentIndex(1);

    setNetworkConfig();
}

/*!
* \brief 类GUtilities的私有槽函数，取消网络设置
* \param 无
* \return 无
*/
void GUtilities::on_buttonNetworkSetCancel_clicked()
{
    m_pool->screen_sound();
    qDebug() << "Network Set Cancel";
    ui->stackedWidgetConfig->setCurrentIndex(1);
}

/*!
* \brief 类GUtilities的私有槽函数，减小液晶亮度
* \param 无
* \return 无
*/
void GUtilities::on_buttonBrightnessLeft_clicked()
{
    m_pool->screen_sound();
    int val = ui->lineEditBrightness->text().toInt() - 1;
    setCurrentBrightness(val);
    emit configChanged();

    if(m_pool->brightness == 1){
        val = 10;
    }else if(m_pool->brightness == 5){
        val = 100;
    }else{
        val = qRound(90.0/(5-1+1)) * m_pool->brightness + 10;
    }

    if(QFile::exists("/sys/class/backlight/pwm-backlight/brightness")){
        QFile file("/sys/class/backlight/pwm-backlight/brightness");
        if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
            file.write(QString::number(val).toUtf8());
            file.close();
        }
        //        QProcess::execute(tr("echo %1 >> /sys/class/backlight/pwm-backlight/brightness").arg(QString::number(val)));
    }

    emit operatorLog(tr("Reduce screen brightness"));
}

/*!
* \brief 类GUtilities的私有槽函数，加大液晶亮度
* \param 无
* \return 无
*/
void GUtilities::on_buttonBrightnessRight_clicked()
{
    m_pool->screen_sound();
    int val = ui->lineEditBrightness->text().toInt() + 1;
    setCurrentBrightness(val);
    emit configChanged();

    if(m_pool->brightness == 1){
        val = 10;
    }else if(m_pool->brightness == 5){
        val = 100;
    }else{
        val = qRound(90.0/(5-1+1)) * m_pool->brightness + 10;
    }

    if(QFile::exists("/sys/class/backlight/pwm-backlight/brightness")){
        QFile file("/sys/class/backlight/pwm-backlight/brightness");
        if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
            file.write(QString::number(val).toUtf8());
            file.close();
        }
        //        QProcess::execute(tr("echo %1 >> /sys/class/backlight/pwm-backlight/brightness").arg(QString::number(val)));
    }

    emit operatorLog(tr("Increase screen brightness"));
}

/*!
* \brief 类GUtilities的私有槽函数，屏幕按键音设置变化
* \param 无
* \return 无
*/
void GUtilities::on_buttonSound_clicked()
{

    m_pool->screen_sound();
    startTimer(5000);
    setScreenSound(!m_pool->screenTouchSound);
    emit configChanged();
}

/*!
* \brief 类GUtilities的私有槽函数，运输仪器锁定
* \param 无
* \return 无
*/
void GUtilities::on_buttonTransportLock_clicked()
{
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键
    emit transportLocking(true);

    GTransportLockWizard wizard(m_pool, this);
    if(wizard.exec() == QDialog::Accepted){
        qDebug() << "Lock Instrument for Transportion";
        emit operatorLog(tr("Lock Instrument for Transportion"));

        //关机
        //        exit(0);
        QProcess::execute("shutdown -h now");
    }else{
        emit transportLocking(false);
    }
    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，清空存储区
* \param 无
* \return 无
*/
void GUtilities::on_buttonEmptyStorage_clicked()
{
    m_pool->screen_sound();

    qDebug() << "Clear Storage Area";
    emit editting2(true);   //无效启动按键
    setTabsEnabled(2, true);

    My_MessageBox mb;
    if(mb.gquestion(m_pool, NULL, tr("Inquiry"), tr("All of the experiment file will be deleted, are you sure?")) == 1){
        setTabsEnabled(2, false);
        emit editting2(false);
        return;
    }

    ui->buttonTransportLock->setEnabled(false);

#ifdef Q_OS_LINUX
    QString sysDir = SYS_DIR_FILE;
#else
    QString sysDir = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_FILE);
#endif
    QFileInfoList fileInfoList = QDir(sysDir).entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs);
    QProgressBar progressBar(NULL);
    progressBar.setStyleSheet("QProgressBar{  color : solid #383e83;  border: 2px solid gray;  border-radius: 5px;  background: transparent;  padding: 0px;  text-align : center ;}  QProgressBar::chunk{  background: #c9e5fe;  }");
    progressBar.setRange(0, fileInfoList.count());
    progressBar.setValue(0);
    progressBar.show();
    int xx = (m_pool->desktopWidth - progressBar.width())/2;
    int yy = (m_pool->desktopHeight - progressBar.height())/2;
    progressBar.move(xx,yy);

    bool isFailed = false;
    int cnt = 0;
    foreach(const QFileInfo &fileInfo, fileInfoList){
        if(fileInfo.isDir()){
            QDir(fileInfo.filePath()).removeRecursively();
            emit clearCurrent();
        }else{
            bool isOk = QFile(fileInfo.filePath()).remove();
            if(isOk){
                if(fileInfo.completeBaseName() == QFileInfo(m_pool->expFile->fileName()).completeBaseName()){
                    //清除当前选择的实验
                    emit clearCurrent();
                }
            }else{
                isFailed = true;
            }
        }

        cnt++;
        progressBar.setValue(cnt);
    }

#ifdef DEVICE_TYPE_TL22
    ui->buttonTransportLock->setEnabled(true);
#endif

    progressBar.hide();

    QString promptStr;
    if(isFailed){
        promptStr = tr("Failed!");
        emit operatorLog(tr("Clear Storage Area failed"));
    }else{
        promptStr = tr("Complete");
        emit operatorLog(tr("Clear Storage Area"));
    }

    mb.gwarning(m_pool, NULL, tr("Prompt"), promptStr);

    setTabsEnabled(2, false);
    emit editting2(false);
}

/*!
* \brief 类GUtilities的私有槽函数，初始化私有变量
* \param 无
* \return 无
*/
void GUtilities::initVariables()
{
    ui->lineEditBrightness->installEventFilter(this);
    ui->lineEditNetItem1->installEventFilter(this);
    ui->lineEditNetItem2->installEventFilter(this);
    ui->lineEditNetItem3->installEventFilter(this);
    ui->lineEditNetItem4->installEventFilter(this);

    //初始化日期设置控制
    QHBoxLayout *hLayout = new QHBoxLayout;
    m_d->calendar = new GCalendar;
    hLayout->addWidget(m_d->calendar);
    ui->groupBoxDate->setLayout(hLayout);

    //初始化自检信息的模板
    m_d->testModel = new QStandardItemModel;
    initSelfTestContent();

    //初始化告警信息的模板
    m_d->warnModel = new QStandardItemModel;
    if(m_d->warnModel){
        QStringList headers;
        headers << tr("Type") << tr("Source") << tr("Code") << tr("Date/Time") << tr("Description");
        m_d->warnModel->setHorizontalHeaderLabels(headers);
    }

    //初始化操作信息的模板
    m_d->logModel = new QStandardItemModel;
    if(m_d->logModel){
        QStringList headers;
        headers << tr("Date/Time") << tr("Description");
        m_d->logModel->setHorizontalHeaderLabels(headers);
    }

    //network设置按键
    signalMapper.setMapping(ui->buttonNetData0, ui->buttonNetData0);
    signalMapper.setMapping(ui->buttonNetData1, ui->buttonNetData1);
    signalMapper.setMapping(ui->buttonNetData2, ui->buttonNetData2);
    signalMapper.setMapping(ui->buttonNetData3, ui->buttonNetData3);
    signalMapper.setMapping(ui->buttonNetData4, ui->buttonNetData4);
    signalMapper.setMapping(ui->buttonNetData5, ui->buttonNetData5);
    signalMapper.setMapping(ui->buttonNetData6, ui->buttonNetData6);
    signalMapper.setMapping(ui->buttonNetData7, ui->buttonNetData7);
    signalMapper.setMapping(ui->buttonNetData8, ui->buttonNetData8);
    signalMapper.setMapping(ui->buttonNetData9, ui->buttonNetData9);
    signalMapper.setMapping(ui->buttonNetDataDot, ui->buttonNetDataDot);
    signalMapper.setMapping(ui->buttonNetDataDel, ui->buttonNetDataDel);

    connect(ui->buttonNetData0, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData1, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData2, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData3, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData4, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData5, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData6, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData7, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData8, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetData9, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetDataDot, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->buttonNetDataDel, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    connect(&signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(slot_buttonClicked(QWidget*)));
}

/*!
* \brief 类GUtilities的私有槽函数，初始化UI
* \param 无
* \return 无
*/
void GUtilities::initUi()
{
    ui->buttonUpgrade->setEnabled(false);
    ui->buttonAlarmExportUSB->setEnabled(false);
    ui->buttonGetlog->setEnabled(false);
#ifdef DEVICE_TYPE_TL22
    ui->buttonTransportLock->setEnabled(true);
#else
//    ui->label_Version->setText(QStringLiteral("1.0"));
    ui->frameLock->setVisible(false);
#endif

    //控制版本
#ifdef DEVICE_TYPE_TL22
    ui->Utilities->setStyleSheet(QLatin1String("QTabBar::tab {height: 35px; width:263px;}\n"
                                               "QTabBar::tab:selected{border-image: url(:/png/BodyClicked);} \n"
                                               "QTabBar::tab:!enabled{border-image: url(:/png/BodyDisabled);} \n"
                                               "QTabBar::tab{border-image: url(:/png/Body);}\n"
                                               "\n"
                                               ""));
#else
    ui->Utilities->setStyleSheet(QLatin1String("QTabBar::tab {height: 35px; width:237px;}\n"
                                               "QTabBar::tab:selected{border-image: url(:/png/BodyClicked);} \n"
                                               "QTabBar::tab:!enabled{border-image: url(:/png/BodyDisabled);} \n"
                                               "QTabBar::tab{border-image: url(:/png/Body);}\n"
                                               "\n"
                                               ""));
#endif
    //    ui->spinBoxHour->installEventFilter(this);
    //    ui->spinBoxMinute->installEventFilter(this);
    //    ui->spinBoxSecond->installEventFilter(this);
    ui->comboBoxDateFormat->installEventFilter(this);

    qDebug() << "Touch screen sound :" << m_pool->screenTouchSound;
    ui->buttonSound->setText(m_pool->screenTouchSound ? tr("Close") : tr("Open"));
    //设置日期时间


    m_pool->dateFormat.isSharedWith(" 100");
    ui->comboBoxDateFormat->blockSignals(true);
    ui->comboBoxDateFormat->setCurrentText(m_pool->dateFormat);

    ui->comboBoxDateFormat->blockSignals(false);
    QRadioButton *radioButton = m_pool->is24HourFormat ? ui->radioButton24 : ui->radioButton12;
    radioButton->blockSignals(true);
    radioButton->setChecked(true);
    radioButton->blockSignals(false);

    setCurrentDate();
    connect(m_d->calendar, SIGNAL(dateChanged()), this, SLOT(slot_dateChanged()));
    setCurrentTime();

    //设置自检信息界面
    ui->tableViewSelfCheck->setModel(m_d->testModel);
    //    ui->tableViewSelfCheck->setColumnWidth(0, SELFTEST_COL_ITEM);
    //    ui->tableViewSelfCheck->setColumnWidth(1, SELFTEST_COL_RESULT);
    //    ui->tableViewSelfCheck->verticalHeader()->setDefaultSectionSize(SELFTEST_ROW_HEIGHT);
    ui->tableViewSelfCheck->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //设置告警信息界面
    ui->tableViewAlarm->setModel(m_d->warnModel);
    ui->tableViewAlarm->setColumnWidth(0, ALARM_COL_TYPE);
    ui->tableViewAlarm->setColumnWidth(1, ALARM_COL_SOURCE);
    ui->tableViewAlarm->setColumnWidth(2, ALARM_COL_CODE);
    ui->tableViewAlarm->setColumnWidth(3, ALARM_COL_TIME);
    ui->tableViewAlarm->verticalHeader()->setDefaultSectionSize(ALARM_ROW_HEIGHT);
    ui->tableViewAlarm->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewAlarm->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewAlarm->horizontalHeader()->setHighlightSections(false);

    //设置操作信息界面
    ui->tableViewLog->setModel(m_d->logModel);
    ui->tableViewLog->setColumnWidth(0, LOG_COL_TIME);
    ui->tableViewLog->setColumnWidth(1, LOG_COL_DESCRIBE);
    ui->tableViewLog->verticalHeader()->setDefaultSectionSize(ALARM_ROW_HEIGHT);
    ui->tableViewLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewLog->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewLog->horizontalHeader()->setHighlightSections(false);
    //屏幕按键音
    setScreenSound(m_pool->screenTouchSound);

    //network
    QNetworkInterface localInterface =QNetworkInterface::interfaceFromName("eth0");
    m_d->localNetwork.mac = localInterface.hardwareAddress();
    ui->labelMAC->setText(m_d->localNetwork.mac);
    m_d->hasNetworkConf = getNetworkConfig();
    m_d->isConnected = false;
    scanNetWork();
    slot_usbDeviceChanged();

    //其他设置
    ui->labelHour->installEventFilter(this);
    ui->labelMinute->installEventFilter(this);
    ui->labelSecond->installEventFilter(this);

    //其他连接
    connect(ui->Utilities, SIGNAL(currentChanged(int)), this, SLOT(slot_currentChanged(int)));

    // start timer 1s
    m_d->timeId = startTimer(1000);
    m_d->networkId = startTimer(3030);
}

/*!
* \brief 类GUtilities的私有函数，得到network的信息
* \param 无
* \return 无
*/
bool GUtilities::getNetworkConfig()
{
    ui->labelIP->clear();
    ui->labelNetworkAddress->clear();
    ui->labelSubnetMask->clear();
    ui->labelGateway->clear();

    if(!QFile::exists("/etc/eth0-setting") || !QFile::exists("/etc/init.d/ifconfig-eth0")) return false;

    QFile file("/etc/eth0-setting");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        qDebug() << "==" << line;
        if(line.contains("IP=")){
            m_d->localNetwork.ip = line.right(line.length()-line.indexOf("=")-1).trimmed();
            ui->labelIP->setText(m_d->localNetwork.ip);
            ui->labelNetworkAddress->setText(m_d->localNetwork.ip);
            //            qDebug() << "-----------------" << m_d->localNetwork.ip;
        }else if(line.contains("Mask=")){
            m_d->localNetwork.mask = line.right(line.length()-line.indexOf("=")-1).trimmed();
            ui->labelSubnetMask->setText(m_d->localNetwork.mask);
            //            qDebug() << "-----------------" << m_d->localNetwork.mask;
        }else if(line.contains("Gateway=")){
            m_d->localNetwork.gateway = line.right(line.length()-line.indexOf("=")-1).trimmed();
            ui->labelGateway->setText(m_d->localNetwork.gateway);
            //            qDebug() << "-----------------" << m_d->localNetwork.gateway;
        }
        line = in.readLine();
    }
    file.close();

    return true;
}

/*!
* \brief 类GUtilities的私有函数，设置network的信息
* \param 无
* \return 无
*/
void GUtilities::setNetworkConfig()
{
    QFile file("/etc/eth0-setting");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    in << tr("IP=%1").arg(m_d->localNetwork.ip).toUtf8() << "\n";
    in << tr("Mask=%1").arg(m_d->localNetwork.mask).toUtf8() << "\n";
    in << tr("Gateway=%1").arg(m_d->localNetwork.gateway).toUtf8() << "\n";
    in << tr("DNS=%1").arg(m_d->localNetwork.dns).toUtf8() << "\n";
    file.close();
    //更新系统IP
    if(m_d->networkId){
        killTimer(m_d->networkId);
        m_d->networkId = 0;
    }

    QProcess process;
    process.start("ifconfig eth0 down");
    process.waitForFinished();
    process.start(tr("ifconfig eth0 %1 netmask %2 up").arg(m_d->localNetwork.ip).arg(m_d->localNetwork.mask));
    process.waitForFinished();
    process.start(tr("route add default gw %1").arg(m_d->localNetwork.gateway));
    process.waitForFinished();
    process.start(tr("echo nameserver %1 > /etc/resolv.conf").arg(m_d->localNetwork.dns));
    process.waitForFinished();
    process.start("/etc/init.d/networking restart");
    process.waitForFinished();
    m_d->networkId = startTimer(3030);
}

/*!
* \brief 类GUtilities的私有函数，搜索network的信息
* \param 无
* \return 无
*/
bool GUtilities::scanNetWork()
{
    if(!m_d->hasNetworkConf) return false;

    bool isOk = false;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

    //获取所有网络接口的列表
    foreach(QNetworkInterface interface,list)
    {  //遍历每一个网络接口
        if(interface.flags() == 0x27){
            QList<QNetworkAddressEntry> entryList = interface.addressEntries();
            //获取IP地址条目列表，每个条目中包含一个IP地址，一个子网掩码和一个广播地址
            foreach(QNetworkAddressEntry entry,entryList)
            {//遍历每一个IP地址条目
                if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol){
                    //我们使用IPv4地址
                    isOk = true;

                    if(!m_d->isConnected){
                        if((entry.ip().toString() != m_d->localNetwork.ip) || (entry.netmask().toString() != m_d->localNetwork.mask))
                            getNetworkConfig();
                    }
                    break;
                }
            }
            break;
        }
    }

    return isOk;
}

/*!
* \brief 类GUtilities的私有函数，初始化自检模板的内容
* \param 无
* \return 无
*/
void GUtilities::initSelfTestContent()
{
    if(m_d->testModel){
        m_d->testModel->clear();
        QString content;
        m_d->testModel->setItem(0,0, new QStandardItem(tr("Self Inspection Result")));
        content = m_pool->selfCheckRptFile ? (m_pool->selfCheckRptFile->checkResult?tr("success"):tr("failed")) : "";
        m_d->testModel->setItem(0,1, new QStandardItem(content));

        m_d->testModel->setItem(1,0, new QStandardItem(tr("Date/Time")));
        QDateTime dateTime = QDateTime::fromString(m_pool->selfCheckRptFile->dateTime, "yyyy_MM_dd-hh_mm_ss");

        content = m_pool->selfCheckRptFile ? (dateTime.toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP"))) : "";
        m_d->testModel->setItem(1,1, new QStandardItem(content));

        //        m_d->testModel->setItem(2,0, new QStandardItem(tr("Run Counter")));
        //        content = m_pool->selfCheckRptFile ? QString::number(m_pool->selfCheckRptFile->runCounter) : "";
        //        m_d->testModel->setItem(2,1, new QStandardItem(content));

        //        m_d->testModel->setItem(3,0, new QStandardItem(tr("Run Time")));
        //        int hours = m_pool->selfCheckRptFile->runMinutes / 60;
        //        int minutes = m_pool->selfCheckRptFile->runMinutes % 60;
        //        if(hours == 0){
        //            content = m_pool->selfCheckRptFile ? tr("%1min").arg(QString::number(minutes).leftJustified(2,' ',true)) : "";
        //        }else{
        //            content = m_pool->selfCheckRptFile ? tr("%1h %2min").arg(QString::number(hours)).arg(QString::number(minutes).leftJustified(2,' ',true)) : "";
        //        }
        //        m_d->testModel->setItem(3,1, new QStandardItem(content));

        int pos = 2;
#ifndef DEVICE_TYPE_TL13
        m_d->testModel->setItem(pos,0, new QStandardItem(tr("Optical Module")));
        content = m_pool->selfCheckRptFile ? (m_pool->selfCheckRptFile->photometer?tr("ok"):tr("error")) : "";
        m_d->testModel->setItem(pos,1, new QStandardItem(content));
        pos++;
#endif
        m_d->testModel->setItem(pos,0, new QStandardItem(tr("Thermal Cycle Module")));
        content = m_pool->selfCheckRptFile ? (m_pool->selfCheckRptFile->thermodynamics?tr("ok"):tr("error")) : "";
        m_d->testModel->setItem(pos,1, new QStandardItem(content));
        pos++;
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        m_d->testModel->setItem(pos,0, new QStandardItem(tr("Driver Module")));
        content = m_pool->selfCheckRptFile ? (m_pool->selfCheckRptFile->coverDrive?tr("ok"):tr("error")) : "";
        m_d->testModel->setItem(pos,1, new QStandardItem(content));
        pos++;
#endif
    }
}

void GUtilities::setCurrentDate()
{
    QDate date = QDate::currentDate();
    m_d->calendar->setCurrentDate(date);
    ui->groupBoxDate->setTitle(date.toString(ui->comboBoxDateFormat->currentText().trimmed()));
}

void GUtilities::setCurrentTime()
{
    QTime currentTime = QTime::currentTime();
    ui->labelHour->setText(QString::number(currentTime.hour()));
    ui->labelMinute->setText(QString::number(currentTime.minute()));
    ui->labelSecond->setText(QString::number(currentTime.second()));
    ui->groupBoxTime->setTitle(currentTime.toString(ui->radioButton24->isChecked()?"hh:mm:ss":"hh:mm:ss AP"));
}

void GUtilities::analysisNetwork(const QString &text)
{
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    if( rx.exactMatch(text) ){
        QRegExp tt("(\\d{1,3}).(\\d{1,3}).(\\d{1,3}).(\\d{1,3})");
        tt.indexIn(text);
        ui->lineEditNetItem1->setText(tt.cap(1));
        ui->lineEditNetItem2->setText(tt.cap(2));
        ui->lineEditNetItem3->setText(tt.cap(3));
        ui->lineEditNetItem4->setText(tt.cap(4));

        ui->buttonNetworkSetOk->setEnabled(true);
    }else{
        ui->lineEditNetItem1->clear();
        ui->lineEditNetItem2->clear();
        ui->lineEditNetItem3->clear();
        ui->lineEditNetItem4->clear();

        ui->buttonNetworkSetOk->setEnabled(false);
    }
}

void GUtilities::setTabsEnabled(int index, bool enable)
{
    emit editting(enable);
    for(int i=0; i<ui->Utilities->count(); i++){
        if(i != index)
            ui->Utilities->setTabEnabled(i, !enable);
    }
}

void GUtilities::setScreenSound(bool open)
{
    m_pool->screenTouchSound = open;
    ui->buttonSound->setText(open ? tr("Close") : tr("Open"));

    emit operatorLog(open?tr("Open screen sound"):tr("Close screen sound"));
}

bool GUtilities::netIpIsValid(QString IP)
{
    QHostAddress addr(IP);
    return !((addr.toIPv4Address() == 0)||(addr.toIPv4Address() == 0xffffffff));
}

bool GUtilities::netMaskIsValid(QString mask)
{    
    bool isOk = netIpIsValid(mask);
#ifdef Q_OS_LINUX
    if(isOk){
        quint32 ii = ntohl(QHostAddress(mask).toIPv4Address());
        if((ii | (ii-1))==0xffffffff) isOk = true;
    }
#endif
    return isOk;
}

bool GUtilities::netMaskAndIpIsValid(QString IP, QString mask)
{
#ifdef Q_OS_LINUX
    bool isOk = netIpIsValid(IP);
    if(!isOk) return false;

    isOk = netMaskIsValid(mask);
    if(!isOk) return false;

    quint32 _ip = QHostAddress(IP).toIPv4Address();
    quint32 _mask = QHostAddress(mask).toIPv4Address();

    int a = _ip&0x000000ff;
    int b = ntohl(_mask);

    /*首先与默认子网掩码比较*/
    if(a>0&&a<127)
    {
        if(_mask<0x000000ff)
            return false;
        if(_mask>0x000000ff)
            b-=0xff000000;
    }
    if(a>=128&&a<=191)
    {
        if(_mask<0x0000ffff)
            return false;
        if(_mask>0x0000ffff)
            b-=0xffff0000;
    }
    if(a>=192&&a<=223)
    {
        if(_mask<0x00ffffff)
            return false;
        if(_mask>0x00ffffff)
            b-=0xffffff00;
    }

    /*每个子网段的第一个是网络地址,用来标志这个网络,最后一个是广播地址,用来代表这个网络上的所有主机.这两个IP地址被TCP/IP保留,不可分配给主机使用.*/
    int c = ~ntohl(_mask)&ntohl(_ip);
    if(c==0||c==(~ntohl(_mask)))
        return false;

    /*RFC 1009中规定划分子网时，子网号不能全为0或1，会导致IP地址的二义性*/
    if(b>0)
    {
        c = b&(ntohl(_ip));
        if(c==0||c==b)
            return false;
    }
#endif
    return true;
}

void GUtilities::on_radioButtonLangEn_toggled(bool checked)
{
    if(!checked) return;

    emit changeLanguage(LANG_ENGLISH);
    emit operatorLog(tr("Convert the current system language to English"));

    emit configChanged();
}

void GUtilities::on_radioButtonLangZh_toggled(bool checked)
{
    if(!checked) return;

    emit changeLanguage(LANG_CHINESE);
    emit operatorLog(tr("Convert the current system language to simplified Chinese"));

    emit configChanged();
}

void GUtilities::slot_buttonClicked(QWidget* widget)
{
    m_pool->screen_sound();
    QString txt = widget->property("buttonValue").toString();
    QLineEdit *edit[4];
    edit[0] = ui->lineEditNetItem1;
    edit[1] = ui->lineEditNetItem2;
    edit[2] = ui->lineEditNetItem3;
    edit[3] = ui->lineEditNetItem4;

    m_d->selectedIndex = (m_d->selectedIndex < 0) ? 0 : ((m_d->selectedIndex > 3) ? 3 : m_d->selectedIndex);

    if(txt == "-"){
        edit[m_d->selectedIndex]->clear();
        edit[m_d->selectedIndex]->setFocus();
    }else if(txt == "."){
        edit[m_d->selectedIndex]->deselect();
        (m_d->selectedIndex < 3) ? (m_d->selectedIndex++) : (m_d->selectedIndex = 0);
        edit[m_d->selectedIndex]->selectAll();
        edit[m_d->selectedIndex]->setFocus();
    }else{
        int deta = 0;
        int pos = -1;
        QString temp = edit[m_d->selectedIndex]->text();
        if(edit[m_d->selectedIndex]->hasSelectedText()){
            deta = temp.count() - edit[m_d->selectedIndex]->selectedText().count();
            pos = temp.indexOf(edit[m_d->selectedIndex]->selectedText());
            temp.remove(edit[m_d->selectedIndex]->selectedText());
        }

        if(temp.count() < 3){
            if(temp.isEmpty()){
                temp = txt;
            }else if(pos < 0){
                temp += txt;
            }else{
                if(deta == 0){
                    temp = txt;
                }else if(deta == 1){
                    if(pos==0){
                        temp = txt+temp;
                    }else if(pos==1){
                        temp += txt;
                    }
                }else if(deta == 2){
                    temp.insert(1,txt);
                }
            }
            edit[m_d->selectedIndex]->setText(temp);
        }else{
            edit[m_d->selectedIndex]->deselect();
            (m_d->selectedIndex < 3) ? (m_d->selectedIndex++) : (m_d->selectedIndex = 0);
            if(edit[m_d->selectedIndex]->text().isEmpty())
                edit[m_d->selectedIndex]->setText(txt);
            else
                edit[m_d->selectedIndex]->selectAll();
            edit[m_d->selectedIndex]->setFocus();
        }
    }

    //判断确定键是否有效
    bool isOk = false;
    for(int i=0; i<4; i++){
        int val = edit[i]->text().toInt(&isOk);
        if(!isOk) break;
        if(val>255){
            isOk = false;
            break;
        }
    }

    if(isOk){
        //        QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

        QString editTxt = edit[0]->text().trimmed();
        for(int i=1; i<4; i++){
            editTxt += "."+edit[i]->text().trimmed();
        }

        int itemIndx = ui->buttonNetworkEdit->property("ItemIndex").toInt();
        if(itemIndx == 2){
            isOk = netMaskIsValid(editTxt);
        }else{
            isOk = netIpIsValid(editTxt);
        }
        //        isOk = netMaskAndIpIsValid(editTxt, m_d->localNetwork.mask);
    }

    ui->buttonNetworkSetOk->setEnabled(isOk);
}

void GUtilities::slot_dateChanged()
{
    m_pool->screen_sound();
    ui->groupBoxDate->setTitle(m_d->calendar->currentDate().toString(ui->comboBoxDateFormat->currentText().trimmed()));
}

void GUtilities::slot_timeChanged()
{    
    if(ui->radioButton24->isChecked()){
        ui->groupBoxTime->setTitle(tr("%1:%2:%3")
                                   .arg(QString::number(ui->labelHour->text().toInt()).rightJustified(2,'0',true))
                                   .arg(QString::number(ui->labelMinute->text().toInt()).rightJustified(2,'0',true))
                                   .arg(QString::number(ui->labelSecond->text().toInt()).rightJustified(2,'0',true)));
    }else{
        int hour = ui->labelHour->text().toInt();
        QString timeStr = hour<12 ? "AM" : "PM";
        hour = hour<12 ? hour : (hour-12);
        ui->groupBoxTime->setTitle(tr("%1:%2:%3 %4")
                                   .arg(QString::number(hour).rightJustified(2,'0',true))
                                   .arg(QString::number(ui->labelMinute->text().toInt()).rightJustified(2,'0',true))
                                   .arg(QString::number(ui->labelSecond->text().toInt()).rightJustified(2,'0',true))
                                   .arg(timeStr));
    }
}

void GUtilities::slot_currentChanged(int index)
{
    m_pool->screen_sound();

    ui->Utilities->widget(index)->clearFocus();
}

void GUtilities::slot_fileOperator(bool operatoring)
{
    ui->buttonEmptyStorage->setEnabled(!operatoring);
}

void GUtilities::on_buttonGetlog_clicked()
{
    m_pool->screen_sound();

    ui->buttonTransportLock->setEnabled(false);
    ui->buttonEmptyStorage->setEnabled(false);
    ui->buttonGetlog->setEnabled(false);
    ui->buttonBuild->setEnabled(false);

    setTabsEnabled(2, true);
    emit editting2(true, true);

    //得到U盘路径
    QString devStr;
    if(m_pool->usbMaps.count() < 1){
        //无U盘
    }else if(m_pool->usbMaps.count() == 1){
        devStr = m_pool->usbMaps.firstKey();
    }

    bool isOk = false;

    if(QFile::exists(QLatin1String("~/log"))){
        m_pool->circular->showProcess(tr("Log data is being compressed"));

        JlCompress::compressDir("~/log.tar.gz", "~/log", true);

        m_pool->circular->showProcess(tr("Copying log data now"));

        QProcess process;
        process.start("mv ~/log.tar.gz "+devStr+"/");
        process.waitForFinished();
        process.start("sync");
        process.waitForFinished();

        QElapsedTimer time;
        time.start();
        while(time.elapsed() < 100){
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }

        m_pool->circular->hideProcess();
        isOk = true;
    }

    //如果拔掉了U盘,退出
    if(isOk && m_pool->usbMaps.count() > 0){
        emit operatorLog(tr("Files export succeed."));
    }else{
        emit operatorLog(tr("Files export interrupt."));
    }

    emit editting2(false, true);
    setTabsEnabled(2, false);
    ui->buttonTransportLock->setEnabled(true);
    ui->buttonEmptyStorage->setEnabled(true);
    ui->buttonGetlog->setEnabled(m_pool->usbMaps.count() > 0);
    ui->buttonBuild->setEnabled(true);
}

/*!
* \brief 类GUtilities的私有槽函数，显示版本的详细信息
* \param 无
* \return 无
*/
void GUtilities::on_buttonBuild_clicked()
{
    ui->buttonBuild->setEnabled(false);
    m_pool->screen_sound();

    emit editting2(true);   //无效启动按键

    m_d->isUpdate = true;
    if(m_pool->isInitialzied())
        m_pool->WriteData(1, 14);
    if(m_pool->isInitialzied())
        m_pool->WriteData(1, 15);

    QElapsedTimer time;
    time.start();
    while(time.elapsed() < 500){
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if(!m_d->isUpdate) break;
    }

    QStringList versionList;
    //系统版本
    versionList << m_pool->systemVersion;
    //UI版本
    //  versionList << qApp->applicationVersion();
    //控制版本
    versionList <<qApp->applicationVersion()+"("+ tr("%1.%2.%3").arg(m_pool->ArmVer.Bits.VerMajor).arg(m_pool->ArmVer.Bits.VerMinor).arg(QString::number(m_pool->ArmVer.Bits.VerRevision).rightJustified(3,'0',true))+")";
    //光学版本
#ifndef DEVICE_TYPE_TL13
    versionList << tr("%1.%2.%3").arg(m_pool->OptVer.Bits.VerMajor).arg(m_pool->OptVer.Bits.VerMinor).arg(QString::number(m_pool->OptVer.Bits.VerRevision).rightJustified(3,'0',true));
#endif
    //热学版本
    versionList << tr("%1.%2.%3").arg(m_pool->TherVer.Bits.VerMajor).arg(m_pool->TherVer.Bits.VerMinor).arg(QString::number(m_pool->TherVer.Bits.VerRevision).rightJustified(3,'0',true));
    //驱动版本
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    versionList << tr("%1.%2.%3").arg(m_pool->DrvVer.Bits.VerMajor).arg(m_pool->DrvVer.Bits.VerMinor).arg(QString::number(m_pool->DrvVer.Bits.VerRevision).rightJustified(3,'0',true));
#endif

    GVersionDetail dialog(versionList);
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    int yy = (m_pool->desktopHeight - dialog.height())/2;
    dialog.move(xx,yy);

    dialog.exec();

    ui->buttonBuild->setEnabled(true);
    this->clearFocus();

    emit editting2(false);
}
