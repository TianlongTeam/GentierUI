/*!
* \file gutilities.h
* \brief ARM板软件中通用设置界面头文件
*
*实现了ARM板软件中仪器的各种配置功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:38
*
*/

#ifndef GUTILITIES_H
#define GUTILITIES_H

#include <QWidget>
//#include "gtabwidget.h"
#include <QSignalMapper>
#include "gselfcheckrptfile.h"
#include "gwarningfile.h"

namespace Ui {
class GUtilities;
}

class GDataPool;
class GUtilities : public QWidget
{
    Q_OBJECT

public:
    explicit GUtilities(GDataPool *pool, QWidget *parent = 0);
    ~GUtilities();

    void initLanguage(int lan);

    void updateSelfTest();
    void updateCurrentDate();

    void setCurrentBrightness(int val);
    void experimentState(bool run);
    void updateDeviceInfo();
public slots:
    void slot_usbDeviceChanged();

signals:
    void editting(bool);
    void editting2(bool,bool all=false);
    void changeLanguage(int lan = 0);
    void operatorLog(const QString &log);
    void networkState(bool);
    void configChanged();    
    void transportLocking(bool);
    void clearCurrent();    ///< 清除当前选择的文件
protected:
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    void timerEvent(QTimerEvent *e);
private slots:
    //Device widget
    void on_buttonUpgrade_clicked();
    void on_buttonSelfTestDetail_clicked();
    void on_buttonSelfTestBack_clicked();
    void on_buttonAlarmDetail_clicked();
    void on_buttonAlarmExportUSB_clicked();
    void on_buttonAlarmDetailBack_clicked();
    void on_buttonDateTimeSet_clicked();
    void on_buttonDateTimeConfirm_clicked();
    void on_buttonDateTimeBack_clicked();

    void on_buttonOperatorDetail_clicked();
    void on_buttonOperatorLogBack_clicked();

    void on_buttonHourAdd_clicked();
    void on_buttonHourReduce_clicked();
    void on_buttonMinuteAdd_clicked();
    void on_buttonMinuteReduce_clicked();
    void on_buttonSecondAdd_clicked();
    void on_buttonSecondReduce_clicked();
    void on_comboBoxDateFormat_currentIndexChanged(int index);
    void on_radioButton24_toggled(bool checked);
    void on_radioButton12_toggled(bool checked);

    //Temperature widget
    void on_buttonDeviceNameEdit_clicked();
    void on_buttonNetworkEdit_clicked();
    void on_buttonNetworkAddress_clicked();
    void on_buttonSubnetMask_clicked();
    void on_buttonGateway_clicked();
    void on_buttonNetworkDefault_clicked();
    void on_buttonNetworkBack_clicked();
    void on_buttonNetworkSetOk_clicked();
    void on_buttonNetworkSetCancel_clicked();

    void on_buttonBrightnessLeft_clicked();
    void on_buttonBrightnessRight_clicked();

    void on_buttonSound_clicked();

    //Server widget   
    void on_buttonTransportLock_clicked();
    void on_buttonEmptyStorage_clicked();

    void on_radioButtonLangEn_toggled(bool checked);
    void on_radioButtonLangZh_toggled(bool checked);

    void slot_buttonClicked(QWidget*);        

    void slot_dateChanged();
    void slot_timeChanged();    

    void slot_currentChanged(int index);            

    void slot_fileOperator(bool operatoring);    
    void on_buttonGetlog_clicked();

    void on_buttonBuild_clicked();

private:
    void initVariables();
    void initUi();

    bool scanNetWork();

    void setNetworkConfig();
    bool getNetworkConfig();

    void initSelfTestContent();

    void setCurrentDate();
    void setCurrentTime();

    void analysisNetwork(const QString &text);
    void setTabsEnabled(int index, bool enable);

    void setScreenSound(bool open);

    //测试IP和Mask的合法性校验
    bool netIpIsValid(QString IP);
    bool netMaskIsValid(QString mask);
    bool netMaskAndIpIsValid(QString IP, QString mask);
private:
    Ui::GUtilities *ui;

    class PrivateData;
    PrivateData *m_d;           ///<私有成员变量类指针

    GDataPool *m_pool; ///< 私有自检报告文件指针
    QSignalMapper signalMapper;
};

#endif // GUTILITIES_H
