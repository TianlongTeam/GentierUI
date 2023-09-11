/*!
* \file grawdata.cpp
* \brief ARM板软件中运行监控界面cpp文件
*
*实现了ARM板软件实验运行过程中实时监控的功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:37
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "grawdata.h"
#include "gruneditor.h"
#if defined(DEVICE_TYPE_TL22)
#include "ui_grawdata.h"
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#include "ui_grawdata.h"
#elif defined(DEVICE_TYPE_TL13)
#include "ui_grawdata-13.h"
#endif

#include "gglobal.h"

#include "timescaledraw.h"
#include "gcolordelegate.h"
#include "gdatapool.h"
#include "grunmethoditem.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_text_label.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <QThread>
#include <QScrollArea>
#include <QScrollBar>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QSettings>
#include <QElapsedTimer>
#include <QDir>

//用于判断是否nan， qIsFinite
#include <qnumeric.h>

//-----------------------------------------------------------------------------
//define declare
//-----------------------------------------------------------------------------
#define THIS_TAB_WIDTH   30

#if (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#define TABLEVIEW_WIDTH        450
#define TABLEVIEW_HEIGHT   263
#else
#define TABLEVIEW_WIDTH         528
#define TABLEVIEW_HEIGHT        288
#endif

//#if defined(DEVICE_TYPE_TL22)
////#define HEAT_MAP_ITEM_WIDTH     (TABLEVIEW_WIDTH / 12)
////#define HEAT_MAP_ITEM_HEIGHT    (TABLEVIEW_HEIGHT / 8)
//#elif defined(DEVICE_TYPE_TL23)
//#define HEAT_MAP_ITEM_WIDTH     75
//#define HEAT_MAP_ITEM_HEIGHT    33
//#endif

//pageTemp页选择按键的个数
#define SELECT_BUTTON_COUNT 2

#define SELECT_BTN_TEMP     0
#define SELECT_BTN_RAW      1

//#ifdef DEVICE_TYPE_TL23
//#define FLUOR_BTN_HEIGHT    52
//#else
#define FLUOR_BTN_HEIGHT    35
////#endif
#define FLUOR_BTN_CENTER_INDEX  4

#define AUTO_RECOVERY_TIME  10000

typedef QMap<int, QwtPlotCurve* > CurveMap;
typedef QMap<int, QVector<double>* > VectorMap;
//-----------------------------------------------------------------------------
//private data class declare
//-----------------------------------------------------------------------------
/*!
* \class PrivateData
* \brief 类GRawData内部的私有数据类
*
* 用于统一管理私有数据
*/
class GRawData::PrivateData
{
public:
    explicit PrivateData() :\
        fluorArea(NULL),
        fluorViewport(NULL),
        plot(NULL),
        grid(NULL),
        tempItem(NULL),
        tempCurve(NULL),
        hotCoverCurve(NULL),
        timeId(0),
        timeCount(0),
        currentCycle(0),
        last_amp_stage_no(-1),
        la4_stage_no(-1),
        isRuning(false),
        testType(0),
        curFluorIndex(0),
        curTempIndex(0),
        curShowIndex(0),
        colorDelegate(NULL),
        model(NULL)
    {
    }

    ~PrivateData(){
        if(tempItem){
            tempItem->detach();
            delete tempItem;
        }

        if(tempCurve){
            tempCurve->detach();
            delete tempCurve;
        }

        if(hotCoverCurve){
            hotCoverCurve->detach();
            delete hotCoverCurve;
        }

        tempX.clear();
        tempY.clear();
        hotcover.clear();

        foreach(CurveMap *curves, ampFluorCurves.values()){
            if(curves == NULL) continue;
            foreach(QwtPlotCurve *curve, curves->values()){
                if(curve == NULL) continue;
                curve->detach();
            }
            qDeleteAll(curves->values());
            curves->clear();
        }
        qDeleteAll(ampFluorCurves.values());
        ampFluorCurves.clear();

        foreach(CurveMap *curves, meltFluorCurves.values()){
            if(curves == NULL) continue;
            foreach(QwtPlotCurve *curve, curves->values()){
                if(curve == NULL) continue;
                curve->detach();
            }
            qDeleteAll(curves->values());
            curves->clear();
        }
        qDeleteAll(meltFluorCurves.values());
        meltFluorCurves.clear();

        if(grid) delete grid;
        if(plot) delete plot;
        if(model) delete model;
        if(colorDelegate) delete colorDelegate;


        fluorBtns.clear();
        ampFluorX.clear();

        foreach(VectorMap *fluorYs, ampFluorY.values()){
            if(fluorYs == NULL) continue;
            qDeleteAll(fluorYs->values());
            fluorYs->clear();
        }
        qDeleteAll(ampFluorY.values());
        ampFluorY.clear();

        meltFluorX.clear();
        foreach(VectorMap *fluorYs, meltFluorY.values()){
            if(fluorYs == NULL) continue;
            qDeleteAll(fluorYs->values());
            fluorYs->clear();
        }
        qDeleteAll(meltFluorY.values());
        meltFluorY.clear();
    }

    QScrollArea *fluorArea;             ///< 荧光素按键放置区域
    QWidget     *fluorViewport;         ///< 荧光素按键显示区域

    QwtPlot *plot;                      ///< 实时温度和荧光值绘图
    QwtPlotGrid *grid;                  ///< 网格
    GRunMethodItem *tempItem;           ///< stage和step框图
    QwtPlotCurve *tempCurve;            ///< 温度曲线
    QwtPlotCurve *hotCoverCurve;        ///< 温度曲线

    int     timeId;                     ///< 定时刷新信息定时器ID
    int     timeCount;                  ///< 定时器计数器
    int     currentCycle;               ///< 当前循环数
    QMap<int,int> sample_step_nos;      ///< 每次实验时显示曲线的step序号; <stageno, stepno>
    int     last_amp_stage_no;          ///< 最后一个采样的扩增stageno
    int     la4_stage_no;         ///< 最后一个熔解stageno
    bool    isRuning;                   ///< 表示正在监控
    int     testType;                   ///< 实验类型，熔解扩增/ 1,2,3
    int     curFluorIndex;              ///< 当前右侧栏pageFluor页按下的荧光素按键
    int     curTempIndex;               ///< 当前右侧栏pageTemp页按下的选择按键
    int     curShowIndex;               ///< 当前选择的显示类型:扩增(0)/熔解(1)/热图(2)

    GColorDelegate      *colorDelegate; ///< 显示热图的代理
    QStandardItemModel  * model;         ///< 显示热图的模板

    _PCR_RUN_CTRL_INFO preInfo;     ///< 前一秒运行信息
    int thkeep;     ///< 恒温标志

    QVector<double> tempX, tempY, hotcover;         ///< 温度曲线数据
    QVector<double> ampFluorX, meltFluorX;          ///< 荧光曲线数据
    QMap<int, CurveMap* >   ampFluorCurves, meltFluorCurves;    ///< 荧光值曲线 <荧光素编号, <孔序号, 曲线指针>>
    QMap<int, VectorMap* >  ampFluorY, meltFluorY;  ///< 荧光曲线数据 <荧光素编号, <孔序号, 曲线数据指针>>

    QList<QToolButton *> fluorBtns;         ///< 统一管理右侧pageFluor页荧光素按钮
    QList<QToolButton *> tempBtns;          ///< 统一管理右侧pageTemp页选择按钮
};
//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------
/*!
* \class GRawData
* \brief ARM板运行监控界面类
*
* 实验过程中实时监控的功能
*/

/*!
* \brief 类GRawData的构造函数
* \param parent = NULL
* \return 无
*/
GRawData::GRawData(GDataPool *dataPool, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GRawData),
    m_d(new GRawData::PrivateData),
    m_pool(dataPool)
{   
    ui->setupUi(this);
    m_runInfo = &dataPool->expFile->m_expRunInfo;
    m_runMethod = &dataPool->expFile->m_expRunMethod;
    mn_dataAmpPtr = &dataPool->expFile->mn_wellAmpData;
    mn_dataMeltPtr = &dataPool->expFile->mn_wellMeltData;
    initVariables();
    initUi();
}

/*!
* \brief 类GRawData的析构函数
* \param 无
* \return 无
*/
GRawData::~GRawData()
{    
    m_runInfo = NULL;
    m_runMethod = NULL;
    mn_dataAmpPtr= NULL;
    mn_dataMeltPtr = NULL;
    m_pool = NULL;

    delete m_d;
    delete ui;
    qDebug() << "delete GRawData widget";
}

/*!
* \brief 类GRawData的公共函数， 断电重启后实验在运行时,读取之前的数据
* \param 无
* \return 无
*/
void GRawData::read_prev_data_of_shutdown()
{    
#ifdef Q_OS_LINUX
    //初始化数据
    if(mn_dataAmpPtr){
        qDeleteAll(*mn_dataAmpPtr);
        mn_dataAmpPtr->clear();
    }
    if(mn_dataMeltPtr){
        qDeleteAll(*mn_dataMeltPtr);
        mn_dataMeltPtr->clear();
    }

    m_d->ampFluorX.clear();
    foreach(VectorMap *fluorYs, m_d->ampFluorY.values()){
        if(fluorYs == NULL) continue;
        qDeleteAll(fluorYs->values());
        fluorYs->clear();
    }

    m_d->meltFluorX.clear();
    foreach(VectorMap *fluorYs, m_d->meltFluorY.values()){
        if(fluorYs == NULL) continue;
        qDeleteAll(fluorYs->values());
        fluorYs->clear();
    }

    m_d->tempX.clear();
    m_d->tempY.clear();
    m_d->hotcover.clear();
    m_d->timeCount = 0;

#ifndef DEVICE_TYPE_TL13

    QString file_path_ = QDir::toNativeSeparators(SYS_DIR_FILE + QLatin1String(".run_fluor"));
    QStringList file_list_ = QDir(file_path_).entryList(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot, QDir::Time);

    //当前实验状态
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    qDebug() << Q_FUNC_INFO << file_path_ << file_list_ << pcrInfo.State.ExpState << pcrInfo.SizeInfo.No.Stage << pcrInfo.SizeInfo.No.Step << pcrInfo.SizeInfo.No.Cycle;

    //获得当前显示曲线的stageno和stepno
    int show_stage_no = -1;
    int show_step_no = -1;
    int cur_stage_no = pcrInfo.SizeInfo.No.Stage;
    while(cur_stage_no >= 0){
        if(m_d->sample_step_nos.contains(cur_stage_no)){
            show_stage_no = cur_stage_no;
            show_step_no = m_d->sample_step_nos.value(cur_stage_no);
            break;
        }
        cur_stage_no--;
    }

    //断电前是扩增时需要获得熔解数据，如果是熔解，则不需要熔解数据
    bool no_melt_dat = (show_stage_no==pcrInfo.SizeInfo.No.Stage && show_stage_no<m_runMethod->count() && m_runMethod->at(show_stage_no)->Property!=0);

    qDebug() << Q_FUNC_INFO << "state:" << pcrInfo.SizeInfo.No.Stage << no_melt_dat << show_stage_no << show_step_no;

    if(show_stage_no>=0 && show_step_no>=0){
        bool cur_is_melt = true;       //用来判断当前显示的是扩增还是熔解
        int another_stage_no = -1;      //另外一种stageno
        int another_step_no = -1;       //另外一种stepno
        //如果有数据需要显示, 倒序读取临时文件
        for(int i=0; i<file_list_.count(); i++){
            QString fn = QDir::toNativeSeparators(file_path_ + QDir::separator() + file_list_.at(i));

            QSettings fileSetting(fn, QSettings::IniFormat);
            fileSetting.setIniCodec("UTF-8");

            fileSetting.beginGroup("FluorData");
            int stage_no = fileSetting.value("StageNo").toInt();

            if((stage_no > show_stage_no)){
                //如果stageno大,查找下一个
                fileSetting.endGroup();
                continue;
            }else if(stage_no == show_stage_no){
                if(no_melt_dat){
                    cur_is_melt = true;
                    continue;
                }

                //如果stageno相等,判断stepno
                int step_no = fileSetting.value("StepNo").toInt();
                if(step_no != show_step_no){
                    fileSetting.endGroup();
                    continue;
                }

                QByteArray dat = GHelper::hexStrToByteArray(fileSetting.value("Value").toString());
                if(dat.size() != sizeof(_FLUOR_DATA)){
                    fileSetting.endGroup();
                    continue;
                }

                bool is_melt = fileSetting.value("IsMelt").toInt() != 0;
                cur_is_melt = is_melt;

                //填写数据
                GWellFluorDataInfo_n *info_ptr = is_melt ? mn_dataMeltPtr : mn_dataAmpPtr;
                _FLUOR_SCAN_INFO *scanInfo = new _FLUOR_SCAN_INFO;
                scanInfo->Type = is_melt;
                scanInfo->SaveFluorFileNo = file_list_.at(i).toInt();
                scanInfo->No = stage_no;
                memcpy((void*)(&scanInfo->FluorValue), (const void*)(dat.data()), sizeof(_FLUOR_DATA));
                info_ptr->push_front(scanInfo);
            }else{
                //如果stageno小,查找另外一种(扩增或熔解)的数据
                bool is_melt = fileSetting.value("IsMelt").toInt() != 0;
                if((!m_d->sample_step_nos.contains(stage_no)) || is_melt==cur_is_melt){
                    fileSetting.endGroup();
                    continue;
                }

                //先获得另一种的stageno
                if(another_stage_no < 0){
                    another_stage_no = stage_no;
                    another_step_no = m_d->sample_step_nos.value(stage_no);
                }else if(another_stage_no != stage_no){
                    fileSetting.endGroup();
                    break;
                }

                //如果stageno相等,判断stepno
                int step_no = fileSetting.value("StepNo").toInt();
                if(step_no != another_step_no){
                    fileSetting.endGroup();
                    continue;
                }

                QByteArray dat = GHelper::hexStrToByteArray(fileSetting.value("Value").toString());
                if(dat.size() != sizeof(_FLUOR_DATA)){
                    fileSetting.endGroup();
                    continue;
                }

                //填写数据
                GWellFluorDataInfo_n *info_ptr = is_melt ? mn_dataMeltPtr : mn_dataAmpPtr;
                _FLUOR_SCAN_INFO *scanInfo = new _FLUOR_SCAN_INFO;
                scanInfo->Type = is_melt;
                scanInfo->SaveFluorFileNo = file_list_.at(i).toInt();
                scanInfo->No = stage_no;
                memcpy((void*)(&scanInfo->FluorValue), (const void*)(dat.data()), sizeof(_FLUOR_DATA));
                info_ptr->push_front(scanInfo);
            }
        }
    }

    for(int c=0; c<mn_dataAmpPtr->count(); c++){
        m_d->ampFluorX.append(c+1);
    }
    for(int c=0; c<mn_dataMeltPtr->count(); c++){
        m_d->meltFluorX.append(c+1);
    }

    int well_max_ = GHelper::row_count * GHelper::column_count;
    qDebug() << Q_FUNC_INFO << "size:" << mn_dataAmpPtr->count() << mn_dataMeltPtr->count();

    this->setProperty("GetPrevData", true);
#endif
#endif
}

