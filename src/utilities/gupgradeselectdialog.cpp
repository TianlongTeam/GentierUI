#include "gupgradeselectdialog.h"
#include "ui_gupgradeselectdialog.h"

#include "gglobal.h"
#include "mymessagebox.h"
#include "gdatapool.h"
#include "pcr_info.h"
#include "JlCompress.h"
#include "quazip.h"
#include "quazipfile.h"
#include "gcircularwidget.h"

#include <QDir>
#include <QProcess>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>
#include <QSettings>
#include <QMouseEvent>
#include <QThread>

GUpgradeSelectDialog::GUpgradeSelectDialog(GDataPool *pool, const QString &devicePath, const QStringList &versions, QWidget *parent) : \
    QDialog(parent),
    ui(new Ui::GUpgradeSelectDialog),
    m_pool(pool),
    m_devicePath(devicePath),
    m_timerId(0),
    m_timeoutId(0),
    m_break(false),
    upgrade_fw_flag(0)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet("QDialog {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n"
                  "QToolButton {\nborder: 0px solid #2f74d0;\nbackground:transparent;\n}\n"
                  "QLabel{\nbackground: transparent;\n}\n"
                  "QPushButton {\nborder: 1px solid #2f74d0;\nfont: 11pt \"文泉驿等宽微米黑\";\nborder-radius: 2px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n"
                  "QPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n"
                  "QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}\n"
                  "QPushButton:disabled { \nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #F1F0EF);\n}");
    ui->buttonFWUpgrade->setEnabled(true);
    //显示当前版本号

    m_pool->fthe = true;
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    m_pool->fopt = true;
    m_pool->fdir = true;

    ui->labelOpticalCurentVersion->setText(versions.at(0));
    ui->labelTempOptiCurentVersion->setText(versions.at(1));
    ui->labelDrvboardCurentVersion->setText(versions.at(2));
    ui->labelUICurrentVersion->setText(versions.at(3));
    ui->labelCtlCurrentVersion->setText(versions.at(4));
    ui->labelSysCurrentVersion->setText(versions.at(5));
#elif defined (DEVICE_TYPE_TL23)
    m_pool->fopt = true;
    m_pool->fdir = false;

    ui->labelDrvboardTitle->setVisible(false);
    //    ui->labelDrvboardCurentVersion->clear();
    //    ui->labelDrvboardUpgradeVersion->clear();
    ui->labelDrvboardCurentVersion->setVisible(false);
    ui->labelDrvboardUpgradeVersion->setVisible(false);
    ui->lineDrvBoard->setVisible(false);

    ui->labelOpticalCurentVersion->setText(versions.at(0));
    ui->labelTempOptiCurentVersion->setText(versions.at(1));
    ui->labelUICurrentVersion->setText(versions.at(2));
    ui->labelCtlCurrentVersion->setText(versions.at(3));
    ui->labelSysCurrentVersion->setText(versions.at(4));
    ui->toolButton_dir->setVisible(false);
#elif defined (DEVICE_TYPE_TL13)
    m_pool->fopt = false;
    m_pool->fdir = false;

    ui->labelOpticalTitle->setVisible(false);
    ui->labelOpticalCurentVersion->setVisible(false);
    ui->labelOpticalUpgradeVersion->setVisible(false);
    ui->lineOptical->setVisible(false);
    ui->labelDrvboardTitle->setVisible(false);
    ui->labelDrvboardCurentVersion->setVisible(false);
    ui->labelDrvboardUpgradeVersion->setVisible(false);
    ui->lineDrvBoard->setVisible(false);
    ui->toolButton_dir->setVisible(false);
    ui->toolButton_opt->setVisible(false);

    ui->labelTempOptiCurentVersion->setText(versions.at(0));
    ui->labelUICurrentVersion->setText(versions.at(1));
    ui->labelCtlCurrentVersion->setText(versions.at(2));
    ui->labelSysCurrentVersion->setText(versions.at(3));

    ui->buttonFWUpgrade->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    ui->buttonSysUpgrade->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
