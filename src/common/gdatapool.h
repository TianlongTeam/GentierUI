#ifndef GDATAPOOL_H
#define GDATAPOOL_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QPair>
#include <QVector>
#include "pcr_info.h"
#include <QSerialPort>

class WidgetKeyboard;
class GExperimentFile;
class GSelfCheckRptFile;
class GWarningFile;
class GCircularWidget;
class BeepThread;
class GDataPool : public QObject
{
    Q_OBJECT
public:   
    explicit GDataPool(int width, int height);
    ~GDataPool();
    static QByteArray adminPassword;

    WidgetKeyboard      *keyboard;

    GExperimentFile     *expFile;
    GSelfCheckRptFile   *selfCheckRptFile;
    GWarningFile        *warnFile;
    BeepThread          *beepThread;

    GCircularWidget     *circular;

    int     current_lan;
    int     powerOnSecs;
    int     brightness;
    bool    screenTouchSound;
    bool    is24HourFormat;

    int     desktopWidth;
    int     desktopHeight;
    QWidget *desktop;

    bool    run_method_edited;

    int     commandType;
    int     commandVule;
    int     info_refresh_flag;
    bool    readingFluor;
    int     infinite_stage_no;
    int     infinite_step_no;

    bool    fopt;
    bool    fdir;
    bool    fthe;

    int     fileOpType;

    double  maxSpeed[2];

    QString dateFormat;

    QString systemVersion;

    _DEVICE_VERSION ArmVer;
    _DEVICE_VERSION OptVer;
    _DEVICE_VERSION TherVer;
    _DEVICE_VERSION DrvVer;

    QMap<QString, QString>  usbMaps;

    QByteArray  runCtrlInfo;

    QString     operatorLogFile;
    QString     filePath;

    QMutex  mutex;

    void set_ui_busy_state(quint16 flag);
public slots:
    void key_sound();
    void general_sound();
    void alarm_sound();
    void screen_sound();
    void auto_end_sound();
    void end_sound();

    bool isInitialzied();
    void WriteData(int type1, int type2);
    void WriteData(int type1, int type2, const QByteArray& data);
    void readData();
    void devicecheck();

signals:
    void usbChanged();
    void sig_dealReadData(quint8,quint16,QByteArray);

private slots:

private:
    QSerialPort m_serialPort;       ////<与下位机通讯
    QByteArray  m_readData;
};

#endif // GDATAPOOL_H