/*!
* \brief 类GRawData的公共槽函数， 实现右侧工具栏通道按键标题及使能的变化
* \param mode 运行设置中的通道选择模式
* \param flag 通道是否有效的标志，按位排列
* \param probes 该通道模式下所有的探针类型
* \return 无
*/
void GRawData::slot_channelModeChanged(const QByteArray &probes)
{ 
    bool isSample = set_to_sampling();
    int cntBase = 0;
    for(int ch=0; ch<GHelper::channel_count; ch++){
        int base = GHelper::indexOfFluorGroup(ch+1);
        for(int i=0; i<GHelper::countOfFluorGroup(ch+1); i++){
            bool hasFluor = probes.contains((char)(base+i));

            m_d->fluorBtns.at(cntBase+i)->setProperty("show", hasFluor);
            m_d->fluorBtns.at(cntBase+i)->setProperty("showbutton", (isSample && hasFluor));

            m_d->fluorBtns.at(cntBase+i)->setVisible(isSample && hasFluor);
        }
        cntBase += GHelper::countOfFluorGroup(ch+1);
    }

    qDebug() << Q_FUNC_INFO << "begin";

    //当前显示的荧光素改为无效时,修改当前荧光素为第一个显示的荧光素
    if(!m_d->fluorBtns.at(m_d->curFluorIndex)->property("showbutton").toBool()){
        int pos = -1;
        for(int i=0; i<m_d->fluorBtns.count(); i++){
            if(m_d->fluorBtns.at(i)->property("showbutton").toBool()){
                pos = i;
                break;
            }
        }
        if(pos < 0){
            //不管当前是什么界面,都转到温度界面
            if(ui->toolBox->currentIndex() != 1)
                ui->toolBox->setCurrentIndex(1);
            ui->toolBox->setItemEnabled(0,false);
        }else{
            changeFluorButtonStatus(pos);

            if(!ui->toolBox->isItemEnabled(0))
                ui->toolBox->setItemEnabled(0,true);

            //如果当前就是荧光素显示界面,则刷新
            if(ui->toolBox->currentIndex() == 0){
                displayFluorOrHeatMap(m_d->curFluorIndex);
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "end";
}

/*!
* \brief 类GRawData的公共槽函数， 实现右侧工具栏通道按键标题及使能的变化
* \param channel 通道序号
* \param enable 设置该通道是否有效
* \param probe 设置该通道的探针类型
* \return 无
*/
void GRawData::slot_channelItemChanged(int channel, bool enable, const QByteArray &probe)
{
    if(channel < 0 || channel >= GHelper::channel_count) return;

    qDebug()<< Q_FUNC_INFO << "step1" << GHelper::byteArrayToHexStr(probe);

    bool isSample = set_to_sampling();
    int cntBase = 0;
    for(int ch=0; ch<channel; ch++){
        cntBase += GHelper::countOfFluorGroup(ch+1);
    }

    int wellMax = GHelper::row_count * GHelper::column_count;

    int base = GHelper::indexOfFluorGroup(channel+1);
    for(int i=0; i<GHelper::countOfFluorGroup(channel+1); i++){
        bool isvisible = false;
        int key = base + i;
        if(enable && probe.contains(key)) isvisible = true;
        m_d->fluorBtns.at(cntBase+i)->setProperty("show", isvisible);
        m_d->fluorBtns.at(cntBase+i)->setProperty("showbutton", isSample && isvisible);
        m_d->fluorBtns.at(cntBase+i)->setVisible(isSample && isvisible);

        quint32 the_color = m_fluorMaps.value(key).first;
        QByteArray wells = m_fluorMaps.value(key).second;
        QList<int> ampWells = m_d->ampFluorCurves.value(key)->keys();
        QList<int> meltWells = m_d->meltFluorCurves.value(key)->keys();

        for(int w=0; w<wells.count(); w++){
            int index = wells.at(w);
            if(index<0 || index>=wellMax) continue;

            CurveMap *ampCurves = m_d->ampFluorCurves.value(key);
            if(ampCurves->contains(index)){
                //删除key
                ampWells.removeOne(index);
            }else{
                //添加扩增曲线
                QwtPlotCurve *curve = new QwtPlotCurve;
                curve->setPen(QColor(the_color));
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                ampCurves->insert(index,curve);

                //添加扩增Y轴数据
                QVector<double> xx,yy;
                if(mn_dataAmpPtr){
                    int xcount = mn_dataAmpPtr->count();
                    if(xcount > 0){
                        for(int c=0; c<xcount; c++){
                            if(mn_dataAmpPtr->at(c) == NULL) continue;
                            xx.append(c+1);
                            yy.append(mn_dataAmpPtr->at(c)->FluorValue.Fluor[channel + 6*index]);
                        }
                    }
                }

                //添加滤波
                int incnt = xx.size(), outcnt = 0;
                QVector<double> filterYY;
                if(incnt > 0)
                    filterYY.fill(0.0, incnt);

                filtAmpSingleCurve_sixChannel(yy.data(), incnt, filterYY.data(), outcnt);
                curve->setSamples(xx, filterYY);


            }

            CurveMap *meltCurves = m_d->meltFluorCurves.value(key);
            if(meltCurves->contains(index)){
                //删除key
                meltWells.removeOne(index);
            }else{
                //添加熔解曲线
                QwtPlotCurve *curve = new QwtPlotCurve;
                curve->setPen(QColor(m_fluorMaps.value(key).first));
                curve->setRenderHint(QwtPlotItem::RenderAntialiased);
                meltCurves->insert(index,curve);

                //添加熔解Y轴数据
                QVector<double> xx,yy;
                if(mn_dataMeltPtr){
                    int xcount = mn_dataMeltPtr->count();
                    if(xcount > 0){
                        for(int c=0; c<xcount; c++){
                            if(mn_dataMeltPtr->at(c) == NULL) continue;
                            xx.append(c+1);
                            yy.append(mn_dataMeltPtr->at(c)->FluorValue.Fluor[channel + 6*index]);

                        }
                    }
                }

                //添加滤波
                int incnt = xx.size(), outcnt = 0;
                QVector<double> filterYY;
                if(incnt > 0)
                    filterYY.fill(0.0, incnt);
                filtSingleCurve_sixChannel(yy.data(), incnt, filterYY.data(), outcnt);
                curve->setSamples(xx, filterYY);

                meltWells.append(index);
            }
        }

        //如果还存在index,则删除
        if(!ampWells.isEmpty()){
            for(int i=0; i<ampWells.count(); i++){
                QwtPlotCurve *curve = m_d->ampFluorCurves.value(key)->value(i);
                if(curve){
                    curve->detach();
                    delete curve;
                    m_d->ampFluorCurves.value(key)->remove(i);
                }
                if(m_d->ampFluorY.value(key)->contains(i)){
                    QVector<double> *dats = m_d->ampFluorY.value(key)->value(i);
                    if(dats){
                        delete dats;
                        m_d->ampFluorY.value(key)->remove(i);
                    }
                }
            }
            for(int i=0; i<meltWells.count(); i++){
                QwtPlotCurve *curve = m_d->meltFluorCurves.value(key)->value(i);
                if(curve){
                    curve->detach();
                    delete curve;
                    m_d->meltFluorCurves.value(key)->remove(i);
                }
                if(m_d->meltFluorY.value(key)->contains(i)){
                    QVector<double> *dats = m_d->meltFluorY.value(key)->value(i);
                    if(dats){
                        delete dats;
                        m_d->meltFluorY.value(key)->remove(i);
                    }
                }
            }
        }
    }
}

/*!
* \brief 类GRawData的公共槽函数， 检测无采样时，隐藏通道按键
* \param 无
* \return 无
*/
void GRawData::slot_sampleChanged()
{    
    bool isSample = set_to_sampling();

    for(int i=0; i<m_d->fluorBtns.count(); i++){
        if(isSample){
            bool vb = m_d->fluorBtns.at(i)->property("show").toBool();
            if(m_d->fluorBtns.at(i)->property("showbutton").toBool() != vb){
                m_d->fluorBtns.at(i)->setProperty("showbutton",vb);
                m_d->fluorBtns.at(i)->setVisible(vb);
            }
        }else{
            m_d->fluorBtns.at(i)->setVisible(false);
        }
    }
    changeFluorButtonStatus(0);
}

/*!
* \brief 类GRawData的公共槽函数， 温控设置变化时，当前状态信息及温控曲线也随之变化
* \param 无
* \return 无
*/
void GRawData::updateTempDisplay()
{    
    init_temp_method();
    displayStatus();

    if(ui->toolBox->currentIndex() == 1){
        plotChanged(ct_Temperature);
        if(m_d->curTempIndex == SELECT_BTN_TEMP){
            m_d->tempItem->initFresh();
            m_d->plot->replot();
        }else{
            displayTempCurve(m_d->timeCount);
        }
    }else{
        int curMapIndex = m_d->testType < 3 ? 1 : 2;
        updateCurrentWidget(curMapIndex);
    }
}

/*!
* \brief 类GRawData的公共槽函数， 选择的实验文件变化时，当前状态栏信息及荧光值曲线或温控曲线也随之变化
* \param 无
* \return 无
*/
void GRawData::updateFluorDisplay()
{
    qDebug() << Q_FUNC_INFO;

    init_temp_method();

    //根据实验温控设置,设置通道按键visible属性
    slot_sampleChanged();

    //选择第一个荧光按键
    for(int i=0; i<m_d->fluorBtns.count(); i++){
        if(m_d->fluorBtns.at(i)->property("showbutton").toBool()){
            changeFluorButtonStatus(i);
            break;
        }
    }

#ifndef DEVICE_TYPE_TL13
    //清空原来的曲线及数据
    foreach(CurveMap *curves, m_d->ampFluorCurves.values()){
        if(curves == NULL) continue;
        foreach(QwtPlotCurve *curve, curves->values()){
            if(curve == NULL) continue;
            curve->detach();
        }
        qDeleteAll(curves->values());
        curves->clear();
    }
    foreach(CurveMap *curves, m_d->meltFluorCurves.values()){
        if(curves == NULL) continue;
        foreach(QwtPlotCurve *curve, curves->values()){
            if(curve == NULL) continue;
            curve->detach();
        }
        qDeleteAll(curves->values());
        curves->clear();
    }
    int wellMax = GHelper::row_count * GHelper::column_count;

    //刷新扩增和熔解荧光值数据
    qDebug() << Q_FUNC_INFO << "show data curve";


    displayFluorOrHeatMap(m_d->curFluorIndex);

    qDebug() << Q_FUNC_INFO << m_d->curFluorIndex << ui->toolBox->currentIndex() << m_d->curTempIndex << m_d->testType << m_d->curShowIndex;

    ui->labelRemainTime->setText("--");
    ui->labelCycle->setText("--/--");
    ui->labelStage->setText("--/--");
    ui->labelStep->setText("--/--");

    if(ui->toolBox->currentIndex() == 1){
        if(m_d->curTempIndex == SELECT_BTN_TEMP){
            m_d->tempItem->update();
            m_d->plot->replot();
        }else{
            displayTempCurve(m_d->curFluorIndex);
        }
    }else{
        int curMapIndex = m_d->testType < 3 ? 1 : 2;
        if(m_d->curShowIndex > curMapIndex){
            m_d->curShowIndex = 0;
        }
    }
#endif
}

/*!
* \brief 类GRawData的公共槽函数，用于启动和停止监控
* \param 无
* \return 无
*/
void GRawData::monitorCtrl(int state)
{
    qDebug() << Q_FUNC_INFO << state << ", curve size:" << m_d->ampFluorX.size();
    switch(state){
    case 0:{
        qDebug() << Q_FUNC_INFO << "等待最后一个荧光值读取,超过2秒退出等待";
        //等待最后一个荧光值读取,超过2秒退出等待
        QElapsedTimer timer;
        timer.start();
        while(m_pool->readingFluor){
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            if(timer.hasExpired(2000)) break;
        }
        qDebug() << Q_FUNC_INFO << "等待结束";
        m_d->tempItem->stopExp();
        m_d->plot->replot();

        if(m_d->timeId){
            killTimer(m_d->timeId);
            m_d->timeId = 0;
        }

        m_d->isRuning = false;

        this->setProperty("isPausing", false);
        this->setProperty("autoShow", false);

        ui->labelRemainTime->setText("--");

        //可能中间关闭实验，再次确认最后一次扩增和熔解的stageno和曲线坐标
        m_d->last_amp_stage_no = mn_dataAmpPtr->isEmpty() ? -1 : mn_dataAmpPtr->first()->No;
        if(m_d->last_amp_stage_no >= 0){
            int amp_stage_cnt = 0;
            for(int i=m_d->last_amp_stage_no; i>=0; i--){
                if(m_d->sample_step_nos.contains(i)) amp_stage_cnt++;
            }
        }

        m_d->la4_stage_no = mn_dataMeltPtr->isEmpty() ? -1 : mn_dataMeltPtr->first()->No;
        if(m_d->la4_stage_no >= 0){
            int melt_stage_cnt = 0;
            for(int i=m_d->la4_stage_no; i>=0; i--){
                if(m_d->sample_step_nos.contains(i)) melt_stage_cnt++;
            }
        }

        //结束时判断是否当前界面还没有采样，退回到前一个界面
        _PCR_RUN_CTRL_INFO pcrInfo;
        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        int last_stage_no = m_d->last_amp_stage_no < m_d->la4_stage_no ? m_d->la4_stage_no : m_d->last_amp_stage_no;

        if(last_stage_no>=0 && m_d->sample_step_nos.contains(pcrInfo.SizeInfo.No.Stage) && pcrInfo.SizeInfo.No.Stage!=last_stage_no){
            qDebug() << Q_FUNC_INFO << "--- 2 ---" << pcrInfo.SizeInfo.No.Stage << last_stage_no;

            //如果当前界面就是荧光界面，刷新
            if(ui->toolBox->currentIndex()==0){
                int curMapIndex = m_d->testType < 3 ? 1 : 2;

                //更新显示
                int curStackedWidgetIndex = m_d->curShowIndex < curMapIndex ? 0 : 1;

                if(ui->stackedWidget->currentIndex() == curStackedWidgetIndex){
                    plotChanged(ct_Fluor);
                    displayFluorOrHeatMap( m_d->curShowIndex);
                }
            }
        }
        qDebug() << Q_FUNC_INFO << "--- 3 ---";
        //清空荧光值曲线数据
        m_d->ampFluorX.clear();
        foreach(VectorMap *fluorYs, m_d->ampFluorY.values()){
            if(fluorYs == NULL) continue;
            foreach(QVector<double> *yy, fluorYs->values()){
                if(yy == NULL) continue;
                yy->clear();
            }
        }
        m_d->meltFluorX.clear();
        foreach(VectorMap *fluorYs, m_d->meltFluorY.values()){
            if(fluorYs == NULL) continue;
            foreach(QVector<double> *yy, fluorYs->values()){
                if(yy == NULL) continue;
                yy->clear();
            }
        }
        qDebug() << Q_FUNC_INFO << "--- 4 ---";
        break;
    }
    case 1:
        memcpy((void*)&m_d->preInfo, (const void *)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));

        //如果是从暂停状态转换过来,则退出
        if(m_d->isRuning) return;

        ui->labelRemainTimeTitle->setText(tr("Remaining Time:"));

        //自动转到温度界面
        if(ui->toolBox->currentIndex() != 1)
            ui->toolBox->setCurrentIndex(1);

        if(!this->property("GetPrevTemp").toBool()){
            //RAW界面
            double max_time = m_d->timeCount < 120 ? 120 : m_d->timeCount;
            m_d->plot->setAxisScale(QwtPlot::xBottom,0, max_time);

            m_d->tempCurve->setSamples(QVector<QPointF>());
            m_d->hotCoverCurve->setSamples(QVector<QPointF>());

            m_d->tempX.clear();
            m_d->tempY.clear();
            m_d->hotcover.clear();

            m_d->timeCount = 0;
        }
        this->setProperty("GetPrevTemp", false);

        //Temp界面
        m_d->tempItem->initFresh();
        if(m_d->curTempIndex == SELECT_BTN_TEMP){
            m_d->plot->replot();
        }

        qDebug() << Q_FUNC_INFO << this->property("isPausing").toBool() << this->property("GetPrevData").toBool();

        if(!this->property("isPausing").toBool() && !this->property("GetPrevData").toBool()){
            //清空存储区
            m_d->ampFluorX.clear();
            foreach(VectorMap *fluorYs, m_d->ampFluorY.values()){
                if(fluorYs == NULL) continue;
                foreach(QVector<double> *yy, fluorYs->values()){
                    if(yy == NULL) continue;
                    yy->clear();
                }
            }
            m_d->meltFluorX.clear();
            foreach(VectorMap *fluorYs, m_d->meltFluorY.values()){
                if(fluorYs == NULL) continue;
                foreach(QVector<double> *yy, fluorYs->values()){
                    if(yy == NULL) continue;
                    yy->clear();
                }
            }
            if(mn_dataAmpPtr){
                qDeleteAll(*mn_dataAmpPtr);
                mn_dataAmpPtr->clear();
            }
            if(mn_dataMeltPtr){
                qDeleteAll(*mn_dataMeltPtr);
                mn_dataMeltPtr->clear();
            }
        }

        qDebug() << Q_FUNC_INFO << "size:" << mn_dataAmpPtr->count() << mn_dataMeltPtr->count();

        this->setProperty("isPausing", false);
        this->setProperty("GetPrevData", false);
        this->setProperty("autoShow", true);

        m_d->thkeep = 0;

        //开始监控
        m_d->isRuning = true;
        m_d->timeId = startTimer(1000);
        qDebug() << "start monitor timer";
        break;
    case 2:
        qDebug() << "set pause flag";
        this->setProperty("isPausing", true);
        break;
    default:break;
    }
}

/*!
* \brief 类GRawData的公共槽函数，用于接收荧光数据
* \param 无
* \return 无
*/
void GRawData::slot_fluor_scan_info(const QByteArray &dat)
{
    //如果是待机时，不再导入数据
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState == 0){
        qDebug() << Q_FUNC_INFO << "取消待机时导入的数据";
        return;
    }

    //转换为结构体
    _FLUOR_SCAN_INFO *scanInfo = new _FLUOR_SCAN_INFO;
    memcpy((void*)scanInfo, (const void*)(dat.data()), scanInfo->_size());

    //荧光值处理
    GWellFluorDataInfo_n *info_ptr = scanInfo->Type ? mn_dataMeltPtr : mn_dataAmpPtr;
    if(info_ptr == 0){
        delete scanInfo;
        return;
    }

    //如果已有数据,判断stageno是否一致,不一致时,清空原来的数据
    if(!info_ptr->isEmpty() && info_ptr->first()->No!=scanInfo->No){
        qDebug() << Q_FUNC_INFO << "删除之前的数据";

        qDeleteAll(*info_ptr);
        info_ptr->clear();

        QVector<double> &_xx_ = scanInfo->Type ? m_d->meltFluorX : m_d->ampFluorX;
        QMap<int, VectorMap* > &_yy_ = scanInfo->Type ? m_d->meltFluorY : m_d->ampFluorY;

        _xx_.clear();
        foreach(VectorMap *fluorYs, _yy_.values()){
            if(fluorYs == NULL) continue;
            qDeleteAll(fluorYs->values());
            fluorYs->clear();
        }
    }

    info_ptr->append(scanInfo);

    if(scanInfo->Type){
        m_d->meltFluorX.append(m_d->ampFluorX.size()+1);
    }else{
        m_d->ampFluorX.append(m_d->ampFluorX.size()+1);
    }
}

/*!
* \brief 类GRawData的公共槽函数，用于获得最后的热图数据
* \param 无
* \return 无
*/
void GRawData::slot_get_hotmap_data(int fluor, int channel)
{
    QMap<int,double> hotmap_dat_;

    CurveMap *fluor_curves = m_d->ampFluorCurves.value(fluor);

    int well_max_ = GHelper::row_count * GHelper::column_count;
    for(int well_indx_=0; well_indx_<well_max_; well_indx_++){

        if(fluor_curves->contains(well_indx_) && fluor_curves->value(well_indx_) && fluor_curves->value(well_indx_)->dataSize()>0){
            int last_indx_ = fluor_curves->value(well_indx_)->dataSize()-1;
            hotmap_dat_.insert(well_indx_, fluor_curves->value(well_indx_)->sample(last_indx_).y());
        }
    }
    emit sendHotmapData(hotmap_dat_);
}

/*!
* \brief 类GRawData的继承事件，用于动态切换语言
* \param 无
* \return 无
*/
void GRawData::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);//在此处刷新语言的

        ui->toolBox->setItemText(0, tr("\nFluorescence\n"));
        ui->toolBox->setItemText(1, tr("\nTemperature\n"));

        int curveType = -1;  //0扩增;1熔解;
        QString currentTitle;
        int curMapIndex = m_d->testType < 3 ? 1 : 2;
        //显示曲线
        if(curMapIndex == 2){
            switch(m_d->curShowIndex){
            case 1:currentTitle = tr("Melting");break;
            case 2:currentTitle = tr("Heat Map");break;
            default:currentTitle = tr("Amplification");break;
            }
            curveType = (m_d->curShowIndex == 1) ? 1 : 0;
        }else if(curMapIndex == 1){
            switch(m_d->curShowIndex){
            case 1:currentTitle = tr("Heat Map");break;
            default:currentTitle = (m_d->testType & 0x02) ? tr("Melting") : tr("Amplification");break;
            }
            curveType = (m_d->testType & 0x02) ? 1 : 0;
        }
        if(curveType == 0)
            m_d->plot->setAxisTitle(QwtPlot::xBottom, tr("Cycles"));
        else  if(curveType == 1)
            m_d->plot->setAxisTitle(QwtPlot::xBottom, tr("Temperature (%1)").arg(trUtf8("℃")));
        ui->labelCurrent->setText(currentTitle);

        //刷新绘图中的中英文

        m_d->tempItem->update();
        m_d->plot->replot();

    }
    QWidget::changeEvent(e);
}