#endif    

    m_pool->circular->showProcess(tr("Processing"));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    //初始化升级目录
    QProcess process;
    if(!QDir(UPGRADE_PATH).exists())
        QDir().mkdir(UPGRADE_PATH);

    if(QDir(FIRMWARE_PATH).exists()){
        process.start("rm -rf "+QLatin1String(FIRMWARE_PATH));
        process.waitForFinished();
    }
    process.start("mkdir "+QLatin1String(FIRMWARE_PATH));
    process.waitForFinished();

    if(QDir(SOFTWARE_PATH).exists()){
        process.start("rm -rf "+QLatin1String(SOFTWARE_PATH));
        process.waitForFinished();
    }
    process.start("mkdir "+QLatin1String(SOFTWARE_PATH));
    process.waitForFinished();

    if(QDir(SYSTEM_PATH).exists()){
        process.start("rm -rf "+QLatin1String(SYSTEM_PATH));
        process.waitForFinished();
    }
    process.start("mkdir "+QLatin1String(SYSTEM_PATH));
    process.waitForFinished();

    //查找固件升级文件,修改日期最晚的版本
    QString fn_mcu;
    bool notFirst = false;
    QDateTime dateTime;

#if defined (DEVICE_TYPE_TL22)
    QString getMcuFile = "*.mcu22";
#elif defined (DEVICE_TYPE_TL23)
    QString getMcuFile = "*.mcu23";
#elif defined (DEVICE_TYPE_TL13)
    QString getMcuFile = "*.mcu13";
#elif defined (DEVICE_TYPE_TL12)
    QString getMcuFile = "*.mcu12";
#endif

    foreach(const QFileInfo &fileInfo, QDir(m_devicePath).entryInfoList(QStringList() << getMcuFile, QDir::NoDotAndDotDot|QDir::Files)){
        bool flesh = false;
        if(notFirst){
            if(dateTime < fileInfo.lastModified()) flesh = true;
        }else{
            flesh = true;
            notFirst = true;
        }

        if(flesh){
            fn_mcu = fileInfo.filePath();
            dateTime = fileInfo.lastModified();
        }
    }

    for(int i=0; i<3; i++){
        m_fwVerCompare[i] = GreaterVer;
    }

    if(!fn_mcu.isEmpty()){
        qDebug() << "firmware upgrade file :" << fn_mcu;
        //拷贝固件升级文件
        if(!QFile::exists(QLatin1String(UPGRADE_PATH)+QFileInfo(fn_mcu).fileName())){
            process.start("cp -a "+fn_mcu+" "+UPGRADE_PATH);
            process.waitForFinished();
            process.start("sync");
            process.waitForFinished();
        }
        //解压
        JlCompress::extractDir(QLatin1String(UPGRADE_PATH)+QFileInfo(fn_mcu).fileName(), FIRMWARE_PATH);
        //读取版本号
        QSettings settings(QLatin1String(FIRMWARE_PATH)+"versions.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("VERSIONLIST");

        QString verTxt = settings.value("Optical").toString().trimmed();
        ui->labelOpticalUpgradeVersion->setText(verTxt);

        verTxt = settings.value("Temperature").toString().trimmed();

        ui->labelTempUpgradeVersion->setText(verTxt);
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        verTxt = settings.value("Drvboard").toString().trimmed();
        ui->labelDrvboardUpgradeVersion->setText(verTxt);
#endif
        settings.endGroup();

        //指定文件
        m_opticalFile.clear();
        QString fn_optical = QDir::toNativeSeparators(QLatin1String(FIRMWARE_PATH) + QLatin1String(OPTICAL_UPGRADE_FILE));
        if(m_pool->fopt && QFile::exists(fn_optical) && !ui->labelOpticalUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelOpticalCurentVersion->text().trimmed();
            QString str2 = ui->labelOpticalUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }

            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_fwVerCompare[0] = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_fwVerCompare[0] = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_fwVerCompare[0] = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_fwVerCompare[0] = EqualVer;
                        }
                    }
                }
            }

            m_opticalFile = fn_optical;
        }
        if(!m_opticalFile.isEmpty()) upgrade_fw_flag |= 1;

        m_drvboardFile.clear();
        QString fn_drv = QDir::toNativeSeparators(QLatin1String(FIRMWARE_PATH) + QLatin1String(DRVBOARD_UPGRADE_FILE));
        if(m_pool->fdir && QFile::exists(fn_drv) && !ui->labelDrvboardUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelDrvboardCurentVersion->text().trimmed();
            QString str2 = ui->labelDrvboardUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }

            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_fwVerCompare[2] = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_fwVerCompare[2] = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_fwVerCompare[2] = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_fwVerCompare[2] = EqualVer;
                        }
                    }
                }
            }

            m_drvboardFile = fn_drv;
        }
        if(!m_drvboardFile.isEmpty()) upgrade_fw_flag |= 2;

        m_temperatureFile.clear();
        QString fn_temp = QDir::toNativeSeparators(QLatin1String(FIRMWARE_PATH) + QLatin1String(TEMPERATURE_UPGRADE_FILE));
        if(m_pool->fthe && QFile::exists(fn_temp) && !ui->labelTempUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelTempOptiCurentVersion->text().trimmed();
            QString str2 = ui->labelTempUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }

            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_fwVerCompare[1] = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_fwVerCompare[1] = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_fwVerCompare[1] = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_fwVerCompare[1] = EqualVer;
                        }
                    }
                }
            }

            m_temperatureFile = fn_temp;
        }
        if(!m_temperatureFile.isEmpty()) upgrade_fw_flag |= 4;

        update_button_fw();
    }else
        ui->buttonFWUpgrade->setEnabled(false);


    //查找应用升级文件,修改日期最晚的版本
    QString fn_tl;
    notFirst = false;

