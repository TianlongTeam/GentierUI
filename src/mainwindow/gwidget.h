/*!
* \file gwidget.h
* \brief ARM板软件主界面头文件
*
*ARM板软件主界面程序框架的定义。
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-17 13:20
*
*/

#ifndef GWIDGET_H
#define GWIDGET_H

#include <QWidget>
#include <QTranslator>
#include <QDateTime>
#include <pcr_info.h>

#define SOCKET_INTERVAL             1000
#define MAX_OPERATOR_LOG_SHOW_NUM   6

#define KEY_ARM     0
#define KEY_OPTICAL 1
#define KEY_THERMAL 2
#define KEY_DRIVER  3

namespace Ui {
class GWidget;
}

class GDataPool;
class GLog;
class GWidget : public QWidget
{
    Q_OBJECT

public:
    enum SubWidget{
        sw_OverView,
        sw_RunEditor,
        sw_RawData,
        sw_Utilities,
        sw_Debugging
    };

    explicit GWidget(const QRect &desktopRect, QWidget *parent = 0);
    ~GWidget();

    int defaultLanguageId();

    void setOpenDateTime(){
        openDateTime = QDateTime::currentDateTime();
    }

public slots:
    void g_changeLanguage(int lan = 0);
    void g_networkState(bool connect);

    void setFileInfomation(const QString &info);
signals:
    void updateExp();
    void usbDevice(bool, const QString& disk = QString());
    void operatorLog(const QString &log);

    void experimentCtrl(int state);

    void fluor_scan_info(const QByteArray &dat);
protected:
    void changeEvent(QEvent *e);
    void timerEvent(QTimerEvent *e);

private slots:
    void on_buttonStart_Pause_clicked();
    void on_buttonStop_clicked();
#ifdef DEVICE_TYPE_TL22
    void on_buttonEject_clicked();
#endif

    void on_buttonAlarm_clicked();

    void slot_fileFocus(bool focus);
    void slot_editting(bool edit, bool all=false);
    void slot_fileChanged(const QString &fileName = QString(), int type = 0);
    void slot_nextWidget();
    void slot_runeditor_tabDisabled(bool enable);
    void slot_runMethod_edited();
    void slot_utilities_tabDisabled(bool enable);

    void slot_usbDeviceChanged();

    void slot_refreshScreen();
    void slot_operatorLog(const QString& log, int colorId = 0);

    void g_dealReadData(quint8 type1, quint16 type2, const QByteArray &dat);

    void slot_tabBarClicked(int index);
    void slot_currentChanged(int index);

    void saveConfInfo();
    void g_transportLocking(bool);    

    void g_sysShutdown();
    void g_sysReboot();
    void g_shutdown_halt();
    void g_shutdown_reboot();
    void g_shutdown_cancel();

    void g_fileCountCheck();

    void g_getMaxSpeedAgain();

private:
    void initVariable();
    void initUi();

    void updateStateInfo(int state);
    void alarm3Sound();
    void version();

    void buttonRefresh(const _PCR_RUN_CTRL_INFO &pcr_info, bool unuse_sync_flag = false);
    void sendControlCMD(int commandId, quint16 ctrl, const QByteArray &dat = QByteArray());

    void readConfInfo();
    void readErrorInfo();
    void clearTmpDirectory();
    bool removeDirWithContent(const QString &dirName, bool removeDir = false);

    void twinkleHotlidIcon();
    void shutdown_save_infos();
private:
    Ui::GWidget *ui;

    class PrivateData;
    PrivateData *m_d;           ///< 私有成员变量类指针

    GDataPool   *d_pool;        ///< 实验文件数据池

    QTranslator translatorQt, translator;     ///<语言翻译

    QDateTime   openDateTime;   ///< 软件打开的日期时间
};

#endif // GWIDGET_H