/*!
* \brief 类GRawData的继承事件，用于定时器控制
* \param e 定时器事件
* \return 无
*/
void GRawData::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == m_d->timeId){
        //如果同一包信息已经刷新，则退出等待下一组
        if(m_pool->info_refresh_flag == 0) return;

        //获得仪器信息和信息刷新标志
        _PCR_RUN_CTRL_INFO pcrInfo;
        m_pool->mutex.lock();
        m_pool->info_refresh_flag = 0;
        memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        //如果是待机状态，停止时钟；否则，实时刷新
        if(pcrInfo.State.ExpState == 0 && !m_pool->readingFluor){
            killTimer(m_d->timeId);
            m_d->timeId = 0;
        }

        qDebug() << Q_FUNC_INFO << pcrInfo.SizeInfo.No.Stage << pcrInfo.SizeInfo.No.Step << pcrInfo.SizeInfo.No.Cycle << pcrInfo.State.AmpFlag;

        bool isKeepTemp = pcrInfo.SizeInfo.No.Stage==m_pool->infinite_stage_no && pcrInfo.SizeInfo.No.Step==m_pool->infinite_step_no;
        //当前时间变化时,开始显示剩余运行时间
        if(m_runInfo->lidStatus>0 && pcrInfo.SizeInfo.ExpTime.CurrentTimes==0){
            ui->labelRemainTime->setText(tr("--"));
            m_d->tempItem->setHotLidMode();
        }else if(!isKeepTemp){
            int _tmp = pcrInfo.SizeInfo.ExpTime.TotalTimes-pcrInfo.SizeInfo.ExpTime.CurrentTimes;
            _tmp = (_tmp < 0) ? 0 : _tmp;
            int hours = _tmp / 3600;
            _tmp = _tmp % 3600;
            int minutes = _tmp / 60;
            if(_tmp < 60){
                minutes = 1;
            }

            if(hours == 0){
                ui->labelRemainTime->setText(tr("%1min").arg(QString::number(minutes).leftJustified(2,' ',true)));
            }else{
                ui->labelRemainTime->setText(tr("%1h %2min").arg(QString::number(hours).leftJustified(2,' ',true)).arg(QString::number(minutes).leftJustified(2,' ',true)));
            }
        }else{
            ui->labelRemainTimeTitle->setText(tr("Status:"));
            ui->labelRemainTime->setText(QString("∞"));
        }

        //如果是开始升温热盖时显示
        bool isTwinkle = ((m_runInfo->lidStatus==0 || pcrInfo.SizeInfo.ExpTime.CurrentTimes!=0));

        if(isTwinkle){
            ui->labelCycle->setText(tr("%1/%2").arg(QString::number(pcrInfo.SizeInfo.No.Cycle+1).rightJustified(2,'0',true)).arg(QString::number(pcrInfo.SizeInfo.Size.Cycle).rightJustified(2,'0',true)));
            ui->labelStage->setText(tr("%1/%2").arg(QString::number(pcrInfo.SizeInfo.No.Stage+1).rightJustified(2,'0',true)).arg(QString::number(pcrInfo.SizeInfo.Size.Stage).rightJustified(2,'0',true)));
            ui->labelStep->setText(tr("%1/%2").arg(QString::number(pcrInfo.SizeInfo.No.Step+1).rightJustified(2,'0',true)).arg(QString::number(pcrInfo.SizeInfo.Size.Step).rightJustified(2,'0',true)));
        }else{
            ui->labelCycle->setText("--/--");
            ui->labelStage->setText("--/--");
            ui->labelStep->setText("--/--");
        }

        if(!isKeepTemp){
            //时间累计
            m_d->timeCount++;
            m_d->tempX.append(m_d->timeCount);
            double val = (double)(pcrInfo.Temp[0]+pcrInfo.Temp[1]+pcrInfo.Temp[2]) / 300.0;
            m_d->tempY.append(val);
            double  hotlidtemp = (double)pcrInfo.Temp[4]/100.0;
            m_d->hotcover.append(hotlidtemp);
            m_d->tempCurve->setSamples(m_d->tempX, m_d->tempY);
            m_d->hotCoverCurve->setSamples(m_d->tempX, m_d->hotcover);
        }

        //运行中,如果stage变化,并且该stage有采样,清空同类型变量中的数据和曲线
        bool need_change_curve = false;
        if(pcrInfo.State.ExpState!=0 && m_d->sample_step_nos.contains(pcrInfo.SizeInfo.No.Stage) && pcrInfo.SizeInfo.No.Stage!=m_d->preInfo.SizeInfo.No.Stage){
            need_change_curve = true;
        }

        qDebug() << Q_FUNC_INFO << "刷新曲线";
        //刷新曲线
        if(ui->toolBox->currentIndex() != 0){
            //如果当前是温度界面
            if(m_d->curTempIndex == SELECT_BTN_TEMP){
                //如果有热盖控制，查看或显示热盖信息
                if(isTwinkle){
                    if(pcrInfo.State.AmpFlag == 0){
                        if((pcrInfo.SizeInfo.No.Cycle != m_d->preInfo.SizeInfo.No.Cycle) || (pcrInfo.SizeInfo.No.Stage != m_d->preInfo.SizeInfo.No.Stage) || (pcrInfo.SizeInfo.No.Step != m_d->preInfo.SizeInfo.No.Step)){
                            m_d->thkeep = 0;
                        }else if(pcrInfo.State.ThKeep){
                            m_d->thkeep = 1;
                        }
                        m_d->tempItem->runExp(pcrInfo.SizeInfo.No.Cycle, pcrInfo.SizeInfo.No.Stage, pcrInfo.SizeInfo.No.Step, m_d->thkeep, pcrInfo.SizeInfo.StepTime.CurrentTimes,pcrInfo.State.ThState);
                    }else{
                        m_d->tempItem->runExp(pcrInfo.SizeInfo.No.Cycle, pcrInfo.SizeInfo.No.Stage, pcrInfo.SizeInfo.No.Step, pcrInfo.State.ThKeep, pcrInfo.SizeInfo.StepTime.CurrentTimes,pcrInfo.State.ThState);
                    }
                }
            }else{
                double max_time = m_d->timeCount < 120 ? 120 : m_d->timeCount;
                m_d->plot->setAxisScale(QwtPlot::xBottom,0, max_time);
            }
            m_d->plot->replot();
        }else{
            //如果当前是荧光界面

            //自动显示时, 判断当前荧光显示界面是否正确
            if(this->property("autoShow").toBool()){
                if(m_runMethod->at(pcrInfo.SizeInfo.No.Stage)->Property == 0){
                    if(m_d->sample_step_nos.contains(pcrInfo.SizeInfo.No.Stage) && m_d->curShowIndex != 0){
                        m_d->curShowIndex = 0;
                        need_change_curve = true;
                    }
                }else{
                    int indx = m_d->testType < 3 ? 0 : 1;
                    if(m_d->curShowIndex != indx){
                        m_d->curShowIndex = indx;
                        need_change_curve = true;
                    }
                }
            }

            if(need_change_curve){
                int curMapIndex = m_d->testType < 3 ? 1 : 2;

                //更新显示
                int curStackedWidgetIndex = m_d->curShowIndex < curMapIndex ? 0 : 1;

                if(ui->stackedWidget->currentIndex() != curStackedWidgetIndex)
                    ui->stackedWidget->setCurrentIndex(curStackedWidgetIndex);

                plotChanged(ct_Fluor);

                //刷新当前窗口的描述
                QString currentTitle;
                if(curMapIndex == 2){
                    switch(m_d->curShowIndex){
                    case 1:currentTitle = tr("Melting");break;
                    case 2:currentTitle = tr("Heat Map");break;
                    default:currentTitle = tr("Amplification");break;
                    }
                }else if(curMapIndex == 1){
                    switch(m_d->curShowIndex){
                    case 1:currentTitle = tr("Heat Map");break;
                    default:currentTitle = (m_d->testType & 0x02) ? tr("Melting") : tr("Amplification");break;
                    }
                }

                ui->labelCurrent->setText(currentTitle);
            }
            displayFluorOrHeatMap( m_d->curShowIndex);
        }

        memcpy(&m_d->preInfo, &pcrInfo, sizeof(_PCR_RUN_CTRL_INFO));

        qDebug() << "=================== (1 s) end ========================";
    }

    QWidget::timerEvent(e);
}