#if defined (DEVICE_TYPE_TL22)
    QString getAppFile = "*.tl22";
#elif defined (DEVICE_TYPE_TL23)
    QString getAppFile = "*.tl23";
#elif defined (DEVICE_TYPE_TL13)
    QString getAppFile = "*.tl13";
#elif defined (DEVICE_TYPE_TL12)
    QString getAppFile = "*.tl12";
#endif

    foreach(const QFileInfo &fileInfo, QDir(m_devicePath).entryInfoList(QStringList() << getAppFile, QDir::NoDotAndDotDot|QDir::Files)){
        bool flesh = false;
        if(notFirst){
            if(dateTime < fileInfo.lastModified()) flesh = true;
        }else{
            flesh = true;
            notFirst = true;
        }

        if(flesh){
            fn_tl = fileInfo.filePath();
            dateTime = fileInfo.lastModified();
        }
    }

    for(int i=0; i<2; i++){
        m_sfVerCompare[i] = GreaterVer;
    }

    if(!fn_tl.isEmpty()){
        qDebug() << "software upgrade file :" << fn_tl;

        //拷贝应用升级文件
        if(!QFile::exists(QLatin1String(UPGRADE_PATH)+QFileInfo(fn_tl).fileName())){
            process.start("cp -a "+fn_tl+" "+UPGRADE_PATH);
            process.waitForFinished();
            process.start("sync");
            process.waitForFinished();
        }
        //解压
        JlCompress::extractDir(fn_tl, SOFTWARE_PATH);
        //读取版本号
        QSettings settings(QLatin1String(SOFTWARE_PATH)+"versions.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("VERSIONLIST");
        QString verTxt = settings.value("UI").toString().trimmed();
        ui->labelUIUpgradeVersion->setText(verTxt);
        verTxt = settings.value("Control").toString().trimmed();
        ui->labelCtlUpgradeVersion->setText(verTxt);
        settings.endGroup();

        //指定文件
        m_uiFile.clear();
        QString fn_ui = QDir::toNativeSeparators(QLatin1String(SOFTWARE_PATH) + UI_UPGRADE_PATH+ UI_UPGRADE_FILE);
        if(QFile::exists(fn_ui) && !ui->labelUIUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelUICurrentVersion->text().trimmed();
            QString str2 = ui->labelUIUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }
            //            qDebug() << "4444444444444" << list1 << list2;
            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_sfVerCompare[0] = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_sfVerCompare[0] = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_sfVerCompare[0] = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_sfVerCompare[0] = EqualVer;
                        }
                    }
                }
            }
            m_uiFile = fn_ui;
        }

        m_ctlFile.clear();
        QString fn_ctl = QDir::toNativeSeparators(QLatin1String(SOFTWARE_PATH) + CTL_UPGRADE_PATH + CTL_UPGRADE_FILE);
        if(QFile::exists(fn_ctl) && !ui->labelCtlUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelCtlCurrentVersion->text().trimmed();
            QString str2 = ui->labelCtlUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }
            //            qDebug() << "555555555555555" << list1 << list2;
            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_sfVerCompare[1] = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_sfVerCompare[1] = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_sfVerCompare[1] = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_sfVerCompare[1] = EqualVer;
                        }
                    }
                }
            }
            m_ctlFile = fn_ctl;
        }

        ui->buttonAppUpgrade->setEnabled(!m_uiFile.isEmpty() || !m_ctlFile.isEmpty());
    }

    //查找系统升级文件,修改日期最晚的版本
    QString fn_sys;
    notFirst = false;

    foreach(const QFileInfo &fileInfo, QDir(m_devicePath).entryInfoList(QStringList() << "*.boot", QDir::NoDotAndDotDot|QDir::Files)){
        bool flesh = false;
        if(notFirst){
            if(dateTime < fileInfo.lastModified()) flesh = true;
        }else{
            flesh = true;
            notFirst = true;
        }

        if(flesh){
            fn_sys = fileInfo.filePath();
            dateTime = fileInfo.lastModified();
        }
    }

    m_sysVerCompare = GreaterVer;

    if(!fn_sys.isEmpty()){
        qDebug() << "System upgrade file :" << fn_sys;

        //拷贝应用升级文件
        if(!QFile::exists(QLatin1String(UPGRADE_PATH)+QFileInfo(fn_sys).fileName())){
            process.start("cp -a "+fn_sys+" "+UPGRADE_PATH);
            process.waitForFinished();
            process.start("sync");
            process.waitForFinished();
        }
        //解压
        JlCompress::extractDir(fn_sys, SYSTEM_PATH);
        //读取版本号
        QSettings settings(QLatin1String(SYSTEM_PATH)+"versions.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("VERSIONLIST");
        QString verTxt = settings.value("Boot").toString().trimmed();
        ui->labelSysUpgradeVersion->setText(verTxt);
        settings.endGroup();

        //指定文件
        m_imgFile.clear();
        QString fn_img = QDir::toNativeSeparators(QLatin1String(SYSTEM_PATH) + SYS_UPGRADE_FILE);
        if(QFile::exists(fn_img) && !ui->labelSysUpgradeVersion->text().trimmed().isEmpty()){
            //版本号比较
            QRegExp rx("(\\d+)");
            QString str1 = ui->labelSysCurrentVersion->text().trimmed();
            QString str2 = ui->labelSysUpgradeVersion->text().trimmed();
            QStringList list1,list2;
            int pos = 0;
            while ((pos = rx.indexIn(str1, pos)) != -1) {
                list1 << rx.cap(1);
                pos += rx.matchedLength();
            }
            pos = 0;
            while ((pos = rx.indexIn(str2, pos)) != -1) {
                list2 << rx.cap(1);
                pos += rx.matchedLength();
            }

            if(list1.count()==3 && list2.count()==3){
                if(list2.at(0).toInt() < list1.at(0).toInt()){
                    m_sysVerCompare = LessVer;
                }else if(list2.at(0).toInt() == list1.at(0).toInt()){
                    if(list2.at(1).toInt() < list1.at(1).toInt()){
                        m_sysVerCompare = LessVer;
                    }else if(list2.at(1).toInt() == list1.at(1).toInt()){
                        if(list2.at(2).toInt() < list1.at(2).toInt()){
                            m_sysVerCompare = LessVer;
                        }else if(list2.at(2).toInt() == list1.at(2).toInt()){
                            m_sysVerCompare = EqualVer;
                        }
                    }
                }
            }
            m_imgFile = fn_img;
        }

        ui->buttonSysUpgrade->setEnabled(!m_imgFile.isEmpty());
    }

    if(m_pool->circular->isShowing()){
        m_pool->circular->hideProcess();
    }

    this->setProperty("upgrading", 0);
    this->setProperty("upgradeindex",0);
    this->setProperty("upgrade_over", false);

    m_timerId = startTimer(1000);
}

