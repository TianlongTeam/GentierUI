/*!
* \file grawdata.h
* \brief ARM板软件中运行监控界面头文件
*
*实现了ARM板软件实验运行过程中实时监控的功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:37
*
*/

#ifndef GRAWDATA_H
#define GRAWDATA_H

#include <QWidget>
#include <qwt_plot_panner.h>
#include <qwt_plot_item.h>
#include <qwt_plot_curve.h>
#include "gexperimentfile.h"
typedef QMap<int, QwtPlotCurve* > CurveMap;
typedef QMap<int, QVector<double>* > VectorMap;

namespace Ui {
class GRawData;
}

class QToolButton;
class GDataPool;
class GRawData : public QWidget
{
    Q_OBJECT

public:
    enum CurveType{
        ct_Fluor,
        ct_Temperature
    };

    explicit GRawData(GDataPool *dataPool, QWidget *parent = 0);
    ~GRawData();

    void read_prev_data_of_shutdown();
public slots:
    void slot_channelModeChanged(const QByteArray &probes);
    void slot_channelItemChanged(int channel, bool enable, const QByteArray &probe);
    void slot_sampleChanged();

    void updateTempDisplay();
    void updateFluorDisplay();

    void monitorCtrl(int state);

    void slot_fluor_scan_info(const QByteArray &dat);
    void slot_get_hotmap_data(int fluor, int channel);
signals:
    void operatorLog(const QString &log);
    void sendHotmapData(QMap<int,double>);
protected:
    void changeEvent(QEvent *e);
    void timerEvent(QTimerEvent *e);
    bool eventFilter(QObject *obj, QEvent *ev);
private slots:
    void slot_fluorBtn_clicked();
    void slot_tempBtn_clicked();
    void slot_toolBox_currentChanged(int);

    void on_buttonPrev_clicked();
    void on_buttonNext_clicked();

private:
    void initVariables();
    void initUi();

    bool set_to_sampling();

    QString secToString(int sec);
    void changeFluorButtonStatus(int index);
    void changeTempButtonStatus(int index);
    void adjustPlotRange();
    void displayFluorOrHeatMap(int fluor);
    void displayTempCurve(int sec);    
    void displayStatus();
    void plotChanged(CurveType type = ct_Fluor);

    void init_temp_method();
    void updateCurrentWidget(int mapIndex);
private:
    Ui::GRawData *ui;

    class PrivateData;
    PrivateData *m_d;           ///<私有成员变量类指针
    GExpFileInfo   *m_expFileInfo;
    GDataPool   *m_pool;
    GExpRunInfo *m_runInfo;
    GExpRunMethod *m_runMethod;

    GWellFluorDataInfo_n *mn_dataAmpPtr;       ///< 扩增荧光数据指针
    GWellFluorDataInfo_n *mn_dataMeltPtr;      ///< 熔解荧光数据指针

    QMap< int,QPair<quint32,QByteArray> > m_fluorMaps;
};

double midPoint(double *inputdata,int size);
void shiftY(double *inputY,int size,double shiftval);
double min(double *inputY,int size);
void filtSingleCurve_sixChannel(double *inputY,int insize,double *outputY,int &outSize);
void filtAmpSingleCurve_sixChannel(double *inputY,int insize,double *outputY,int &outSize);



#endif // GRAWDATA_H