/*!
* \brief 类GRawData的继承事件，用于温度绘图翻页控制
* \param e 定时器事件
* \return 无
*/
bool GRawData::eventFilter(QObject *obj, QEvent *ev)
{    
    if(obj == m_d->plot){
        switch(ev->type()){
        case QEvent::MouseButtonPress:{
            QMouseEvent *e = static_cast<QMouseEvent*>(ev);
            if(e && ui->toolBox->currentIndex() == 1 && m_d->curTempIndex == SELECT_BTN_TEMP){
                m_d->tempItem->clickButton(m_d->plot->canvas()->mapFromGlobal(e->globalPos()),true);
                m_d->plot->replot();
            }
            break;
        }
        case QEvent::MouseButtonRelease:{
            QMouseEvent *e = static_cast<QMouseEvent*>(ev);
            if(e && ui->toolBox->currentIndex() == 1 && m_d->curTempIndex == SELECT_BTN_TEMP){
                m_d->tempItem->clickButton(m_d->plot->canvas()->mapFromGlobal(e->globalPos()),false);
                m_d->plot->replot();
            }
            break;
        }
        case QEvent::MouseMove:{
            QMouseEvent *e = static_cast<QMouseEvent*>(ev);
            if(e && ui->toolBox->currentIndex() == 1 && m_d->curTempIndex == SELECT_BTN_TEMP){
                m_d->tempItem->moveButton(m_d->plot->canvas()->mapFromGlobal(e->globalPos()));
                m_d->plot->replot();
            }
            break;
        }
        default:break;
        }
    }

    return QWidget::eventFilter(obj,ev);
}