GUpgradeSelectDialog::~GUpgradeSelectDialog()
{
    delete ui;

    m_pool = NULL;
}

void GUpgradeSelectDialog::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    if(id == m_timerId){

        _PCR_RUN_CTRL_INFO pcrInfo;

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();
        qDebug()<<"----------------------------------pcrInfo.State.UpdateMcu---------------"<<pcrInfo.State.UpdateMcu<<this->property("upgrading").toInt()<<this->property("upgradeindex").toInt();
        switch(pcrInfo.State.UpdateMcu){
        case 0:{
            int stepId = this->property("upgrading").toInt();
            if(stepId == 0){
                if(!ui->buttonAppUpgrade->isEnabled())
                    ui->buttonAppUpgrade->setEnabled(!m_uiFile.isEmpty() || !m_ctlFile.isEmpty());

                if(!ui->buttonSysUpgrade->isEnabled())
                    ui->buttonSysUpgrade->setEnabled(!m_imgFile.isEmpty());

                if(!ui->buttonClose->isEnabled())
                    ui->buttonClose->setEnabled(true);
            }else if(stepId == 2){
                if(this->property("upgrade_over").toBool() && m_pool->circular->isShowing()){
                    m_pool->circular->hideProcess();
                }

                this->setProperty("upgrading", 0);
                this->setProperty("upgradeindex",0);
            }
            break;
        }
        case 1:
            qDebug() << "upgrading index :" << this->property("upgradeindex").toInt();
            this->setProperty("upgrading", 2);
            if(!m_pool->circular->isShowing()){
                m_pool->circular->showProcess(tr("Upgrading"));
                this->setProperty("upgrade_over", false);
            }
            break;
        case 2:{
            if(m_pool->circular->isShowing()){
                m_pool->circular->hideProcess();
            }
            this->setProperty("upgrading", -1);
            break;
        }
        default:break;
        }
    }else if(id == m_timeoutId){
        killTimer(m_timeoutId);
        m_timeoutId = 0;

        m_break = true;
    }

    QDialog::timerEvent(e);
}

