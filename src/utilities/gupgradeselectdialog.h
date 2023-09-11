#ifndef GUPGRADESELECTDIALOG_H
#define GUPGRADESELECTDIALOG_H

#include <QDialog>
#include <QTime>

#define OPTICAL_UPGRADE_FILE        "Photology.hex"
#define TEMPERATURE_UPGRADE_FILE    "BuckCtrl.hex"
#define DRVBOARD_UPGRADE_FILE       "F28035.hex"

#define UI_UPGRADE_PATH             "ui/"
#define CTL_UPGRADE_PATH            "control/"
#define CTL_UPGRADE_FILE            "master_control"

#if defined (DEVICE_TYPE_TL22)
#define UI_UPGRADE_FILE             "ui_tl22"
#elif defined (DEVICE_TYPE_TL23)
#define UI_UPGRADE_FILE             "ui_tl23"
#elif defined (DEVICE_TYPE_TL13)
#define UI_UPGRADE_FILE             "ui_tl13"
#elif defined (DEVICE_TYPE_TL12)
#define UI_UPGRADE_FILE             "ui_tl12"
#endif

#define SYS_UPGRADE_FILE            "uImage"

#define UPGRADE_PATH                "~/upgrade/"
#define FIRMWARE_PATH               "~/upgrade/firmware/"
#define SOFTWARE_PATH               "~/upgrade/software/"
#define SYSTEM_PATH                 "~/upgrade/system/"
#define UPGRADE_LOCK                "upgrade.lock"

#define OPTICAL_ADDR        2
#define TEMPERATURE_ADDR    3
#define DRVBOARD_ADDR       4

namespace Ui {
class GUpgradeSelectDialog;
}

class GDataPool;
class GUpgradeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    enum VersionCompare{
        GreaterVer = 0,
        EqualVer = 1,
        LessVer = 2

    };

    explicit GUpgradeSelectDialog(GDataPool *pool, const QString &devicePath, const QStringList &versions, QWidget *parent = 0);
    ~GUpgradeSelectDialog();

protected:
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *event);

private slots:
    void on_buttonFWUpgrade_clicked();
    void on_buttonAppUpgrade_clicked();
    void on_buttonSysUpgrade_clicked();
    void on_buttonClose_clicked();

    void on_toolButton_opt_clicked();
    void on_toolButton_dir_clicked();
    void on_toolButton_the_clicked();    

private:
    void update_button_fw();

    void upgrade_drvboard();
    void upgrade_optical();
    void upgrade_temp();
private:
    Ui::GUpgradeSelectDialog *ui;
    GDataPool *m_pool;
    const QString &m_devicePath;
    int     m_timerId;
    int     m_timeoutId;
    bool    m_break;
    quint8  upgrade_fw_flag;             ///< 固件文件存在的标志

    QTime   m_start_upgrade;

    VersionCompare  m_fwVerCompare[3];     ///< 0:大于,1:等于,2:小于
    VersionCompare  m_sfVerCompare[2];     ///< 0:等于,1:小于,2:大于
    VersionCompare  m_sysVerCompare;    ///< 0:等于,1:小于,2:大于
    QString m_opticalFile;              ///< 光学固件文件名
    QString m_temperatureFile;          ///< 热学固件文件名
    QString m_drvboardFile;             ///< 驱动板固件文件名
    QString m_uiFile;                   ///< UI文件名
    QString m_ctlFile;                  ///< 主控文件名
    QString m_imgFile;                  ///< 系统内核文件名
};

#endif // GUPGRADESELECTDIALOG_H