/*!
* \brief 类GRawData的私有函数，初始化私有变量
* \param 无
* \return 无
*/
void GRawData::initVariables()
{    
    //初始化变量
    m_d->fluorArea = new QScrollArea;
    m_d->fluorViewport = new QWidget;
    m_d->fluorArea->setWidget(m_d->fluorViewport);
    m_d->fluorArea->setWidgetResizable(true);
    m_d->fluorArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_d->fluorArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_d->fluorArea->setFocusPolicy(Qt::NoFocus);
    m_d->fluorArea->setFrameShape(QFrame::NoFrame);
    m_d->fluorArea->setObjectName("pageFluor");
    m_d->fluorArea->setProperty("class","SideBar");

    //添加荧光素按键
    QVBoxLayout *vl = new QVBoxLayout(m_d->fluorViewport);
    vl->setSpacing(0);
    vl->setMargin(0);
    vl->setContentsMargins(0,0,0,0);

    //pageFluor 把荧光素按钮放在一个list里为了方便管理
    QFontMetrics metrics(qApp->font());
    m_d->fluorBtns.clear();
    for(int ch=0; ch<GHelper::channel_count; ch++){
        for(int i=0; i<GHelper::countOfFluorGroup(ch+1); i++){
            QToolButton *btn = new QToolButton;
            btn->setObjectName("toolButton_"+QString::number(ch+1)+"_"+QString::number(i));
            int key = ((ch+1)<<0x04) + i;
            btn->setProperty("value",key);
            btn->setProperty("showbutton", true);
#ifdef DEVICE_TYPE_TL22
            if((GHelper::total_instrument_id == 201 && ch == 4) || ((GHelper::total_instrument_id==202 || GHelper::total_instrument_id==203) && ch == 5)){
                btn->setObjectName("toolButton_"+QString::number(ch+1)+"_"+QString::number(i+1));
                key = ((ch+1)<<0x04) + i + 1;
                btn->setProperty("value",key);
                btn->setText(metrics.elidedText(GHelper::keyOfFluor(ch+1,i+1), Qt::ElideRight, 80));
            }else if( GHelper::total_instrument_id == 204 && ch == 5 ){
                btn->setObjectName("toolButton_"+QString::number(ch+1)+"_4");
                key = ((ch+1)<<0x04) + 4;
                btn->setProperty("value",key);
                btn->setText(metrics.elidedText(GHelper::keyOfFluor(ch+1,4), Qt::ElideRight, 80));
            }else if( GHelper::total_instrument_id == 205 && ch == 5 ){
                btn->setObjectName("toolButton_"+QString::number(ch+1)+"_5");
                key = ((ch+1)<<0x04) + 5;
                btn->setProperty("value",key);
                btn->setText(metrics.elidedText(GHelper::keyOfFluor(ch+1,5), Qt::ElideRight, 80));
            }else{
                btn->setText(metrics.elidedText(GHelper::keyOfFluor(ch+1,i), Qt::ElideRight, 80));
            }
#else
            btn->setText(metrics.elidedText(GHelper::keyOfFluor(ch+1,i), Qt::ElideRight, 80));
#endif

            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            btn->setMinimumHeight(FLUOR_BTN_HEIGHT);
            btn->setMaximumHeight(FLUOR_BTN_HEIGHT);
            btn->setFocusPolicy(Qt::NoFocus);
            vl->addWidget(btn);
            m_d->fluorBtns.append(btn);
            connect(btn, SIGNAL(clicked()), this, SLOT(slot_fluorBtn_clicked()));
            //添加扩增和熔解的曲线和数据的表
            m_d->ampFluorCurves.insert(key, new QMap<int,QwtPlotCurve*>());
            m_d->meltFluorCurves.insert(key, new QMap<int,QwtPlotCurve*>());

            m_d->ampFluorY.insert(key, new QMap<int,QVector<double>* >());
            m_d->meltFluorY.insert(key, new QMap<int,QVector<double>* >());
        }
    }
    //添加弹簧
    vl->addItem(new QSpacerItem(20,10,QSizePolicy::Preferred,QSizePolicy::Expanding));

    ui->toolBox->insertItem(0, m_d->fluorArea, tr("\nFluorescence\n"));
    ui->toolBox->setItemText(1, tr("\nTemperature\n"));

    //选择第一个荧光素
    changeFluorButtonStatus(0);
    //初始化显示序号,显示扩增曲线

    m_d->curShowIndex = 0;
    //pageTemp 把选择按钮放在一个list里为了方便管理

    m_d->tempBtns.clear();
    m_d->tempBtns.append(ui->toolButton_temp);
    //
    foreach (QToolButton *b, m_d->tempBtns) {
        connect(b, SIGNAL(clicked()), this, SLOT(slot_tempBtn_clicked()));
    }

    //选择显示温度曲线
    changeTempButtonStatus(0);
    //toolbox设置为pageTemp页
    connect(ui->toolBox, SIGNAL(currentChanged(int)), this, SLOT(slot_toolBox_currentChanged(int)));
    ui->toolBox->setCurrentIndex(1);

    //新建温度曲线变量
    m_d->plot = new QwtPlot;
    if(m_d->plot){
        m_d->plot->plotLayout()->setAlignCanvasToScales(true);

        QPalette canvasPalette( Qt::white );
        canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
        m_d->plot->canvas()->setPalette( canvasPalette );

        //        m_d->plot->setCanvasLineWidth(1);
        //        m_d->plot->canvas()->setFrameStyle(QFrame::Box | QFrame::Plain);

        m_d->grid = new QwtPlotGrid;
        if(m_d->grid){
            m_d->grid->setPen(QPen(QBrush(Qt::blue), 1, Qt::DotLine));
            m_d->grid->attach(m_d->plot);
        }

        m_d->plot->plotLayout()->setCanvasMargin(0);
        m_d->plot->plotLayout()->setSpacing(0);

        m_d->plot->titleLabel()->setMargin(0);

        m_d->plot->setMouseTracking(true);
        m_d->plot->installEventFilter(this);
    }

    //初始化曲线，设置显示第一通道
    m_d->tempItem = new GRunMethodItem(m_pool, m_runMethod);

    m_d->tempCurve = new QwtPlotCurve;
    if(m_d->tempCurve){
        m_d->tempCurve->setPen(QPen(Qt::blue));
        m_d->tempCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    }

    m_d->hotCoverCurve = new QwtPlotCurve;
    if(m_d->hotCoverCurve){
        m_d->hotCoverCurve->setPen(QPen(Qt::red));
        m_d->hotCoverCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    }

    m_d->tempX.clear();
    m_d->tempY.clear();
    m_d->hotcover.clear();

    //显示温度
    plotChanged(ct_Temperature);

    //新建扩增和熔解曲线变量
    m_d->ampFluorX.clear();
    m_d->meltFluorX.clear();


    //初始化热图的模板和代理
    m_d->model = new QStandardItemModel;
    if(m_d->model){
        QStringList headers;
        for(int i=0; i<GHelper::column_count; i++)
            headers << QString::number(i+1);
        m_d->model->setHorizontalHeaderLabels(headers);
        headers.clear();
        for(int i=0; i<GHelper::row_count; i++)
            headers << QString(i+'A');
        m_d->model->setVerticalHeaderLabels(headers);
    }
    m_d->colorDelegate = new GColorDelegate(m_pool);

    this->setProperty("isPausing", false);
    this->setProperty("GetPrevTemp", false);
    this->setProperty("GetPrevData", false);
    this->setProperty("autoShow", false);   ///< 自动显示扩增或熔解荧光曲线界面
}

/*!
* \brief 类GRawData的私有函数，初始化designer设计的界面
* \param 无
* \return 无
*/
void GRawData::initUi()
{
    //初始化界面
    ui->btnWidget->setVisible(false);

    //设置温度监控界面
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_d->plot);
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    ui->pageRawData->setLayout(hLayout);

    //设置热图监控界面
    ui->tableView->setModel(m_d->model);
#ifndef DEVICE_TYPE_TL13
    int heat_item_width = TABLEVIEW_WIDTH / GHelper::column_count;
    for(int i=0; i<GHelper::column_count; i++)
        ui->tableView->setColumnWidth(i, heat_item_width);
    int heat_item_height = TABLEVIEW_HEIGHT / GHelper::row_count;
    for(int i=0; i<GHelper::row_count; i++)
        ui->tableView->setRowHeight(i, heat_item_height);
#endif
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->tableView->setItemDelegate(m_d->colorDelegate);

    //信号槽连接
    connect(ui->toolBox, SIGNAL(currentChanged(int)), m_pool, SLOT(screen_sound()));
}

/*!
* \brief 类GRawData的私有函数， 判读当前实验是否有采样
* \param sec 秒数
* \return 无
*/
bool GRawData::set_to_sampling()
{
    bool isSampled = false;

    for(int i=0; i<m_runMethod->count(); i++){
        if(m_runMethod->at(i) == NULL) continue;
        for(int j=0; j<m_runMethod->at(i)->SubNum; j++){
            if(m_runMethod->at(i)->ReadFluor[j]){
                isSampled = true;
                break;
            }
        }
        if(isSampled) break;
    }

    return isSampled;
}

/*!
* \brief 类GRawData的私有函数， 实现秒数到时间格式的转换
* \param sec 秒数
* \return 无
*/
QString GRawData::secToString(int sec)
{
    int h,m,s;
    h = (int)sec / 3600;
    m = ((int)sec % 3600) / 60;
    s = ((int)sec % 3600) % 60;
    return QObject::tr("%1:%2:%3").arg(QString::number(h).rightJustified(2,'0',true))
            .arg(QString::number(m).rightJustified(2,'0',true))
            .arg(QString::number(s).rightJustified(2,'0',true));
}

/*!
* \brief 类GRawData的私有函数， 实现pageFluor页通道按键的状态变化
* \param button 状态变化的按键
* \return 无
*/
void GRawData::changeFluorButtonStatus(int index)
{
    if(index < 0 || index >= m_d->fluorBtns.count()) return;
    for(int i=0; i<m_d->fluorBtns.count(); i++){
        QString val = (index == i) ? "true" : "false";
        m_d->fluorBtns.at(i)->setProperty("current", val);
        m_d->fluorBtns.at(i)->setStyleSheet("");  // 刷新按钮的样式
    }
    m_d->curFluorIndex = index;
}

/*!
* \brief 类GRawData的私有函数， 实现pageTemp页选择按键的状态变化
* \param button 状态变化的按键
* \return 无
*/
void GRawData::changeTempButtonStatus(int index)
{
    if(index < 0 || index >= SELECT_BUTTON_COUNT) return;

    for(int i=0; i<m_d->tempBtns.count(); i++){
        QString val = (index == i) ? "true" : "false";
        m_d->tempBtns.at(i)->setProperty("current", val);
        m_d->tempBtns.at(i)->setStyleSheet("");  // 刷新按钮的样式
    }
    m_d->curTempIndex = index;
}