void GUpgradeSelectDialog::paintEvent(QPaintEvent */*event*/)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(2, 2, this->width()-4, this->height()-4);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient gradient(0.3,-0.4,1.35,0.3,-0.4);
    QVector<QGradientStop> gradientStops;
    gradientStops.append(qMakePair(0,0x000fff));
    gradientStops.append(qMakePair(1,0xc9e5fe));
    gradient.setStops(gradientStops);
    painter.fillPath(path, QBrush(gradient));

    QColor color(47, 116, 208, 50);
    for(int i=0; i<1; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(2-i, 2-i, this->width()-(2-i)*2, this->height()-(2-i)*2);
        //        color.setAlpha(150 - qSqrt(i)*50);
        color.setAlpha(255);
        painter.setPen(color);
        painter.drawPath(path);
    }
}


void GUpgradeSelectDialog::on_buttonFWUpgrade_clicked()
{
    qDebug() << Q_FUNC_INFO;
    m_pool->screen_sound();
    quint8 flag = 0;
    //判断是否有相同版本或低版本的升级
    if(m_pool->fopt) flag |= m_fwVerCompare[0];     //光学
    if(m_pool->fthe) flag |= m_fwVerCompare[1];     //热学
    if(m_pool->fdir) flag |= m_fwVerCompare[2];     //驱动板

    if(flag>0){
        QString questionStr;
        switch(flag){
        case 1:
            questionStr = tr("Upgrade file with equivalent No. exists, do you want to continue?");
            break;
        case 2:
            questionStr = tr("Upgrade file with previous No. exists, do you want to continue?");
            break;
        case 3:
            questionStr = tr("Upgrade file with previous or equivalent No. exists, do you want to continue?");
            break;
        default:
            My_MessageBox mb;
            connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
            mb.gwarning(NULL, NULL, tr("Warning"), tr("Build No. comparison failed!"));
            mb.disconnect(m_pool);
            return;
        }

        My_MessageBox mb;
        connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
        int ret_ = mb.gquestion(NULL, NULL, tr("Inquiry"), questionStr, tr("Continue"), tr("Cancel"));
        mb.disconnect(m_pool);
        if(ret_ == 1) return;
    }

    ui->buttonFWUpgrade->setEnabled(false);
    ui->buttonAppUpgrade->setEnabled(false);
    ui->buttonSysUpgrade->setEnabled(false);
    ui->buttonClose->setEnabled(false);

    this->setProperty("upgrade_over", false);
    int upgrade_cycles_ = 0;
    int upgrade_rst_flag = 0;

#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    upgrade_drvboard();

    while(m_pool->fdir && this->property("upgradeindex").toInt()>0){
        if(m_break){
            upgrade_rst_flag |= 0x01;
            break;
        }

        QTime elapsed_ = QTime::currentTime().addSecs(-2);
        if(m_start_upgrade > elapsed_) continue;

        if(this->property("upgrading").toInt() < 0){
            if(m_pool->circular->isShowing()){
                m_pool->circular->hideProcess();
            }

            if(upgrade_cycles_<3){
                upgrade_cycles_++;
                QThread::msleep(500);

                qDebug() << "驱动板升级次数" << upgrade_cycles_;

                this->setFocus();
                upgrade_drvboard();
            }else{
                this->setFocus();
                My_MessageBox mb;
                connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
                mb.gwarning(NULL, NULL, tr("Warning"), tr("Driver board module upgrade failed,plase power off the instrument and power on again to complete the upgrade!"));
                mb.disconnect(m_pool);
                QProcess::execute("shutdown -h now");
            }
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
        this->setFocus();
    }
#endif

    upgrade_cycles_ = 0;
#ifndef DEVICE_TYPE_TL13
    upgrade_optical();

    while(m_pool->fopt && this->property("upgradeindex").toInt()>0){
        if(m_break){
            upgrade_rst_flag |= 0x02;
            break;
        }

        QTime elapsed_ = QTime::currentTime().addSecs(-2);
        if(m_start_upgrade > elapsed_) continue;

        if(this->property("upgrading").toInt() < 0){
            if(m_pool->circular->isShowing()){
                m_pool->circular->hideProcess();
            }

            if(upgrade_cycles_<3){
                upgrade_cycles_++;
                QThread::msleep(500);

                qDebug() << "光学升级次数" << upgrade_cycles_;

                this->setFocus();
                upgrade_optical();
            }else{
                this->setFocus();
                My_MessageBox mb;
                connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
                mb.gwarning(NULL, NULL, tr("Warning"), tr("Optical module upgrade failed,plase power off the instrument and power on again to complete the upgrade!"));
                mb.disconnect(m_pool);
                QProcess::execute("shutdown -h now");
            }
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
        this->setFocus();
    }
#endif

    upgrade_cycles_ = 0;
    upgrade_temp();

    while(m_pool->fthe && this->property("upgradeindex").toInt()>0){
        if(m_break){
            upgrade_rst_flag |= 0x04;
            break;
        }

        QTime elapsed_ = QTime::currentTime().addSecs(-2);
        if(m_start_upgrade > elapsed_) continue;

        if(this->property("upgrading").toInt() < 0){
            if(m_pool->circular->isShowing()){
                m_pool->circular->hideProcess();
            }

            if(upgrade_cycles_<3){
                upgrade_cycles_++;
                QThread::msleep(500);

                qDebug() << "热学升级次数" << upgrade_cycles_;

                this->setFocus();
                upgrade_temp();
            }else{
                this->setFocus();
                My_MessageBox mb;
                connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
                mb.gwarning(NULL, NULL, tr("Warning"), tr("Thermal module upgrade failed,plase power off the instrument and power on again to complete the upgrade!"));
                mb.disconnect(m_pool);
                QProcess::execute("shutdown -h now");
            }
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
        this->setFocus();
    }

    qDebug() << "完成固件升级，等待电机运动停止后重启";
    if(m_timerId){
        killTimer(m_timerId);
        m_timerId = 0;
    }
    if(m_timeoutId){
        killTimer(m_timeoutId);
        m_timeoutId = 0;
    }

    if(m_pool->circular->isShowing()){
        m_pool->circular->hideProcess();
    }

    _PCR_RUN_CTRL_INFO pcrInfo;
    memset((void*)&pcrInfo, 0, sizeof(_PCR_RUN_CTRL_INFO));

    do{
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();
    }while(pcrInfo.State.DrvMotoBusy);

    qDebug() << "电机停止，重启";
    this->setFocus();
    this->setProperty("upgrade_over", true);

    QString rst_msg_ = tr("Complete the upgrade, please shut down the system .");
    if(upgrade_rst_flag != 0){
        QStringList err_module_list_;

        if(upgrade_rst_flag & 0x01){
            err_module_list_ << tr("Driver module");
        }
        if(upgrade_rst_flag & 0x02){
            err_module_list_ << tr("Optical module");
        }
        if(upgrade_rst_flag & 0x04){
            err_module_list_ << tr("Thermal Cycle module");
        }
        rst_msg_ = err_module_list_.join(",") + tr(" upgrade failed!");
    }

    My_MessageBox mb;
    connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
    mb.ginformation(NULL, NULL, tr("Prompt"), rst_msg_, tr("Ok"));
    mb.disconnect(m_pool);

    QProcess::execute("shutdown -r now");
}

void GUpgradeSelectDialog::on_buttonAppUpgrade_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_pool->screen_sound();

    //判断是否有相同版本或低版本的升级
    quint8 flag = 0;
    qDebug()<<"----------判断是否有相同版本或低版本的升级--------------------"<<m_pool->fdir<<m_pool->fopt<<m_pool->fthe;
    for(int i=0; i<2; i++){
        flag |= m_sfVerCompare[i];
    }
    if(flag > 0){
        QString questionStr;
        switch(flag){
        case 1:
            questionStr = tr("There are upgrade files which equivalent to the current No., continue?");
            break;
        case 2:
            questionStr = tr("There are upgrade files which less than the current No., continue?");
            break;
        case 3:
            questionStr = tr("There are upgrade files which is equal to or less than the current No., continue?");
            break;
        default:
            My_MessageBox mb;
            connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
            mb.gwarning(NULL, NULL, tr("Warning"), tr("Build No. comparison failed!"));
            mb.disconnect(m_pool);
            return;
        }

        My_MessageBox mb;
        connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
        int ret_ = mb.gquestion(NULL, NULL, tr("Inquiry"), questionStr, tr("Continue"), tr("Cancel"));
        mb.disconnect(m_pool);
        if(ret_ == 1) return;
    }

    ui->buttonFWUpgrade->setEnabled(false);
    ui->buttonAppUpgrade->setEnabled(false);
    ui->buttonSysUpgrade->setEnabled(false);
    ui->buttonClose->setEnabled(false);

    if(m_timerId){
        killTimer(m_timerId);
        m_timerId = 0;
    }

    if(!m_pool->circular->isShowing()){
        m_pool->circular->showProcess(tr("Upgrading"));
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,50);
    }

    QProcess process;

    if(QFile::exists("~/upgrade/software/Configure.sh")){
        process.start("chmod +x ~/upgrade/software/Configure.sh");
        process.waitForFinished();

        process.start("sh ~/upgrade/software/Configure.sh");
        process.waitForFinished();

        process.start("rm ~/upgrade/software/Configure.sh");
        process.waitForFinished();
    }else{
        if(QFile::exists("~/upgrade/software/StartPcr")){
            process.start("rm ~/StartPcr");
            process.waitForFinished();
            process.start("chmod +x ~/upgrade/software/StartPcr");
            process.waitForFinished();
            process.start("mv ~/upgrade/software/StartPcr ~/");
            process.waitForFinished();
        }

        if(QFile::exists("~/upgrade/software/upgrade_ui.sh")){
            process.start("chmod +x ~/upgrade/software/upgrade_ui.sh");
            process.waitForFinished();
        }

        if(QFile::exists("~/upgrade/software/upgrade_ctl.sh")){
            process.start("chmod +x ~/upgrade/software/upgrade_ctl.sh");
            process.waitForFinished();
        }
    }

    //新建升级锁定文件
    if(!m_uiFile.isEmpty()){
        process.start(QString("touch ~/upgrade/software/upgrade_ui.lock"));
        process.waitForFinished();
    }
    if(!m_ctlFile.isEmpty()){
        process.start(QString("touch ~/upgrade/software/upgrade_ctl.lock"));
        process.waitForFinished();
    }

    if(m_pool->circular->isShowing()){
        m_pool->circular->hideProcess();
    }

    QProcess::execute("shutdown -r now");
}

void GUpgradeSelectDialog::on_buttonSysUpgrade_clicked()
{
    qDebug() << Q_FUNC_INFO;

    if(m_timerId){
        killTimer(m_timerId);
        m_timerId = 0;
    }

    if(!m_pool->circular->isShowing()){
        m_pool->circular->showProcess(tr("Upgrading"));
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,50);
    }

    QProcess process;

    //复制内核文件
    process.start("mv ~/upgrade/system/uImage /media/mmcblk0p1/");
    process.waitForFinished();
    process.start("sync");
    process.waitForFinished();

    //设置内核版本
    QFile file("/etc/sysversion");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream out(&file);
        out << ui->labelSysUpgradeVersion->text() << "\n";
    }

    m_pool->circular->hideProcess();

    QProcess::execute("shutdown -r now");
}

void GUpgradeSelectDialog::on_buttonClose_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_pool->screen_sound();
    ui->buttonFWUpgrade->setEnabled(true);
    accept();
}

void GUpgradeSelectDialog::on_toolButton_opt_clicked()
{
    if(m_pool->fopt){
        ui->toolButton_opt->setIcon(QIcon(":/png/la1"));
        m_pool->fopt = false;
    }else{
        ui->toolButton_opt->setIcon(QIcon(":/png/la2"));
        m_pool->fopt = true;
    }

    update_button_fw();
}

void GUpgradeSelectDialog::on_toolButton_dir_clicked()
{   
    if( m_pool->fdir){
        ui->toolButton_dir->setIcon(QIcon(":/png/la1"));
        m_pool->fdir = false;
    }else{
        ui->toolButton_dir->setIcon(QIcon(":/png/la2"));
        m_pool->fdir = true;
    }

    update_button_fw();
}

void GUpgradeSelectDialog::on_toolButton_the_clicked()
{
    if( m_pool->fthe){
        ui->toolButton_the->setIcon(QIcon(":/png/la1"));
        m_pool->fthe = false;
    }else{
        ui->toolButton_the->setIcon(QIcon(":/png/la2"));
        m_pool->fthe = true;
    }

    update_button_fw();
}