/*!
* \brief 类GRawData的私有函数， 调整曲线显示的横纵坐标的范围
* \param 无
* \return 无
*/
void GRawData::adjustPlotRange()
{    
    if(ui->toolBox->currentIndex() == 1) return;

    int curveType =0;  //0扩增;1熔解;
    int curMapIndex = m_d->testType < 3 ? 1 : 2;
    if(m_d->curShowIndex < curMapIndex){
        //显示曲线
        if(curMapIndex == 2){
            curveType = (m_d->curShowIndex == 1) ? 1 : 0;
        }else if(curMapIndex == 1){
            curveType = (m_d->testType & 0x02) ? 1 : 0;
        }else
            return;
    }else{
        return;
    }

    qDebug() << Q_FUNC_INFO << "begin";

    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    double ymin = 0.0, ymax = 400.0;
    if(m_d->plot->itemList(QwtPlotItem::Rtti_PlotCurve).count() > 0){
        bool isFirst = true;
        foreach(QwtPlotItem *item, m_d->plot->itemList(QwtPlotItem::Rtti_PlotCurve)){
            QwtPlotCurve *curve = static_cast<QwtPlotCurve*>(item);
            if(curve->dataSize() <= 0) continue;

            if(isFirst){
                double tmp = curve->minYValue();
                if(qIsFinite(tmp))
                    ymin = tmp;
                tmp = curve->maxYValue();
                if(qIsFinite(tmp) && ymax < tmp)
                    ymax = tmp;

                isFirst = false;
                continue;
            }
            double tmp = curve->minYValue();
            if(qIsFinite(tmp) && ymin > tmp) ymin = tmp;
            tmp = curve->maxYValue();
            if(qIsFinite(tmp) && ymax < tmp) ymax = tmp;
        }
    }

    //设置Y轴界限值为5的倍数
    ymin = ((int)ymin / 5) * 5;
    ymax = ((int)ymax / 5) * 5 + 5;

    qDebug() << Q_FUNC_INFO << "step1" << pcrInfo.State.ExpState << curveType << m_d->last_amp_stage_no << m_d->la4_stage_no << m_runMethod->count();

    double xMin = 1.0, xMax = 40.0;

    if(curveType == 0){
        if(pcrInfo.State.ExpState == 0){
            //如果是待机时,打开最后一次扩增stage的循环数,如果没有最后一次,打开第1个
            if(m_d->last_amp_stage_no>=0 && m_d->last_amp_stage_no<m_runMethod->count()){
                xMax = m_runMethod->at(m_d->last_amp_stage_no)->Cycles;
            }else{
                for(int i=0; i<m_runMethod->count(); i++){
                    if(m_runMethod->at(i) == NULL) continue;
                    if(m_d->sample_step_nos.contains(i) && m_runMethod->at(i)->Property==0){
                        xMax = m_runMethod->at(i)->Cycles;
                        break;
                    }
                }
            }
        }else{
            //运行中,如果正在跑扩增,显示当前循环数,如果在跑熔解,查找之前第1个扩增的循环数
            bool has_find = false;
            for(int i=pcrInfo.SizeInfo.No.Stage; i>=0; i--){
                if(m_runMethod->at(i)->Property==0 && m_d->sample_step_nos.contains(i)){
                    has_find = true;
                    xMax = m_runMethod->at(i)->Cycles;
                    break;
                }
            }

            //如果前面没有，则查找后面的
            if(!has_find){
                for(int i=pcrInfo.SizeInfo.No.Stage+1; i<m_runMethod->count(); i++){
                    if(m_runMethod->at(i)->Property==0 && m_d->sample_step_nos.contains(i)){
                        xMax = m_runMethod->at(i)->Cycles;
                        break;
                    }
                }
            }
        }
    }else{
        xMin = 40.0, xMax = 98.0;
        bool run_range_ok = false;
        if(pcrInfo.State.ExpState != 0){
            int stage_no_ = -1;

            //运行中,如果正在跑扩增,查找前1个熔解
            if(m_runMethod->at(pcrInfo.SizeInfo.No.Stage)->Property == 0){
                bool has_find = false;
                for(int i=pcrInfo.SizeInfo.No.Stage-1; i>=0; i--){
                    if(m_runMethod->at(i)->Property != 0){
                        has_find = true;
                        stage_no_ = i;
                        break;
                    }
                }
                //如果没有发现，向后找
                if(!has_find){
                    for(int i=pcrInfo.SizeInfo.No.Stage+1; i<m_runMethod->count(); i++){
                        if(m_runMethod->at(i)->Property != 0){
                            stage_no_ = i;
                            break;
                        }
                    }
                }
            }else{
                stage_no_ = pcrInfo.SizeInfo.No.Stage;
            }

            if(m_d->sample_step_nos.contains(stage_no_)){
                int step_no_ = m_d->sample_step_nos.value(stage_no_);
                if(step_no_ < m_runMethod->at(stage_no_)->SubNum+1)
                    xMin = m_runMethod->at(stage_no_)->Temp[step_no_-1];
                if(step_no_ < m_runMethod->at(stage_no_)->SubNum)
                    xMax = m_runMethod->at(stage_no_)->Temp[step_no_];
                run_range_ok = true;
            }
        }
        qDebug() << Q_FUNC_INFO << "step2" << run_range_ok;
        if(!run_range_ok){
            if(m_d->la4_stage_no < 0){
                //显示第1个熔解的坐标
                for(int i=0; i<m_runMethod->count(); i++){
                    if(m_runMethod->at(i) == NULL) continue;
                    if(m_d->sample_step_nos.contains(i) && m_runMethod->at(i)->Property!=0){
                        int step_no_ = m_d->sample_step_nos.value(i);
                        if(step_no_ < m_runMethod->at(i)->SubNum+1)
                            xMin = m_runMethod->at(i)->Temp[step_no_-1];
                        if(step_no_ < m_runMethod->at(i)->SubNum)
                            xMax = m_runMethod->at(i)->Temp[step_no_];
                        break;
                    }
                }
            }else{
                int step_no_ = m_d->sample_step_nos.value(m_d->la4_stage_no);
                if(m_d->la4_stage_no<m_runMethod->count() && step_no_>0){
                    if(step_no_ < m_runMethod->at(m_d->la4_stage_no)->SubNum+1)
                        xMin = m_runMethod->at(m_d->la4_stage_no)->Temp[step_no_-1];
                    if(step_no_ < m_runMethod->at(m_d->la4_stage_no)->SubNum)
                        xMax = m_runMethod->at(m_d->la4_stage_no)->Temp[step_no_];
                }
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "step3" << xMin << xMax;
    //添加横坐标的边界数据
    double deta_val_ = qAbs(xMax - xMin);
    if(deta_val_<=10 && deta_val_>=0){
        m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,1);
    }else if(deta_val_>10 && deta_val_<=50){
        m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,5);
        m_d->plot->replot();

        QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
        QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
        if(!majorTicks.contains(xMax)&&(int)xMax%5==1){
            majorTicks.removeLast();
            majorTicks.append(xMax);
        }
        if(!majorTicks.contains(xMax)&&(int)xMax%5>1){
            majorTicks.append(xMax);
        }
        if(!majorTicks.contains(xMin)&&(int)xMin%5==4){
            majorTicks.removeFirst();
            majorTicks.append(xMin);
        }
        if(!majorTicks.contains(xMin)&&(int)xMin%5<4){
            majorTicks.append(xMin);
        }
        scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
        m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
    }else if(deta_val_>50 && deta_val_<100){
        m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,10);
        m_d->plot->replot();

        QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
        QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
        if(!majorTicks.contains(xMax)&&(int)xMax%10==1){
            majorTicks.removeLast();
            majorTicks.append(xMax);
        }
        if(!majorTicks.contains(xMax)&&(int)xMax%10>1){
            majorTicks.append(xMax);
        }
        if(!majorTicks.contains(xMin)&&(int)xMin%10==9){
            majorTicks.removeFirst();
            majorTicks.append(xMin);
        }
        if(!majorTicks.contains(xMin)&&(int)xMin%10<9){
            majorTicks.append(xMin);
        }
        scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
        m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
    }else{
        m_d->plot->setAxisTitle(QwtPlot::xBottom, "");
        m_d->plot->setAxisScale(QwtPlot::xBottom,0,40);
    }
    qDebug() << Q_FUNC_INFO << "step4";
    m_d->plot->setAxisScale(QwtPlot::yLeft, ymin, ymax);
    m_d->plot->replot();

    qDebug() << Q_FUNC_INFO << "end";
}

/*!
* \brief 类GRawData的私有槽函数， 显示荧光曲线或热图
* \param channel 通道序号
* \return 无
*/
void GRawData::displayFluorOrHeatMap(int fluor)
{
    if(m_runMethod->count() <= 0)  return;

    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(ui->toolBox->currentIndex() != 0) return;
    if(fluor < 0 || fluor >= m_d->fluorBtns.count()) return;

    //清空绘图
    foreach(QwtPlotItem *item, m_d->plot->itemList(QwtPlotItem::Rtti_PlotCurve)){
        item->detach();
    }
    foreach(QwtPlotItem *item, m_d->plot->itemList(QwtPlotItem::Rtti_PlotUserItem)){
        item->detach();
    }
    //得到当前选择的通道
    int fluorDat = m_d->fluorBtns.at(m_d->curFluorIndex)->property("value").toInt();
    int channel = (fluorDat >> 0x04) & 0x0f;
    if(channel <= 0) return; else channel--;

    int well_max_ = GHelper::row_count * GHelper::column_count;

    //绘图上根据选择添加扩增或熔解曲线
    int curMapIndex = m_d->testType < 3 ? 1 : 2;

    qDebug() << Q_FUNC_INFO << fluor << channel << fluorDat << m_d->isRuning <<  m_d->curShowIndex << curMapIndex;

    if(m_d->curShowIndex < curMapIndex){
        qDebug() << Q_FUNC_INFO << "更新荧光曲线";
        //显示曲线
        QMap<int, QMap< int, QwtPlotCurve* > * > *fluorCurves = NULL;
        if(curMapIndex == 2){
            fluorCurves = (m_d->curShowIndex == 1) ? &m_d->meltFluorCurves : &m_d->ampFluorCurves;
        }else if(curMapIndex == 1){
            fluorCurves = (m_d->testType & 0x02) ? &m_d->meltFluorCurves : &m_d->ampFluorCurves;
        }
    }else{
        qDebug() << Q_FUNC_INFO << "更新热图";
        //显示热图, 从当前选择的荧光数据中找到最大的一个荧光值设置为热图的最大值
        double max_val = MAX_COLOR_VAL;
        int stage_pro_ = -1;
        bool show_prev_stage_dat = false;
        QMap<int, double> stage_last_val;

        if(pcrInfo.State.ExpState != 0){
            //如果运行中,查找最后一次采样的曲线
            for(int i=pcrInfo.SizeInfo.No.Stage; i>=0; i--){
                if(i>=m_runMethod->count() || m_runMethod->at(i)==NULL) continue;
                //如果是采样stage
                if(m_d->sample_step_nos.contains(i)){
                    //如果当前stage有数据，就使用当前最后一组，如果没有，就使用前１个stage的最后一组
                    GWellFluorDataInfo_n *dat_ptr_ = (m_runMethod->at(i)->Property==0) ? mn_dataAmpPtr : mn_dataMeltPtr;
                    if(i == pcrInfo.SizeInfo.No.Stage){
                        if(!dat_ptr_->isEmpty() && dat_ptr_->last()->No==pcrInfo.SizeInfo.No.Stage){
                            stage_pro_ = m_runMethod->at(i)->Property;
                            break;
                        }else{
                            continue;
                        }
                    }else{
                        if(!dat_ptr_->isEmpty()){
                            stage_pro_ = m_runMethod->at(i)->Property;
                        }
                        show_prev_stage_dat = true;
                        qDebug() << Q_FUNC_INFO << "stage变化，显示上一个stage最后一组数据";
                        break;
                    }
                }
            }
        }else{
            //待机时,最后一次采样的stage
            int stage_no = m_d->last_amp_stage_no < m_d->la4_stage_no ? m_d->la4_stage_no : m_d->last_amp_stage_no;
            if(stage_no>=0 && stage_no<m_runMethod->count()) stage_pro_ = m_runMethod->at(stage_no)->Property;
        }

        if(stage_pro_ >= 0){
            if(!show_prev_stage_dat){
                CurveMap *curves = (stage_pro_==0) ? m_d->ampFluorCurves.value(fluorDat) : m_d->meltFluorCurves.value(fluorDat);
                if(curves){
                    for(int well_indx_=0; well_indx_<well_max_; well_indx_++){

                        if(curves->contains(well_indx_)){
                            QwtPlotCurve *curve = curves->value(well_indx_);
                            if(curve && curve->dataSize()>0){
                                double tmp = curve->sample(curve->dataSize()-1).y();
                                stage_last_val.insert(well_indx_, tmp);
                                if(qIsFinite(tmp) && tmp > max_val){
                                    max_val = tmp;
                                }
                            }
                        }
                    }
                }
            }

        }

        //设置最大颜色值
        m_d->colorDelegate->setMaxColorValue(max_val);
        ui->labelHotMax->setText(QString::number(max_val,'f',1));

        //设置孔颜色
        for(int well=0; well<well_max_; well++){
            int row = well / GHelper::column_count;
            int col = well % GHelper::column_count;

            double val = -1.0;
            if(stage_last_val.contains(well)){
                val = stage_last_val.value(well);
            }

            m_d->model->setData(m_d->model->index(row,col), val, Qt::EditRole);
        }
    }
}