void GUpgradeSelectDialog::update_button_fw()
{
    quint8 flag_ = 0;

    m_pool->fopt ? (flag_ |= 1) : (flag_ &= ~1);
    m_pool->fdir ? (flag_ |= 2) : (flag_ &= ~2);
    m_pool->fthe ? (flag_ |= 4) : (flag_ &= ~4);

    ui->buttonFWUpgrade->setEnabled((flag_&upgrade_fw_flag)>0);
}

void GUpgradeSelectDialog::upgrade_drvboard()
{
    qDebug() << Q_FUNC_INFO;
    //等待电机运动结束
    _PCR_RUN_CTRL_INFO pcrInfo;
    memset((void*)&pcrInfo, 0, sizeof(_PCR_RUN_CTRL_INFO));

    do{
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();
    }while(pcrInfo.State.DrvMotoBusy);

    this->setProperty("upgrading", 1);
    this->setProperty("upgradeindex",0);

    if(m_pool->fdir && !m_drvboardFile.isEmpty() && QFile::exists(m_drvboardFile)){
        QByteArray dat;
        dat.resize(sizeof(quint16));
        quint16 addr = DRVBOARD_ADDR;
        memcpy((void*)dat.data(), (const void*)&addr, sizeof(quint16));
        dat.append(QString(DRVBOARD_UPGRADE_FILE).toUtf8());

        if(m_pool->isInitialzied()){
            this->setProperty("upgradeindex",3);
            m_pool->WriteData(3, 13, dat);
        }
    }

    m_start_upgrade = QTime::currentTime();

    if(m_timeoutId){
        killTimer(m_timeoutId);
        m_timeoutId = 0;
    }
    m_break = false;
    m_timeoutId = startTimer(15000);
}

void GUpgradeSelectDialog::upgrade_optical()
{
    qDebug() << Q_FUNC_INFO;
    //等待电机运动结束
    _PCR_RUN_CTRL_INFO pcrInfo;
    memset((void*)&pcrInfo, 0, sizeof(_PCR_RUN_CTRL_INFO));

    do{
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();
    }while(pcrInfo.State.DrvMotoBusy);

    this->setProperty("upgrading", 1);
    this->setProperty("upgradeindex",0);

    if(m_pool->fopt && !m_opticalFile.isEmpty() && QFile::exists(m_opticalFile)){
        QByteArray dat;
        dat.resize(sizeof(quint16));

        quint16 addr = OPTICAL_ADDR;
        memcpy((void*)dat.data(), (const void*)&addr, sizeof(quint16));
        dat.append(QString(OPTICAL_UPGRADE_FILE).toUtf8());

        if(m_pool->isInitialzied()){
            this->setProperty("upgradeindex",1);
            m_pool->WriteData(3, 13, dat);
        }
    }

    m_start_upgrade = QTime::currentTime();

    if(m_timeoutId){
        killTimer(m_timeoutId);
        m_timeoutId = 0;
    }
    m_break = false;
    m_timeoutId = startTimer(15000);
}

void GUpgradeSelectDialog::upgrade_temp()
{
    qDebug() << Q_FUNC_INFO;
    //等待电机运动结束
    _PCR_RUN_CTRL_INFO pcrInfo;
    memset((void*)&pcrInfo, 0, sizeof(_PCR_RUN_CTRL_INFO));

    do{
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();
    }while(pcrInfo.State.DrvMotoBusy);

    this->setProperty("upgrading", 1);
    this->setProperty("upgradeindex",0);

    if(m_pool->fthe && !m_temperatureFile.isEmpty() && QFile::exists(m_temperatureFile)){
        QByteArray dat;
        dat.resize(sizeof(quint16));

        quint16 addr = TEMPERATURE_ADDR;
        memcpy((void*)dat.data(), (const void*)&addr, sizeof(quint16));
        dat.append(QString(TEMPERATURE_UPGRADE_FILE).toUtf8());

        if(m_pool->isInitialzied()){
            this->setProperty("upgradeindex",2);
            m_pool->WriteData(3, 13, dat);
        }
    }

    m_start_upgrade = QTime::currentTime();

    if(m_timeoutId){
        killTimer(m_timeoutId);
        m_timeoutId = 0;
    }
    m_break = false;
    m_timeoutId = startTimer(15000);
}