/*!
* \brief 类GRawData的私有槽函数， 实现温控的曲线的显示
* \param sec 实验运行到的时间（秒）
* \return 无
*/
void GRawData::displayTempCurve(int sec)
{        
    if(m_d->curTempIndex == SELECT_BTN_RAW){
        double max_time = sec < 120 ? 120 : sec;
        m_d->plot->setAxisScale(QwtPlot::xBottom,0, max_time);

        m_d->tempCurve->setSamples(m_d->tempX, m_d->tempY);
        m_d->hotCoverCurve->setSamples(m_d->tempX, m_d->hotcover);
        m_d->plot->replot();
    }
}

/*!
* \brief 类GRawData的私有槽函数， 显示状态栏信息的变化
* \param type 曲线显示的类型：荧光值或温度
* \return 无
*/
void GRawData::displayStatus()
{
    ui->labelRemainTime->setText("--");

    //刷新状态栏显示
    if(m_runMethod && m_runMethod->count()>0){
        int stage = m_runMethod->count();
        int cycle = m_runMethod->first()->Cycles;
        int step = m_runMethod->first()->SubNum;

        ui->labelCycle->setText(QString("0/%1").arg(cycle));
        ui->labelStage->setText(QString("0/%1").arg(stage));
        ui->labelStep->setText(QString("0/%1").arg(step));
    }else{
        ui->labelCycle->setText("--/--");
        ui->labelStage->setText("--/--");
        ui->labelStep->setText("--/--");
    }
}

/*!
* \brief 类GRawData的私有槽函数， 实现温度曲线和荧光值曲线变换时坐标系的变化
* \param type 曲线显示的类型：荧光值或温度
* \return 无
*/
void GRawData::plotChanged(CurveType type)
{
    if(m_d->plot == NULL) return;

    bool isRaw = m_d->curTempIndex == SELECT_BTN_RAW;

    if(type == ct_Temperature){
        m_d->plot->enableAxis(QwtPlot::yLeft, isRaw);
        m_d->plot->enableAxis(QwtPlot::xBottom, isRaw);

        if(isRaw){
            m_d->plot->setAxisTitle(QwtPlot::yLeft, tr("Temperature (%1)").arg(trUtf8("℃")));
            m_d->plot->setAxisTitle(QwtPlot::xBottom, tr("Time (hh:mm)"));

            m_d->plot->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());

            m_d->plot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
            QwtScaleWidget *scaleWidget = m_d->plot->axisWidget(QwtPlot::xBottom);
            const int fmh = QFontMetrics(scaleWidget->font()).height();
            scaleWidget->setMinBorderDist(0, fmh / 2);

            double max_time = m_d->timeCount < 120 ? 120 : m_d->timeCount;
            m_d->plot->setAxisScale(QwtPlot::xBottom,0,max_time);

            m_d->grid->attach(m_d->plot);
        }else{
            m_d->plot->setAxisScale(QwtPlot::xBottom,0,40);

            m_d->plot->setAxisTitle(QwtPlot::yLeft, "");
            m_d->plot->setAxisTitle(QwtPlot::xBottom, "");

            m_d->grid->detach();
        }
        m_d->plot->setAxisScale(QwtPlot::yLeft, 0, 115);
    }else{
        m_d->plot->enableAxis(QwtPlot::yLeft, true);
        m_d->plot->enableAxis(QwtPlot::xBottom, true);

        m_d->grid->attach(m_d->plot);

        m_d->plot->setAxisTitle(QwtPlot::yLeft, tr("RFU"));

        int curveType = -1;  //0扩增;1熔解;
        int curMapIndex = m_d->testType < 3 ? 1 : 2;

        if(m_d->curShowIndex < curMapIndex){
            //显示曲线
            if(curMapIndex == 2){
                curveType = (m_d->curShowIndex == 1) ? 1 : 0;
            }else if(curMapIndex == 1){
                curveType = (m_d->testType & 0x02) ? 1 : 0;
            }
        }

        _PCR_RUN_CTRL_INFO pcrInfo;
        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        m_d->plot->setAxisScale(QwtPlot::yLeft, 0, 400);
        m_d->plot->setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw);

        qDebug() << Q_FUNC_INFO << curveType << m_d->last_amp_stage_no << m_d->la4_stage_no;

        if(curveType == 0){
            //扩增曲线的坐标
            m_d->plot->setAxisTitle(QwtPlot::xBottom, tr("Cycles"));

            double xMin = 1.0, xMax = 40.0;

            if(pcrInfo.State.ExpState == 0){
                //如果是待机时,打开最后一次扩增stage的循环数,如果没有最后一次,打开第1个
                if(m_d->last_amp_stage_no>=0 && m_d->last_amp_stage_no<m_runMethod->count()){
                    xMax = m_runMethod->at(m_d->last_amp_stage_no)->Cycles;
                }else{
                    for(int i=0; i<m_runMethod->count(); i++){
                        if(m_runMethod->at(i) == NULL) continue;
                        if(m_d->sample_step_nos.contains(i) && m_runMethod->at(i)->Property==0){
                            xMax = m_runMethod->at(i)->Cycles;
                            break;
                        }
                    }
                }
            }else{
                //运行中,如果正在跑扩增,显示当前循环数,如果在跑熔解,查找之前第1个扩增的循环数
                bool has_find = false;
                for(int i=pcrInfo.SizeInfo.No.Stage; i>=0; i--){
                    if(m_runMethod->at(i)->Property==0 && m_d->sample_step_nos.contains(i)){
                        has_find = true;
                        xMax = m_runMethod->at(i)->Cycles;
                        break;
                    }
                }

                //如果前面没有，则查找后面的
                if(!has_find){
                    for(int i=pcrInfo.SizeInfo.No.Stage+1; i<m_runMethod->count(); i++){
                        if(m_runMethod->at(i)->Property==0 && m_d->sample_step_nos.contains(i)){
                            xMax = m_runMethod->at(i)->Cycles;
                            break;
                        }
                    }
                }
            }

            qDebug() << Q_FUNC_INFO << "Amp" << xMin << xMax;

            if(xMax<=10 && xMax>=0){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,1);
            }else if(xMax>10 && xMax<=50){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,5);
                m_d->plot->replot();
                QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
                QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
                if(!majorTicks.contains(xMax)&&(int)xMax%5==1){
                    majorTicks.removeLast();
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMax)&&(int)xMax%5>1){
                    majorTicks.append(xMax);
                }

                if(!majorTicks.contains(xMin)&&(int)xMin%5==4){
                    majorTicks.removeFirst();
                    majorTicks.append(xMin);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%5<4){
                    majorTicks.append(xMin);
                }

                scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
                m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
            }else if(xMax>50 && xMax<100){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,10);
                m_d->plot->replot();
                QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
                QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
                if(!majorTicks.contains(xMax)&&(int)xMax%10==1){
                    majorTicks.removeLast();
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMax)&&(int)xMax%10>1){
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%10==9){
                    majorTicks.removeFirst();
                    majorTicks.append(xMin);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%10<9){
                    majorTicks.append(xMin);
                }

                scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
                m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
            }
        }else if(curveType == 1){
            //熔解曲线的坐标
            m_d->plot->setAxisTitle(QwtPlot::xBottom, tr("Temperature (%1)").arg(trUtf8("℃")));

            double xMin = 40.0, xMax = 98.0;

            bool run_range_ok = false;
            if(pcrInfo.State.ExpState != 0){
                int stage_no_ = -1;

                //运行中,如果正在跑扩增,查找前1个熔解
                if(m_runMethod->at(pcrInfo.SizeInfo.No.Stage)->Property == 0){
                    bool has_find = false;
                    for(int i=pcrInfo.SizeInfo.No.Stage-1; i>=0; i--){
                        if(m_runMethod->at(i)->Property != 0){
                            has_find = true;
                            stage_no_ = i;
                            break;
                        }
                    }
                    //如果没有发现，向后找
                    if(!has_find){
                        for(int i=pcrInfo.SizeInfo.No.Stage+1; i<m_runMethod->count(); i++){
                            if(m_runMethod->at(i)->Property != 0){
                                stage_no_ = i;
                                break;
                            }
                        }
                    }
                }else{
                    stage_no_ = pcrInfo.SizeInfo.No.Stage;
                }

                if(m_d->sample_step_nos.contains(stage_no_)){
                    int step_no_ = m_d->sample_step_nos.value(stage_no_);
                    if(step_no_ < m_runMethod->at(stage_no_)->SubNum+1)
                        xMin = m_runMethod->at(stage_no_)->Temp[step_no_-1];
                    if(step_no_ < m_runMethod->at(stage_no_)->SubNum)
                        xMax = m_runMethod->at(stage_no_)->Temp[step_no_];
                    run_range_ok = true;
                }
            }

            if(!run_range_ok){
                if(m_d->la4_stage_no < 0){
                    //显示第1个熔解的坐标
                    for(int i=0; i<m_runMethod->count(); i++){
                        if(m_runMethod->at(i) == NULL) continue;
                        if(m_d->sample_step_nos.contains(i) && m_runMethod->at(i)->Property!=0){
                            int step_no_ = m_d->sample_step_nos.value(i);
                            if(step_no_ < m_runMethod->at(i)->SubNum+1)
                                xMin = m_runMethod->at(i)->Temp[step_no_-1];
                            if(step_no_ < m_runMethod->at(i)->SubNum)
                                xMax = m_runMethod->at(i)->Temp[step_no_];
                            break;
                        }
                    }
                }else{
                    int step_no_ = m_d->sample_step_nos.value(m_d->la4_stage_no);
                    if(m_d->la4_stage_no<m_runMethod->count() && step_no_>0){
                        if(m_d->la4_stage_no<m_runMethod->count() && step_no_>0){
                            if(step_no_ < m_runMethod->at(m_d->la4_stage_no)->SubNum+1)
                                xMin = m_runMethod->at(m_d->la4_stage_no)->Temp[step_no_-1];
                            if(step_no_ < m_runMethod->at(m_d->la4_stage_no)->SubNum)
                                xMax = m_runMethod->at(m_d->la4_stage_no)->Temp[step_no_];
                        }
                    }
                }
            }

            qDebug() << Q_FUNC_INFO << "Melt" << xMin << xMax;

            double deta_val_ = qAbs(xMax-xMin);
            if(deta_val_<=10 && deta_val_>=0){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,1);
            }else if(deta_val_>10 && deta_val_<=50){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,5);
                m_d->plot->replot();
                QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
                QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
                if(!majorTicks.contains(xMax)&&(int)xMax%5==1){
                    majorTicks.removeLast();
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMax)&&(int)xMax%5>1){
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%5==4){
                    majorTicks.removeFirst();
                    majorTicks.append(xMin);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%5<4){
                    majorTicks.append(xMin);
                }
                scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
                m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
            }else if(deta_val_>50 && deta_val_<100){
                m_d->plot->setAxisScale(QwtPlot::xBottom,xMin,xMax,10);
                m_d->plot->replot();
                QwtScaleDiv scaleDiv = m_d->plot->axisScaleDiv(QwtPlot::xBottom);
                QList <double> majorTicks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
                if(!majorTicks.contains(xMax)&&(int)xMax%10==1){
                    majorTicks.removeLast();
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMax)&&(int)xMax%10>1){
                    majorTicks.append(xMax);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%10==9){
                    majorTicks.removeFirst();
                    majorTicks.append(xMin);
                }
                if(!majorTicks.contains(xMin)&&(int)xMin%10<9){
                    majorTicks.append(xMin);
                }

                scaleDiv.setTicks(QwtScaleDiv::MajorTick,majorTicks);
                m_d->plot->setAxisScaleDiv(QwtPlot::xBottom,scaleDiv);
            }
        }else{
            m_d->plot->setAxisTitle(QwtPlot::xBottom, "");
            m_d->plot->setAxisScale(QwtPlot::xBottom,0,40);
        }
    }
    m_d->plot->axisWidget(QwtPlot::yLeft)->setMinimumWidth(16);

    //添加曲线
    foreach(QwtPlotItem *item, m_d->plot->itemList(QwtPlotItem::Rtti_PlotCurve))
        item->detach();

    foreach(QwtPlotItem *item, m_d->plot->itemList(QwtPlotItem::Rtti_PlotUserItem))
        item->detach();

    if(type == ct_Temperature){
        if(isRaw){
            if(m_d->tempCurve)
                m_d->tempCurve->attach(m_d->plot);
            if(m_d->hotCoverCurve)
                m_d->hotCoverCurve->attach(m_d->plot);
        }else{
            if(m_d->tempItem)
                m_d->tempItem->attach(m_d->plot);
        }
    }

    m_d->plot->replot();
}

/*!
* \brief 类GRawData的私有函数， 计算扩增的循环数和熔解的起始终止温度
* \param 无
* \return 无
*/
void GRawData::init_temp_method()
{    
    m_d->sample_step_nos.clear();
    m_d->last_amp_stage_no = -1;
    m_d->la4_stage_no = -1;
    m_pool->infinite_stage_no = -1;
    m_pool->infinite_step_no = -1;

    m_d->currentCycle = 0;
    m_d->testType = 0;

    int amp_stage_cnt = 0;
    int melt_stage_cnt = 0;
}

/*!
* \brief 类GRawData的私有函数，刷新当前Widget显示
* \param mapIndex 热图对应的显示序号
* \return 无
*/
void GRawData::updateCurrentWidget(int mapIndex)
{//2,1,12,只设置的FAM，只显示FAM
    qDebug() << Q_FUNC_INFO << mapIndex << m_d->curShowIndex << m_d->curFluorIndex;
    //更新显示
    int curStackedWidgetIndex = m_d->curShowIndex < mapIndex ? 0 : 1;

    if(ui->stackedWidget->currentIndex() != curStackedWidgetIndex){
        ui->stackedWidget->setCurrentIndex(curStackedWidgetIndex);
    }

    if(m_d->curShowIndex < mapIndex){
        plotChanged(ct_Fluor);
    }
    displayFluorOrHeatMap(m_d->curFluorIndex);
    //刷新当前窗口的描述
    QString currentTitle;
    if(mapIndex == 2){
        switch(m_d->curShowIndex){
        case 1:currentTitle = tr("Melting");break;
        case 2:currentTitle = tr("Heat Map");break;
        default:currentTitle = tr("Amplification");break;
        }
    }else if(mapIndex == 1){
        switch(m_d->curShowIndex){
        case 1:currentTitle = tr("Heat Map");break;
        default:currentTitle = (m_d->testType & 0x02) ? tr("Melting") : tr("Amplification");break;
        }
    }
    ui->labelCurrent->setText(currentTitle);
}

/*!
* \brief 类GRawData的私有槽函数， 实现pageChannel页通道按键的功能
* \param 无
* \return 无
*/
void GRawData::slot_fluorBtn_clicked()
{    
    m_pool->screen_sound();
    //得到按键的序号
    QToolButton *source = qobject_cast<QToolButton *>(sender());
    int index = m_d->fluorBtns.indexOf(source);

    qDebug() << Q_FUNC_INFO << index;

    //把该按键设为选中状态
    changeFluorButtonStatus(index);

    //得到选择按键在显示的按键中的序号和显示按键的总数
    int btnShowCount = 0;
    int curShowIndex = -1;
    for(int i=0; i<m_d->fluorBtns.count(); i++){
        if(index  == i){
            curShowIndex = btnShowCount;
        }
        if(m_d->fluorBtns.at(i)->property("showbutton").toBool()) btnShowCount++;
    }

    //判断选择按键的位置,把按键移动到中间
    int max = m_d->fluorArea->verticalScrollBar()->maximum();
    int val = m_d->fluorArea->verticalScrollBar()->value();

    qDebug() << Q_FUNC_INFO << index  << m_d->fluorBtns.count() << btnShowCount << curShowIndex << max << val;

    if(max > 0){
        if(curShowIndex <= FLUOR_BTN_CENTER_INDEX){
            if(val != 0) m_d->fluorArea->verticalScrollBar()->setValue(0);
        }else if(curShowIndex < btnShowCount-FLUOR_BTN_CENTER_INDEX-1){
            m_d->fluorArea->verticalScrollBar()->setValue((curShowIndex-FLUOR_BTN_CENTER_INDEX)*FLUOR_BTN_HEIGHT);
        }else{
            if(val != max) m_d->fluorArea->verticalScrollBar()->setValue(max);
        }
    }

    displayFluorOrHeatMap(index );
}

/*!
* \brief 类GRawData的私有槽函数， 实现pageTemp页选择按键的功能
* \param 无
* \return 无
*/
void GRawData::slot_tempBtn_clicked()
{
    m_pool->screen_sound();

    QToolButton *source = qobject_cast<QToolButton *>(sender());
    int index = m_d->tempBtns.indexOf(source);

    changeTempButtonStatus(index);

    plotChanged(ct_Temperature);

    if(m_d->curTempIndex == SELECT_BTN_TEMP){
        m_d->tempItem->locateFresh();
        m_d->plot->replot();
    }else{
        displayTempCurve(m_d->timeCount);
    }
}

/*!
* \brief 类GRawData的私有槽函数， 执行QToolBox控件当前页变化时的操作
* \param 无
* \return 无
*/
void GRawData::slot_toolBox_currentChanged(int index)
{
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    ui->btnWidget->setVisible(index == 0);
    if(index == 0){
        int curMapIndex = m_d->testType < 3 ? 1 : 2;
        updateCurrentWidget(curMapIndex);
    }else{
        if(ui->stackedWidget->currentIndex() != 0)
            ui->stackedWidget->setCurrentIndex(0);
        plotChanged(ct_Temperature);

        if(m_d->curTempIndex == SELECT_BTN_TEMP){
            m_d->tempItem->locateFresh();
            m_d->plot->replot();
        }else{
            displayTempCurve(m_d->timeCount);
        }
    }
}

/*!
* \brief 类GRawData的私有槽函数， 实现荧光素按键下扩增,熔解和热图的向前转换
* \param 无
* \return 无
*/
void GRawData::on_buttonPrev_clicked()
{
    m_pool->screen_sound();

    //更新变量设置
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    //取消自动转换界面功能
    if(pcrInfo.State.ExpState==1 || pcrInfo.State.ExpState==3){
        this->setProperty("autoShow", false);
    }

    //显示序号
    int curMapIndex = m_d->testType < 3 ? 1 : 2;
    m_d->curShowIndex > 0 ? (m_d->curShowIndex--) : (m_d->curShowIndex = curMapIndex);

    //显示当前显示序号下的界面
    updateCurrentWidget(curMapIndex);
}

/*!
* \brief 类GRawData的私有槽函数， 实现荧光素按键下扩增,熔解和热图的向后转换
* \param 无
* \return 无
*/
void GRawData::on_buttonNext_clicked()
{   
    m_pool->screen_sound();

    //更新变量设置
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    //取消自动转换界面功能
    if(pcrInfo.State.ExpState==1 || pcrInfo.State.ExpState==3){
        this->setProperty("autoShow", false);
    }

    //显示序号
    int curMapIndex = m_d->testType < 3 ? 1 : 2;
    m_d->curShowIndex < curMapIndex ? (m_d->curShowIndex++) : (m_d->curShowIndex = 0);

    //显示当前显示序号下的界面
    updateCurrentWidget(curMapIndex);
}

///////////////////////////////////////////////////////////////////////////////
/// 滤波算法
///////////////////////////////////////////////////////////////////////////////
double midPoint(double *inputdata,int size)
{
    double *tmpdata = new double[size];
    memcpy(tmpdata,inputdata,sizeof(double)*size);
    for(int i =0;i<size/2+1;i++)
    {
        int indx=i;
        for(int j =i+1;j<size;j++)
        {
            if(inputdata[indx] >inputdata[j])
                indx=j;
        }
        if(indx!=i)
        {
            double tmpdata1=tmpdata[i];
            tmpdata[i]=tmpdata[indx];
            tmpdata[indx]=tmpdata1;
        }
    }
    double midval =0;
    if(0==size%2)
    {
        midval =(tmpdata[size/2-1]+tmpdata[size/2])/2;
    }
    else
    {
        midval = tmpdata[size/2];
    }
    delete tmpdata;
    return midval;
}

void shiftY(double *inputY,int size,double shiftval)
{
    for(int i =0;i<size;i++)
    {
        inputY[i]-=shiftval;
    }
}
double min(double *inputY,int size)
{
    double minval =0;
    if(size<1) return minval;
    else
    {
        minval = inputY[0];
        for(int i =1;i<size;i++)
            if(minval>inputY[i])
                minval = inputY[i];
        return minval;
    }
}

//熔解单曲线实时滤波
void filtSingleCurve_sixChannel(double *inputY,int insize,double *outputY,int &outSize)
{
    for(int i = outSize;i<insize;i++)
    {
        outputY[i] = inputY[i];
        outSize++;
        if(i>=4)
        {
            double data[5];
            memcpy(data,inputY+i-4,sizeof(double)*5);
            for(int j =1;j<4;j++)
            {
                if((data[j]>data[j-1] && data[j]>data[j+1])
                        ||(data[j]<data[j-1] && data[j]<data[j+1]))
                {
                    data[j]=(data[j-1]+data[j+1])/2;
                    j++;
                }
            }
            data[2]=midPoint(data,5);
            data[2]=(data[0]+2*data[1]+4*data[2]+4*data[3]+data[4])/12.0;
            memcpy(outputY+i-2,data+2,sizeof(double)*3);
        }
    }
}
//扩增单曲线实时滤波
void  filtAmpSingleCurve_sixChannel(double *inputY,int insize,double *outputY,int &outSize)
{
    int diffval =0;
    if(outSize>0)
        diffval = inputY[outSize-1]-outputY[outSize-1];
    for(int i = outSize;i<insize;i++)
    {
        //        outputY[i] = inputY[i];
        outSize++;
        if(i>=4)
        {
            double data[5];
            memcpy(data,inputY+i-4,sizeof(double)*5);
            for(int j =1;j<4;j++)
            {
                if((data[j]>data[j-1] && data[j]>data[j+1])
                        ||(data[j]<data[j-1] && data[j]<data[j+1]))
                {
                    data[j]=(data[j-1]+data[j+1])/2;
                    j++;
                }
            }
            data[2]=midPoint(data,5);
            data[2]=(data[0]+2*data[1]+4*data[2]+4*data[3]+data[4])/12.0;
            //            memcpy(outputY+i-2,data+2,sizeof(double)*3);
            for(int j =i-2;j<=i;j++)
                outputY[j]=data[4-(i-j)]-diffval;
        }else
        {
            outputY[i] = inputY[i]-diffval;
        }
    }
    double minval = min(outputY,outSize);

    shiftY(outputY,outSize,minval);
}
