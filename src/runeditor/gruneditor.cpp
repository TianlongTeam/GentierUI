/*!
* \file gruneditor.cpp
* \brief ARM板软件中运行设置界面cpp文件
*
*实现了ARM板软件实验控制的设置功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:36
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gruneditor.h"
//#include "ui_gruneditor.h"
//#include "myinputpanelcontext.h"
#if defined(DEVICE_TYPE_TL22)
#include "ui_gruneditor.h"
#elif defined(DEVICE_TYPE_TL23)
#include "ui_gruneditor-23.h"
#elif defined(DEVICE_TYPE_TL13)
#include "ui_gruneditor-13.h"
#elif (defined DEVICE_TYPE_TL12)
#include "ui_gruneditor-12.h"
#endif
#include "gcircularwidget.h"
#include "gglobal.h"
#include "gchannelsetdelegate.h"
#include "ggradientdetail.h"

#include "gdatapool.h"

#include "geditdeletgate.h"
#include "gstageadddelegate.h"
#include "grunmethoditem.h"

//#include "myinputpanelcontext.h"
#include "gdatainputdialog.h"
#include "gtimeinputdialog.h"
#include "mymessagebox.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_plot_panner.h>

#include <QStandardItemModel>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QThread>
//-----------------------------------------------------------------------------
//define declare
//-----------------------------------------------------------------------------


#if defined(DEVICE_TYPE_TL22)
#define SHOW_CHANNEL_HEIGHT_PIXEL  334
#define SHOW_CHANNEL_WIDTH_PIXEL   764
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#define SHOW_CHANNEL_HEIGHT_PIXEL  200
#define SHOW_CHANNEL_WIDTH_PIXEL  650
#endif

#define CHANNEL_COL0_WIDTH   120

#if defined(DEVICE_TYPE_TL22)
#define CHANNEL_COL1_WIDTH   460
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#define CHANNEL_COL1_WIDTH  280
#endif

#define CHANNEL_COL2_WIDTH   (SHOW_CHANNEL_WIDTH_PIXEL-CHANNEL_COL0_WIDTH-CHANNEL_COL1_WIDTH)
#define CHANNEL_HEADER_HEIGHT    34
//#define CHANNEL_HEIGHT       (SHOW_CHANNEL_HEIGHT_PIXEL/MAX_FLUOR_CH)

#define HORIZONTAL_HEADER_HEIGHT    27
#define VSCROLLBAR_WIDTH    40

#if defined(DEVICE_TYPE_TL22)
#define STAGE_STEP_HEIGHT   140
#define STAGE_STEP_ITEM_HEIGHT  (STAGE_STEP_HEIGHT/4)
#define STAGE_COL0_WIDTH    183+VSCROLLBAR_WIDTH
#define STAGE_COL1_WIDTH    83
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#define STAGE_STEP_HEIGHT  185
#define STAGE_STEP_ITEM_HEIGHT  (STAGE_STEP_HEIGHT/4)
#define STAGE_COL0_WIDTH    155+VSCROLLBAR_WIDTH
#define STAGE_COL1_WIDTH   58//60
#elif defined(DEVICE_TYPE_TL13)
#define STAGE_STEP_HEIGHT  180.4
#define STAGE_STEP_ITEM_HEIGHT  (STAGE_STEP_HEIGHT/4.1)
//#define STAGE_COL0_WIDTH    125+VSCROLLBAR_WIDTH
#define STAGE_COL0_WIDTH   145+VSCROLLBAR_WIDTH
#define STAGE_COL1_WIDTH   98
#endif

#if defined(DEVICE_TYPE_TL22)
#define STEP_COL0_WIDTH     115+(VSCROLLBAR_WIDTH/2)
#define STEP_COL2_WIDTH     62  //120
#define STEP_COL3_WIDTH     62 //120
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
#define STEP_COL0_WIDTH     96+(VSCROLLBAR_WIDTH/2)
#define STEP_COL2_WIDTH   59//120
#define STEP_COL3_WIDTH   59  //120
#elif defined(DEVICE_TYPE_TL13)
#define STEP_COL0_WIDTH   95+(VSCROLLBAR_WIDTH/2)
#define STEP_COL2_WIDTH  40  //120
#define STEP_COL3_WIDTH 40//120
#endif

//#if defined(DEVICE_TYPE_TL22)
//#define STAGEADD_COLUMN_COUNT   3
//#define STAGEADD_ITEM_WIDTH     250 //344
//#define STAGEADD_ITEM_HEIGHT    42  //45
//#elif defined(DEVICE_TYPE_TL23)
//#define STAGEADD_COLUMN_COUNT  2
//#define STAGEADD_ITEM_WIDTH    344
//#define STAGEADD_ITEM_HEIGHT   47
//#elif defined(DEVICE_TYPE_TL13)
//#define STAGEADD_COLUMN_COUNT   2
//#define STAGEADD_ITEM_WIDTH  260 //344
//#define STAGEADD_ITEM_HEIGHT 52//45
//#endif


#ifndef DEVICE_TYPE_TL13
#define STAGEADD_COLUMN_COUNT  2
#define STAGEADD_ITEM_WIDTH    344
#define STAGEADD_ITEM_HEIGHT   47
#else
#define STAGEADD_COLUMN_COUNT   2
#define STAGEADD_ITEM_WIDTH  260 //344
#define STAGEADD_ITEM_HEIGHT 52//45
#endif

#define FRET_INDEX  5
//-----------------------------------------------------------------------------
//private data class declare
//-----------------------------------------------------------------------------
/*!
* \class PrivateData
* \brief 类GRunEditor内部的私有数据类
*
* 用于统一管理私有数据
*/
class GRunEditor::PrivateData
{
public:
    explicit PrivateData() : \
        channelModel(NULL),
        channelDelegate(NULL),
        stageModel(NULL),
        stepModel(NULL),
        stageAddModel(NULL),
        stageAddDelegate(NULL),
        stepEditDelegate(NULL),
        stepCheckDelegate(NULL),
        plot(NULL),
        plotStageAdd(NULL),
        item(NULL),
        itemStageAdd(NULL),
        selectedStageRow(-1),
        selectedStepRow(-1),
        specialIndex(-1),
        isSingleStep(false)
    {
    }

    ~PrivateData(){

        if(channelModel) delete channelModel;
        if(channelDelegate) delete channelDelegate;
        if(stageModel) delete stageModel;
        if(stepModel) delete stepModel;
        if(stageAddModel) delete stageAddModel;
        if(stageAddDelegate) delete stageAddDelegate;
        if(stepEditDelegate) delete stepEditDelegate;
        if(stepCheckDelegate) delete stepCheckDelegate;

        if(item){
            item->detach();
            delete item;
        }
        if(plot) delete plot;

        if(itemStageAdd){
            itemStageAdd->detach();
            delete itemStageAdd;
        }
        if(plotStageAdd) delete plotStageAdd;
    }

    QStandardItemModel *channelModel;       ///< 通道设置模板
    GChannelSetDelegate *channelDelegate;   ///< 通道设置显示代理
    QStandardItemModel *stageModel;         ///< 过程设置模板
    QStandardItemModel *stepModel;          ///< 步骤设置模板
    QStandardItemModel *stageAddModel;      ///< 过程添加模板
    GStageAddDelegate  *stageAddDelegate;   ///< 过程添加显示代理
    GEditIndicatorDeletgate *stepEditDelegate;      ///< 步骤编辑代理
    GCheckIndicatorDeletgate *stepCheckDelegate;    ///< 步骤是否采样的代理

    QwtPlot *plot,*plotStageAdd;        ///< 温控绘图
    GRunMethodItem *item,*itemStageAdd; ///< 温控子图

    int selectedStageRow;               ///< 温控设置时，选择的过程的行数
    int selectedStepRow;                ///< 温控设置时，选择的步骤的行数
    int specialIndex;                   ///< steps中Touchdown，Long,梯度或熔解的序号

    bool isSingleStep;                  ///< 当前设置为阶跃熔解
};
//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------
/*!
* \class GRunEditor
* \brief ARM板运行设置界面类
*
* 实验温控设置功能
*/

/*!
* \brief 类GRunEditor的构造函数
* \param parent = NULL
* \return 无
*/
GRunEditor::GRunEditor(GDataPool *dataPool, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GRunEditor),
    m_d(new GRunEditor::PrivateData),
    m_pool(dataPool)
{
    ui->setupUi(this);

    m_runInfo = &dataPool->expFile->m_expRunInfo;
    m_runMethod = &dataPool->expFile->m_expRunMethod;

    initVariables();
    initUi();

    this->clearFocus();
}

/*!
* \brief 类GRunEditor的析构函数
* \param 无
* \return 无
*/
GRunEditor::~GRunEditor()
{
    m_runInfo = NULL;
    m_runMethod = NULL;

    //    if(m_input) delete m_input;
    delete m_d;
    delete ui;
    qDebug() << "delete GRunEditor widget";
}

/*!
* \brief 类GRunEditor的公共函数，显示第一个Tab界面
* \param 无
* \return 无
*/
void GRunEditor::showFirstTab()
{
    if(ui->RunEditor->currentIndex() != 0){
        ui->RunEditor->setCurrentIndex(0);
    }
}

/*!
* \brief 类GRunEditor的公共槽函数，实现运行信息和方法的更新
* \param 无
* \return 无
*/
void GRunEditor::updateConfig()
{
    qDebug() << Q_FUNC_INFO;

    //运行信息
    if(m_runInfo){
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        QStringList titles, simplifys;
        titles << tr("Clear") << tr("White") << tr("Frosted");
        simplifys << "C" << "W" << "F";
        ui->labelTubeType->setText(titles.at(0));
        ui->labelTubeType->setProperty("simplify", simplifys.at(0));
#endif

        ui->labelReaction->setText(QString::number(m_runInfo->reactionValume)+" μL");
        ui->labelHotLid->setText(QString::number(m_runInfo->lidValue)+" ℃");
        ui->labelHotLid->setStyleSheet(m_runInfo->lidValue<100?"background-color:yellow;":"");
        ui->checkBoxHotCover->blockSignals(true);
        ui->checkBoxHotCover->setChecked((m_runInfo->lidStatus<0?true:m_runInfo->lidStatus));
        ui->checkBoxHotCover->blockSignals(false);
        ui->labelHotLid->setEnabled(ui->checkBoxHotCover->isChecked());
    }
    qDebug() << "updateConfig ---"<<m_runMethod->count()<<m_runMethod->size();

    //运行方法
    if(m_runMethod){
        ui->tableViewStage->selectionModel()->clear();
        for(int i=0; i<m_runMethod->count(); i++){
            if(m_runMethod->at(i) == NULL) continue;

            m_d->stageModel->appendRow(new QStandardItem);
            m_d->stageModel->setData(m_d->stageModel->index(i,0), QString::fromUtf8(m_runMethod->at(i)->Name), Qt::DisplayRole);
            m_d->stageModel->setData(m_d->stageModel->index(i,1), m_runMethod->at(i)->Cycles, Qt::DisplayRole);

            int type;
#ifndef DEVICE_TYPE_TL13
            if(m_runMethod->at(i)->Property == 1){
                type = 4;
                m_d->isSingleStep = true;
            }else if(m_runMethod->at(i)->Property == 2){
                type = 5;
                m_d->isSingleStep = false;     ///add=705
            }else{
#endif
                int stepCount = m_runMethod->at(i)->SubNum;
                if(stepCount == 1){
                    type = (m_runMethod->at(i)->Temp[0]) > PREINCUBATION_AND_COOL ? 0 : 6;
                }else if(stepCount == 2){
                    type = 2;
                }else if(stepCount == 3){
                    type = 3;
                }else{
                    type = 7;
                }
#ifndef DEVICE_TYPE_TL13
            }
#endif
            m_d->stageModel->setData(m_d->stageModel->index(i,2), type, Qt::EditRole);
        }
        ui->tableViewStage->viewport()->update();

        if(m_d->stageModel->rowCount() > 0)
            ui->tableViewStage->clearSelection();
        ui->tableViewStage->selectRow(0);
    }

    if(m_d->item){
        m_d->item->currentSelect(0,0);
        m_d->plot->setAxisScale(QwtPlot::xBottom,0,40);
        m_d->plot->replot();
    }
    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(0,0);
        m_d->plotStageAdd->setAxisScale(QwtPlot::xBottom,0,100);
        m_d->plotStageAdd->replot();
    }

    _PCR_RUN_CTRL_INFO pcrInfo;

    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState==0){
        m_d->stepEditDelegate->setEnable(m_pool->expFile->is_compressed_file == 0);
        m_d->stepCheckDelegate->setEnable(m_pool->expFile->is_compressed_file == 0);
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        ui->buttonTubeTypeNext->setEnabled(m_pool->expFile->is_compressed_file == 0);
#endif
        ui->buttonReactionAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonReactionReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->checkBoxHotCover->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonHotlidAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonHotlidReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);


        ui->buttonStageAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        bool hasStage = m_d->stageModel->rowCount() > 0;
        ui->buttonStageUp->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        ui->buttonStageReduce->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        ui->buttonStageDown->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);

        if(GHelper::total_instrument_id != 6){
            ui->buttonStepUp->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepAdd->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepReduce->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepDown->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        }
    }
    emit methodChanged();

    qDebug() << Q_FUNC_INFO << "step5";

#ifndef DEVICE_TYPE_TL13
    //    slot_currentChanged(0);
    slot_tableViewStage_selectionChanged();
    slot_tableViewStep_selectionChanged();
#endif    

    //为了防止stage和step在删除和下移时同时触发导致数据错误
    this->setProperty("Stage_Status", false);
    this->setProperty("Step_Status", false);

    qDebug() << Q_FUNC_INFO << "end";
}

/*!
* \brief 类GRunEditor的公共槽函数，实验运行和结束时界面的控制
* \param state 实验状态 0停止,1运行,2暂停
* \return 无
*/
void GRunEditor::experimentState(int state)
{
    qDebug() << Q_FUNC_INFO << state;
    switch(state){
    case 0:{
#ifndef DEVICE_TYPE_TL13
        if(!ui->RunEditor->widget(1)->isEnabled())
            ui->RunEditor->widget(1)->setEnabled(true);
#endif
        m_d->stepEditDelegate->setEnable(m_pool->expFile->is_compressed_file == 0);
        m_d->stepCheckDelegate->setEnable(m_pool->expFile->is_compressed_file == 0);
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        ui->buttonTubeTypeNext->setEnabled(m_pool->expFile->is_compressed_file == 0);
#endif
        ui->buttonReactionAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonReactionReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->checkBoxHotCover->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonHotlidAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        ui->buttonHotlidReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);

        ui->buttonStageAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
        bool hasStage = m_d->stageModel->rowCount() > 0;
        ui->buttonStageUp->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        ui->buttonStageReduce->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        ui->buttonStageDown->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);

        if(GHelper::total_instrument_id != 6){
            ui->buttonStepUp->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepAdd->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepReduce->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
            ui->buttonStepDown->setEnabled(m_pool->expFile->is_compressed_file==0 && hasStage);
        }

        //        slot_currentChanged(0);
        slot_tableViewStage_selectionChanged();
        slot_tableViewStep_selectionChanged();
        break;}
    case 1:{
#ifndef DEVICE_TYPE_TL13
        if(ui->RunEditor->widget(1)->isEnabled())
            ui->RunEditor->widget(1)->setEnabled(false);
#endif
        m_d->stepEditDelegate->setEnable(false);
        m_d->stepCheckDelegate->setEnable(false);
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
        ui->buttonTubeTypeNext->setEnabled(false);
#endif
        ui->buttonReactionAdd->setEnabled(false);
        ui->buttonReactionReduce->setEnabled(false);
        ui->checkBoxHotCover->setEnabled(false);
        ui->buttonHotlidAdd->setEnabled(false);
        ui->buttonHotlidReduce->setEnabled(false);

        ui->buttonStageUp->setEnabled(false);
        ui->buttonStageAdd->setEnabled(false);
        ui->buttonStageReduce->setEnabled(false);
        ui->buttonStageDown->setEnabled(false);

        if(GHelper::total_instrument_id != 6){
            ui->buttonStepUp->setEnabled(false);
            ui->buttonStepAdd->setEnabled(false);
            ui->buttonStepReduce->setEnabled(false);
            ui->buttonStepDown->setEnabled(false);
        }
        break;}
    default:break;
    }
    qDebug() << Q_FUNC_INFO << "end";
}

/*!
* \brief 类GRunEditor的继承事件，用于动态切换语言
* \param 无
* \return 无
*/
void GRunEditor::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);//在此处刷新语言的

        if(m_d->channelModel){
            QStringList header;
            header << tr("Channel") << tr("Dye") << tr("Excitation/Emission");
            m_d->channelModel->setHorizontalHeaderLabels(header);
        }

        if(m_d->stageModel){
            QStringList header;
            header << tr("Stage") << tr("Cycle") << tr("Type");
            m_d->stageModel->setHorizontalHeaderLabels(header);
        }

        if(m_d->stepModel){
            QStringList header;

#ifdef DEVICE_TYPE_TL13
            header << tr("Temperature") << tr("Time") << tr("Edit") << tr("Type");
#else
            header << tr("Temperature") << tr("Time") << tr("Fluor") << tr("Edit")<< tr("Type");
#endif

            m_d->stepModel->setHorizontalHeaderLabels(header);
        }

        if(m_d->stageAddModel){
            QStringList describes;

#ifdef DEVICE_TYPE_TL13
            describes << tr("Preincubation") \
                      << tr("Reverse Transcription") \
                      << tr("2 Step Amplification") \
                      << tr("3 Step Amplification") \
                      << tr("Cooling") \
                      << tr("Custom Stage");
#else
            describes << tr("Preincubation") \
                      << tr("Reverse Transcription") \
                      << tr("2 Step Amplification") \
                      << tr("3 Step Amplification") \
                      << tr("Melting") \
                      << tr("Continuous Melting") \
                      << tr("Cooling") \
                      << tr("Custom Stage");
#endif

            for(int i=0; i<describes.count(); i++){
                int row = i / STAGEADD_COLUMN_COUNT;
                int col = i % STAGEADD_COLUMN_COUNT;
                m_d->stageAddModel->setItem(row,col, new QStandardItem(describes.at(i)));
            }
        }
        int val = m_runInfo->lidValue/100;
        ui->labelReactionTitle->setText(tr("Reaction Volume:"));
        ui->labelReaction->setText(QString::number(m_runInfo->reactionValume)+" μL");
        ui->checkBoxHotCover->setText(tr("Hot Lid:"));
        ui->labelHotLid->setText(QString::number(val)+" ℃");
        ui->labelRamp->setText(tr("Ramp:"));
        ui->labelCenter->setText(tr("Temp. Center:"));
        ui->labelOffset->setText(tr("Temp. Offset:"));
        ui->buttonStepGradientDetail->setText(tr("Details"));

        if(ui->radioButtonStepTouchdown->isChecked()){
            ui->labelTarget->setText(tr("Initial Temp.:"));
            ui->labelDuration->setText(tr("Time:"));
            ui->labelTargetValue->setText(tr("Target Temp.:"));
            //            ui->labelTarValueUnit->setText(trUtf8("℃"));
            ui->labelTarValueUnit->setVisible(true);
            ui->labelDelta->setText(tr("Delta Temp.:"));
            ui->labelDeltaUnit->setText(trUtf8("℃/Cycle"));
        }else if(ui->radioButtonStepLong->isChecked()){
            ui->labelTarget->setText(tr("Temperature:"));
            ui->labelDuration->setText(tr("Initial Time:"));
            ui->labelTargetValue->setText(tr("Target Time:"));
            //            ui->labelTarValueUnit->setText(trUtf8("s"));
            ui->labelTarValueUnit->setVisible(false);
            ui->labelDelta->setText(tr("Delta Time:"));
            ui->labelDeltaUnit->setText(tr("/Cycle"));
        }else{
            ui->labelTarget->setText(tr("Temperature:"));
            ui->labelDuration->setText(tr("Time:"));

            ui->labelTargetValue->setText(tr("Target Value:"));
        }

        //       ui->labelDelta->setText(tr("Increment:"));
        ui->labelBeginCycle->setText(tr("Start Cycle:"));

        ui->labelMode->setText(tr("Step Mode:"));
        ui->radioButtonStepStd->setText(tr("Standard"));
        ui->radioButtonStepGradient->setText(tr("Gradient"));
        ui->radioButtonStepTouchdown->setText(tr("Touchdown"));
        ui->radioButtonStepLong->setText(tr("Long"));
        ui->buttonStepEditAmpBack->setText(tr("Back"));

        ui->labelMeltTarget->setText(tr("Temperature:"));
        ui->labelMeltDuration->setText(tr("Time:"));
        ui->labelMeltRamp->setText(tr("Ramp:"));
        ui->labelMeltStep->setText(m_d->isSingleStep?tr("Increment:"):tr("Readings:"));
        ui->labelMeltUnit->setText(m_d->isSingleStep?trUtf8("℃"):tr("Readings/%1").arg(trUtf8("℃")));
    }
    QWidget::changeEvent(e);
}

/*!
* \brief 类GRunEditor的继承事件，用于显示数据输入框
* \param obj 发生事件的对象
* \param event 对象上的事件
* \return 无
*/
bool GRunEditor::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->labelReaction){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            _PCR_RUN_CTRL_INFO pcrInfo;
            m_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
            m_pool->mutex.unlock();

            //如果正在实验,退出
            if(pcrInfo.State.ExpState != 0){
                return true;
            }

            if(m_pool->expFile->is_compressed_file != 0) return true;

            emit editting2(true);

            QRegExp rx("(\\d+)");
            QString str = ui->labelReaction->text();
            str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;
            GDataInputDialog dialog(tr("Reaction Volume [%1]").arg(tr("μL")), str, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(MIN_REACTION_VOL, MAX_REACTION_VOL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < MIN_REACTION_VOL){
                    val = MIN_REACTION_VOL;
                }else if(val > MAX_REACTION_VOL){
                    val = MAX_REACTION_VOL;
                }
                ui->labelReaction->setText(QString::number(val)+" μL");
                ui->labelReaction->clearFocus();

                if(m_runInfo){
                    m_runInfo->reactionValume = val;
                }
                qDebug() << "save method file 1";
                saveMethodFile();

                if(val == 0){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Block control mode"));
                }
            }

            emit editting2(false);
            return true;
        }
    }else if(obj == ui->labelHotLid){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            _PCR_RUN_CTRL_INFO pcrInfo;
            m_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
            m_pool->mutex.unlock();

            //如果正在实验,退出
            if(pcrInfo.State.ExpState != 0){
                return true;
            }

            if(m_pool->expFile->is_compressed_file != 0) return true;

            emit editting2(true);

            QRegExp rx("(\\d+)");
            QString str = ui->labelHotLid->text();
            str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;
            GDataInputDialog dialog(tr("Hot Lid [%1]").arg(trUtf8("℃")), str, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(MIN_HOTCOVER_VOL, MAX_HOTCOVER_VOL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < MIN_HOTCOVER_VOL){
                    val = MIN_HOTCOVER_VOL;
                }else if(val > MAX_HOTCOVER_VOL){
                    val = MAX_HOTCOVER_VOL;
                }

                ui->labelHotLid->setText(QString::number(val)+" ℃");
                ui->labelHotLid->setStyleSheet(val<100?"background-color:yellow;":"");
                ui->labelHotLid->clearFocus();
                m_runInfo->lidValue = val;

                qDebug() << "save method file 2";
                saveMethodFile();

                if(val < 100){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Hot Lid temperature lower than 100 %1").arg(trUtf8("℃")));
                }
            }
            emit editting2(false);
            return true;
        }
    }else if(obj == ui->lineEditStepTarget){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            QString title;
            double minTemp = MIN_STD_TEMP;
            if(ui->radioButtonStepTouchdown->isChecked()){
                title = tr("Initial Temp. [%1]").arg(trUtf8("℃"));
                minTemp = MIN_TOUCHDOWN_TEMP;
            }else{
                title = tr("Temperature [%1]").arg(trUtf8("℃"));
            }
            GDataInputDialog dialog(title, ui->lineEditStepTarget->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setDoubleRange(minTemp, MAX_STD_TEMP, TEMP_DECIMAL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double val = dialog.value().toDouble();
                if(val < minTemp){
                    val = minTemp;
                }else if(val > MAX_STD_TEMP){
                    val = MAX_STD_TEMP;
                }

                ui->lineEditStepTarget->setText(QString::number(val,'f',1));
                ui->lineEditStepTarget->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->selectedStepRow] = val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditStepDuration){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            QString title;
            if(ui->radioButtonStepLong->isChecked()){
                title = tr("Initial Time");
            }else{
                title = tr("Time");
            }

            int minutes = 0, seconds = 0;
            QRegExp rx("(\\d*):(\\d*)");
            int dPos = rx.indexIn(ui->lineEditStepDuration->text());
            if (dPos > -1) {
                minutes = rx.cap(1).toInt();
                seconds = rx.cap(2).toInt();
            }

            bool canInfinity = (m_runMethod && (m_d->selectedStageRow == m_runMethod->count()-1) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                                && (m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 0) \
                                && (m_runMethod->at(m_d->selectedStageRow)->Cycles == 1) \
                                && (m_d->selectedStepRow == m_runMethod->at(m_d->selectedStageRow)->SubNum-1));

            GTimeInputDialog dialog(title, minutes, seconds, canInfinity, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(1, MAX_DURATION);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 0){
                    ui->lineEditStepDuration->setText("∞");
                }else{
                    if(val < 1 )
                        val = 1;
                    else if(val > MAX_DURATION)
                        val = MAX_DURATION;
                    minutes = val / 60;
                    seconds = val % 60;

                    ui->lineEditStepDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
                }
                ui->lineEditStepDuration->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Time[m_d->selectedStepRow] = (val<0?0:val);
                    if(val<0){
                        m_runMethod->at(m_d->selectedStageRow)->ReadFluor[m_d->selectedStepRow] = 0;
                    }
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditStepRamp){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            GDataInputDialog dialog(tr("Ramp [%1]").arg(trUtf8("℃/s")), ui->lineEditStepRamp->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));

            int z = 0;
            dialog.setDoubleRange(MIN_RAMP,m_pool->maxSpeed[z],1);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double val = dialog.value().toDouble();
                if(val < MIN_RAMP){
                    val = MIN_RAMP;
                }else if(val > m_pool->maxSpeed[z]){
                    val = m_pool->maxSpeed[z];
                }
                ui->lineEditStepRamp->setText(QString::number(val,'f',1));
                ui->lineEditStepRamp->clearFocus();
                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Ramp[m_d->selectedStepRow] =val;
                }
            }
            return true;
        }
    }else if(obj == ui->tableViewStage->viewport()){
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e && e->type() == QEvent::MouseButtonRelease){
            QModelIndex index = ui->tableViewStage->indexAt(e->pos());
            if(!index.isValid()) return true;

            //判断是否移动
            int min = ui->tableViewStage->verticalScrollBar()->minimum();
            int single = ui->tableViewStage->verticalScrollBar()->singleStep();
            int current = ui->tableViewStage->verticalScrollBar()->value();
            int topRow = (current - min) / single;

            if(index.row() == topRow){
                ui->tableViewStage->verticalScrollBar()->setValue(current-single);
            }
#if defined(DEVICE_TYPE_TL22)
            else if(index.row() == topRow+3){
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
            else if(index.row() == topRow+4){
#elif(DEVICE_TYPE_TL13)
            else if(index.row() == topRow+5){
#endif
                ui->tableViewStage->verticalScrollBar()->setValue(current+single);
            }

            _PCR_RUN_CTRL_INFO pcrInfo;
            m_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
            m_pool->mutex.unlock();

            //如果正在实验,退出
            if(pcrInfo.State.ExpState != 0){
                return true;
            }

            m_d->selectedStageRow = index.row();

            if(index.column()==1 && index.row()<m_d->stageModel->rowCount()){
                if(m_pool->expFile->is_compressed_file != 0) return true;
                m_pool->screen_sound();

                if(m_d->stageModel->rowCount() != m_runMethod->count()) return true;
                if(!ui->tableViewStage->selectionModel()->hasSelection()) return true;

                //如果是熔解或高分辨熔解时不能修改循环数
                int stageType = m_d->stageModel->data(m_d->stageModel->index(index.row(),2),Qt::EditRole).toInt();
#ifndef DEVICE_TYPE_TL13
                if(stageType == 4 || stageType == 5) return true;
#endif
                emit editting2(true);

                GDataInputDialog dialog(tr("Cycle"), m_d->stageModel->data(index,Qt::DisplayRole).toString().trimmed(), this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setIntRange(1,MAX_CYCLE);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    int val = dialog.value().toDouble();
                    if(val < 1){
                        val = 1;
                    }else if(val > MAX_CYCLE){
                        val = MAX_CYCLE;
                    }

                    if(m_d->selectedStageRow >= 0 && m_d->selectedStageRow < m_d->stageModel->rowCount()){
                        //如果是Touchdown或Long模式,判断
                        if(m_d->specialIndex>=0){
                            if(m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 1){
                                int dd = m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex];
                                int s1 = m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex];
                                if(s1<MIN_TOUCHDOWN_TEMP ) s1=MIN_TOUCHDOWN_TEMP;
                                int s2 = m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex];
                                int bb = m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex];

                                if(qAbs(s1-s2) > dd*(val-bb)){
                                    My_MessageBox mb;
                                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Cannot reach the target temperature within cycles."));
                                }
                            }else if(m_d->specialIndex>=0 && m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 2){
                                int dd = m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex];

                                int s1 = m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex];
                                int s2 = m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex];
                                int bb = m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex];

                                if(qAbs(s1-s2) > dd*(val-bb)){
                                    My_MessageBox mb;
                                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Cannot reach the target temperature within cycles."));
                                }
                            }
                        }

                        m_d->stageModel->setData(index, QString::number(val), Qt::DisplayRole);
                        if(m_runMethod && (m_runMethod->count() > m_d->selectedStageRow) && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
                            int prevCycle = m_runMethod->at(m_d->selectedStageRow)->Cycles;
                            m_runMethod->at(m_d->selectedStageRow)->Cycles = val;
                            if(prevCycle==1 && m_d->selectedStageRow==m_runMethod->count()-1){
                                if(m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex] == 0){
                                    m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex] = 30;
                                    refreshStepDescribe(m_d->selectedStageRow,m_runMethod->at(m_d->selectedStageRow)->SubNum-1);
                                }
                            }
                        }
                    }

                    m_d->selectedStageRow = -1;

                    saveMethodFile();

                    if(m_d->item){
                        m_d->item->update();
                        m_d->plot->replot();
                    }

                    if(m_d->itemStageAdd){
                        m_d->itemStageAdd->update();
                        m_d->plotStageAdd->replot();
                    }
                }

                emit editting2(false);
            }
        }
    }else if(obj == ui->tableViewStep->viewport()){
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e && e->type() == QEvent::MouseButtonRelease){
            QModelIndex index = ui->tableViewStep->indexAt(e->pos());
            if(!index.isValid()) return true;
            if(index.column() < 2){
                //判断是否移动
                int min = ui->tableViewStep->verticalScrollBar()->minimum();
                int max = ui->tableViewStep->verticalScrollBar()->maximum();
                int single = ui->tableViewStep->verticalScrollBar()->singleStep();
                int current = ui->tableViewStep->verticalScrollBar()->value();
                int topRow = (current - min) / single;

                if(index.row() == topRow){
                    //先设置采样功能,再移动
                    int scroll_value_ = current-single;
                    if(current!=min && current!=scroll_value_){
                        ui->tableViewStep->verticalScrollBar()->setValue(scroll_value_);
                        ui->tableViewStep->viewport()->update();
                        return true;
                    }
                }
#if defined(DEVICE_TYPE_TL22)
                else if(index.row() == topRow+3){
#elif (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
                else if(index.row() == topRow+4){
#elif(DEVICE_TYPE_TL13)
                else if(index.row() == topRow+5){
#endif
                    int scroll_value_ = current+single;
                    if(current!=max && current!=scroll_value_){
                        ui->tableViewStep->verticalScrollBar()->setValue(current+single);
                        ui->tableViewStep->viewport()->update();
                        return true;
                    }
                }
            }
        }
    }else if(obj == ui->lineEditStepCenter){
        //35.5 ~ 99.5
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            GDataInputDialog dialog(tr("Temp. Center [%1]").arg(trUtf8("℃")), ui->lineEditStepCenter->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setDoubleRange(MIN_CENTER_TEMP, MAX_CENTER_TEMP, TEMP_DECIMAL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double offset = ui->lineEditStepOffset->text().toDouble();
                double val = dialog.value().toDouble();
                if(val < MIN_CENTER_TEMP){
                    val = MIN_CENTER_TEMP;
                }else if(val > MAX_CENTER_TEMP){
                    val = MAX_CENTER_TEMP;
                }
                ui->lineEditStepCenter->setText(QString::number(val,'f',1));
                ui->lineEditStepCenter->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditStepOffset){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            GDataInputDialog dialog(tr("Temp. Offset [%1]").arg(trUtf8("℃")), ui->lineEditStepOffset->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setDoubleRange(MIN_OFFSET_TEMP, MAX_OFFSET_TEMP, TEMP_DECIMAL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double center = ui->lineEditStepCenter->text().toDouble();
                double val = dialog.value().toDouble();
                if(val < MIN_OFFSET_TEMP){
                    val = MIN_OFFSET_TEMP;
                }else if(val > MAX_OFFSET_TEMP){
                    val = MAX_OFFSET_TEMP;
                }

                ui->lineEditStepOffset->setText(QString::number(val,'f',1));
                ui->lineEditStepOffset->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = center;

                }
            }
            return true;
        }
    }else if(obj == ui->lineEditStepTarValue){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            if(ui->radioButtonStepTouchdown->isChecked()){
                GDataInputDialog dialog(tr("Target Temp. [%1]").arg(trUtf8("℃")), ui->lineEditStepTarValue->text(), this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setDoubleRange(MIN_TOUCHDOWN_TEMP, MAX_TOUCHDOWN_TEMP, TEMP_DECIMAL);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    double val = dialog.value().toDouble();
                    if(val < MIN_TOUCHDOWN_TEMP){
                        val = MIN_TOUCHDOWN_TEMP;
                    }else if(val > MAX_TOUCHDOWN_TEMP){
                        val = MAX_TOUCHDOWN_TEMP;
                    }

                    ui->lineEditStepTarValue->setText(QString::number(val,'f',1));
                    ui->lineEditStepTarValue->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){

                        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = val;
                    }
                }
            }else{
                int minutes = 0, seconds = 0;
                QRegExp rx("(\\d*):(\\d*)");
                int dPos = rx.indexIn(ui->lineEditStepTarValue->text());
                if (dPos > -1) {
                    minutes = rx.cap(1).toInt();
                    seconds = rx.cap(2).toInt();
                }

                GTimeInputDialog dialog(tr("Target Time"), minutes, seconds, false, this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setIntRange(1, MAX_DURATION);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    int val = dialog.value().toInt();
                    if(val < 1){
                        val = 1;
                    }else if(val > MAX_DURATION){
                        val = MAX_DURATION;
                    }

                    minutes = val / 60;
                    seconds = val % 60;
                    ui->lineEditStepTarValue->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
                    ui->lineEditStepTarValue->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = val;
                    }
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditStepDelta){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            if(ui->radioButtonStepTouchdown->isChecked()){
                GDataInputDialog dialog(tr("Delta Temp. [%1/%2]").arg(trUtf8("℃")).arg(tr("Cycle")), ui->lineEditStepDelta->text(), this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setDoubleRange(MIN_TEMP_DETA, MAX_TEMP_DETA, TEMP_DECIMAL);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    double val = dialog.value().toDouble();

                    if(val < MIN_TEMP_DETA){
                        val = MIN_TEMP_DETA;
                    }else if(val > MAX_TEMP_DETA){
                        val = MAX_TEMP_DETA;
                    }

                    ui->lineEditStepDelta->setText(QString::number(val,'f',1));
                    ui->lineEditStepDelta->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){

                        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = val * 100;
                    }
                }
            }else{
                int minutes = 0, seconds = 0;
                QRegExp rx("(\\d*):(\\d*)");
                int dPos = rx.indexIn(ui->lineEditStepDelta->text());
                if (dPos > -1) {
                    minutes = rx.cap(1).toInt();
                    seconds = rx.cap(2).toInt();
                }

                GTimeInputDialog dialog(tr("Delta Time [/%1]").arg(tr("Cycle")), minutes, seconds, false, this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setIntRange(1, MAX_DURATION_DETA);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    int val = dialog.value().toInt();

                    if(val < 1){
                        val = 1;
                    }else if(val > MAX_DURATION_DETA){
                        val = MAX_DURATION_DETA;
                    }

                    minutes = val / 60;
                    seconds = val % 60;
                    ui->lineEditStepDelta->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
                    ui->lineEditStepDelta->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = val;
                    }
                }
            }            
            return true;
        }
    }else if(obj == ui->lineEditStepBeginCycle){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            int cycleCount = MAX_CYCLE;
            if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
                cycleCount = m_runMethod->at(m_d->selectedStageRow)->Cycles;
            }
            GDataInputDialog dialog(tr("Start Cycle"), ui->lineEditStepBeginCycle->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(1, cycleCount);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 1){
                    val = 1;
                }else if(val > cycleCount){
                    val = cycleCount;
                }

                ui->lineEditStepBeginCycle->setText(QString::number(val));
                ui->lineEditStepBeginCycle->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex] = val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditMeltTarget){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            double minTemp = MIN_MELT_TEMP1;
            if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                    && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                if((m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 4) \
                        || ((m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum-1) \
                            && (m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->selectedStepRow+1] == 4))){
                    minTemp = MIN_MELT_TEMP2;
                }
            }

            GDataInputDialog dialog(tr("Temperature [%1]").arg(trUtf8("℃")), ui->lineEditMeltTarget->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setDoubleRange(minTemp, MAX_MELT_TEMP, TEMP_DECIMAL);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double val = dialog.value().toDouble();
                if(val < minTemp){
                    val = minTemp;
                }else if(val > MAX_MELT_TEMP){
                    val = MAX_MELT_TEMP;
                }

                ui->lineEditMeltTarget->setText(QString::number(val,'f',1));
                ui->lineEditMeltTarget->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditMeltDuration){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();

            int minutes = 0, seconds = 0;
            QRegExp rx("(\\d*):(\\d*)");
            int dPos = rx.indexIn(ui->lineEditMeltDuration->text());
            if (dPos > -1) {
                minutes = rx.cap(1).toInt();
                seconds = rx.cap(2).toInt();
            }

            GTimeInputDialog dialog(tr("Time"), minutes, seconds, false, this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
            dialog.setIntRange(1, MAX_DURATION);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                int val = dialog.value().toInt();
                if(val < 1){
                    val = 1;
                }else if(val > MAX_DURATION){
                    val = MAX_DURATION;
                }

                minutes = val / 60;
                seconds = val % 60;
                ui->lineEditMeltDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
                ui->lineEditMeltDuration->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
                        && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
                    m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex] = val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditMeltRamp){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            GDataInputDialog dialog(tr("Ramp [%1]").arg(trUtf8("℃/s")), ui->lineEditMeltRamp->text(), this);
            connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));

            int z = 0;
            //            if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL)&& (m_d->selectedStepRow > 0) && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)&& (m_runMethod->at(m_d->selectedStageRow)->Steps.at(m_d->selectedStepRow-1) != NULL)&& (m_runMethod->at(m_d->selectedStageRow) != NULL)&& (m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->selectedStepRow-1] > m_runMethod->at(m_d->selectedStageRow)->Temp)){
            //                    z = 1;
            //            }

            dialog.setDoubleRange(MIN_RAMP,m_pool->maxSpeed[z],1);

            QPoint pos = this->mapToGlobal(this->geometry().center());
            dialog.move(pos.x()-176, pos.y()-167);

            if(dialog.exec() == QDialog::Accepted){
                double val = dialog.value().toDouble();
                if(val < MIN_RAMP){
                    val = MIN_RAMP;
                }else if(val > m_pool->maxSpeed[z]){
                    val = m_pool->maxSpeed[z];
                }

                //                val = (val / 10) * 10;
                ui->lineEditMeltRamp->setText(QString::number(val,'f',1));
                ui->lineEditMeltRamp->clearFocus();

                if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                        && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
                    m_runMethod->at(m_d->selectedStageRow)->Ramp[m_d->specialIndex] =val;
                }
            }
            return true;
        }
    }else if(obj == ui->lineEditMeltCon){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
            if(m_d->isSingleStep==true){
                GDataInputDialog dialog(tr("Increment [%1]").arg(trUtf8("℃")), ui->lineEditMeltCon->text(), this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setDoubleRange(MIN_SINGLESTEP_TEMP, MAX_SINGLESTEP_TEMP, 1);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    double val = dialog.value().toDouble();
                    if(val < MIN_SINGLESTEP_TEMP){
                        val = MIN_SINGLESTEP_TEMP;
                    }else if(val > MAX_SINGLESTEP_TEMP){
                        val = MAX_SINGLESTEP_TEMP;
                    }

                    ui->lineEditMeltCon->setText(QString::number(val,'f',1));
                    ui->lineEditMeltCon->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
                            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
                        m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex] = val;
                    }
                }
            }else{
                GDataInputDialog dialog(tr("Readings [Readings/%1]").arg(trUtf8("℃")), ui->lineEditMeltCon->text(), this);
                connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
                dialog.setIntRange(MIN_CONTINUE_TIMES, MAX_CONTINUE_TIMES);

                QPoint pos = this->mapToGlobal(this->geometry().center());
                dialog.move(pos.x()-176, pos.y()-167);

                if(dialog.exec() == QDialog::Accepted){
                    int val = dialog.value().toInt();
                    if(val < MIN_CONTINUE_TIMES){
                        val = MIN_CONTINUE_TIMES;
                    }else if(val > MAX_CONTINUE_TIMES){
                        val = MAX_CONTINUE_TIMES;
                    }

                    ui->lineEditMeltCon->setText(QString::number(val));
                    ui->lineEditMeltCon->clearFocus();

                    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
                            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
                            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
                        m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex] = val;
                    }
                }
            }
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

/*!
* \brief 类GRunEditor的私有函数，初始化私有变量
* \param 无
* \return 无
*/
void GRunEditor::initVariables()
{
    //设置输入
    ui->labelReaction->installEventFilter(this);
    ui->labelHotLid->installEventFilter(this);

    ui->tableViewStage->viewport()->installEventFilter(this);
    ui->tableViewStep->viewport()->installEventFilter(this);

    ui->lineEditStepTarget->installEventFilter(this);
    ui->lineEditStepDuration->installEventFilter(this);
    ui->lineEditStepRamp->installEventFilter(this);

    ui->lineEditStepCenter->installEventFilter(this);
    ui->lineEditStepOffset->installEventFilter(this);

    ui->lineEditStepTarValue->installEventFilter(this);
    ui->lineEditStepDelta->installEventFilter(this);
    ui->lineEditStepBeginCycle->installEventFilter(this);

    ui->lineEditMeltTarget->installEventFilter(this);
    ui->lineEditMeltDuration->installEventFilter(this);
    ui->lineEditMeltRamp->installEventFilter(this);
    ui->lineEditMeltCon->installEventFilter(this);

    //新建通道设置模板
    m_d->channelModel = new QStandardItemModel;
    if(m_d->channelModel){
        //设置横向标题
        QStringList header;
        header << tr("Channel") << tr("Dye") << tr("Excitation/Emission");
        m_d->channelModel->setHorizontalHeaderLabels(header);

        //设置所有通道的数据

        QStringList excitations;
#if (defined DEVICE_TYPE_TL12)
        excitations << "470/525" << "523/564" << "571/612" << "630/670" << "680/730" << "465/616";
#else
        excitations << "465/510" << "527/563" << "580/616" << "632/664";
#ifdef DEVICE_TYPE_TL22
        if(GHelper::total_instrument_id == 201)
            excitations << "527/616";
        else
            excitations << "680/730";

        if ( GHelper::total_instrument_id==202 || GHelper::total_instrument_id==203 )
            excitations << "527/616";
        else if ( GHelper::total_instrument_id==204 )
            excitations << "425/584";
        else if( GHelper::total_instrument_id==205 )
            excitations << "750/794";
        else
            excitations << "465/616";
#else
        excitations << "680/730" << "465/616";
#endif
#endif

        m_d->channelModel->setData(m_d->channelModel->index(4,0), QVariant(4?1:0), Qt::EditRole);
        for(int i=0; i<GHelper::channel_count; i++){
            m_d->channelModel->appendRow(new QStandardItem);
            //通道号及是否选择
#if (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
            m_d->channelModel->setData(m_d->channelModel->index(i,0), QVariant(1), Qt::EditRole);
#else
            m_d->channelModel->setData(m_d->channelModel->index(i,0), QVariant(i==0?1:0), Qt::EditRole);
#endif
            //探针类型
            QByteArray probeType;
            probeType.append(GHelper::indexOfFluorGroup(i+1));
            m_d->channelModel->setData(m_d->channelModel->index(i,1), i==0?probeType:QVariant(), Qt::EditRole);
            //激发
            QString txt = i < excitations.count() ? excitations.at(i) : "-";
            m_d->channelModel->setData(m_d->channelModel->index(i,2), txt, Qt::DisplayRole);
        }

    }

    //新建代理
#ifndef DEVICE_TYPE_TL13
    m_d->channelDelegate = new GChannelSetDelegate(ui->tableViewChannel);

    connect(m_d->channelDelegate, SIGNAL(itemEdited(int,QByteArray)), this, SLOT(slot_itemEdited(int,QByteArray)));
    connect(m_d->channelDelegate, SIGNAL(itemUnableClicked()), this, SLOT(slot_itemUnableClicked()));
    connect(m_d->channelDelegate, SIGNAL(editCtrl(bool)), this, SIGNAL(editting2(bool)));

    connect(m_d->channelDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(slot_closeEditor()));
    connect(m_d->channelDelegate, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
#endif
    //新建过程模板
    m_d->stageModel = new QStandardItemModel;
    if(m_d->stageModel){
        QStringList header;
        header << tr("Stage") << tr("Cycle") << tr("Type");
        m_d->stageModel->setHorizontalHeaderLabels(header);

        connect(m_d->stageModel, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(slot_stage_insertOrRemoved()));
        connect(m_d->stageModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(slot_stage_insertOrRemoved()));

    }

    //新建步骤模板
    m_d->stepModel = new QStandardItemModel;
    if(m_d->stepModel){
        QStringList header;

#ifdef DEVICE_TYPE_TL13
        header << tr("Temperature") << tr("Time")<< tr("Edit")<< tr("Type");
#else
        header << tr("Temperature") << tr("Time") << tr("Fluor") << tr("Edit") << tr("Type");
#endif
        m_d->stepModel->setHorizontalHeaderLabels(header);
    }

    //新建过程添加模板
    m_d->stageAddModel = new QStandardItemModel;
    if(m_d->stageAddModel){
        QStringList describes;

#ifdef DEVICE_TYPE_TL13
        describes << tr("Preincubation") \
                  << tr("Reverse Transcription") \
                  << tr("2 Step Amplification") \
                  << tr("3 Step Amplification") \
                  << tr("Cooling") \
                  << tr("Custom Stage");
#else
        describes << tr("Preincubation") \
                  << tr("Reverse Transcription") \
                  << tr("2 Step Amplification") \
                  << tr("3 Step Amplification") \
                  << tr("Melting") \
                  << tr("Continuous Melting") \
                  << tr("Cooling") \
                  << tr("Custom Stage");
#endif

        for(int i=0; i<describes.count(); i++){
            int row = i / STAGEADD_COLUMN_COUNT;
            int col = i % STAGEADD_COLUMN_COUNT;
            m_d->stageAddModel->setItem(row,col, new QStandardItem(describes.at(i)));
        }
    }

    //新建过程添加显示代理
    m_d->stageAddDelegate = new GStageAddDelegate;

    //新建曲线
#ifdef DEVICE_TYPE_TL22
    m_d->plot = new QwtPlot;
    if(m_d->plot){
        m_d->plot->plotLayout()->setAlignCanvasToScales(true);

        m_d->plot->enableAxis(QwtPlot::yLeft, false);
        m_d->plot->enableAxis(QwtPlot::xBottom, false);

        m_d->plot->setAxisTitle(QwtPlot::yLeft, "");
        m_d->plot->setAxisTitle(QwtPlot::xBottom, "");

        m_d->plot->setAxisScale(QwtPlot::yLeft,0,115);
        //  m_d->plot->setAxisScale(QwtPlot::xBottom,0,100);
        m_d->plot->setAxisScale(QwtPlot::xBottom,0,40);
    }

    m_d->item = new GRunMethodItem(m_pool, m_runMethod,true);
    if(m_d->item){
        m_d->item->attach(m_d->plot);
    }
#endif
    //    //新建曲线stageadd
    //    m_d->plotStageAdd = new QwtPlot;
    //    if(m_d->plotStageAdd){
    //        m_d->plotStageAdd->plotLayout()->setAlignCanvasToScales(true);

    //        m_d->plotStageAdd->enableAxis(QwtPlot::yLeft, false);
    ////        m_d->plotStageAdd->enableAxis(QwtPlot::xBottom, false);

    //        m_d->plotStageAdd->setAxisTitle(QwtPlot::yLeft, "");
    //        m_d->plotStageAdd->setAxisTitle(QwtPlot::xBottom, "");

    //        m_d->plotStageAdd->setAxisScale(QwtPlot::yLeft,0,115);
    //        m_d->plotStageAdd->setAxisScale(QwtPlot::xBottom,0,100);
    //    }

    //    m_d->itemStageAdd = new GRunMethodItem(m_pool, m_runMethod,true);
    //    if(m_d->itemStageAdd){
    //        m_d->itemStageAdd->attach(m_d->plotStageAdd);
    //    }
}

/*!
* \brief 类GRunEditor的私有函数，初始化designer设计的界面
* \param 无
* \return 无
*/
void GRunEditor::initUi()
{
    ui->radioButtonStepGradient->setVisible(GHelper::has_gradient);

#ifndef DEVICE_TYPE_TL13
    //设置通道设置的模板和代理
    ui->tableViewChannel->setModel(m_d->channelModel);
    ui->tableViewChannel->setItemDelegate(m_d->channelDelegate);
    //隐藏通道设置的纵向标题
    ui->tableViewChannel->verticalHeader()->hide();
    //自动调整最后一列的宽度使它和表格的右边界对齐
    ui->tableViewChannel->horizontalHeader()->setStretchLastSection(true);
    //设置列宽度
    ui->tableViewChannel->setColumnWidth(0, CHANNEL_COL0_WIDTH);
    ui->tableViewChannel->setColumnWidth(1, CHANNEL_COL1_WIDTH);
    ui->tableViewChannel->setColumnWidth(2, CHANNEL_COL2_WIDTH);
    int ch_height = SHOW_CHANNEL_HEIGHT_PIXEL / GHelper::channel_count;
    ui->tableViewChannel->verticalHeader()->setDefaultSectionSize(ch_height);
    //去掉headerview的高亮功能
    ui->tableViewChannel->horizontalHeader()->setHighlightSections(false);
    ui->tableViewChannel->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewChannel->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    //设置headerview上的字体颜色为Qt::darkBlue
    ui->tableViewChannel->horizontalHeader()->setStyleSheet("QHeaderView::section {color:#000080;}");
#endif
    //设置过程和步骤的模板
    ui->tableViewStage->setModel(m_d->stageModel);
    ui->tableViewStage->verticalHeader()->hide();
    ui->tableViewStage->setColumnWidth(0, STAGE_COL0_WIDTH);
    ui->tableViewStage->setColumnWidth(1, STAGE_COL1_WIDTH);
    ui->tableViewStage->verticalHeader()->setDefaultSectionSize(STAGE_STEP_ITEM_HEIGHT);
    ui->tableViewStage->hideColumn(2);  //隐藏过程的类型
    ui->tableViewStage->setItemDelegateForColumn(1, new GRightAlignDeletgate);
    //去掉headerview的高亮功能
    ui->tableViewStage->horizontalHeader()->setHighlightSections(false);
    ui->tableViewStage->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewStage->horizontalHeader()->setFixedHeight(HORIZONTAL_HEADER_HEIGHT);
    ui->tableViewStage->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    connect(ui->tableViewStage->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), \
            this, SLOT(slot_tableViewStage_selectionChanged()));
    //  connect(ui->tableViewStage->verticalScrollBar(), SIGNAL(actionTriggered(int)), m_pool, SLOT(screen_sound()));

    ui->tableViewStep->setModel(m_d->stepModel);
    ui->tableViewStep->verticalHeader()->hide();

#ifdef DEVICE_TYPE_TL13
    ui->tableViewStep->setColumnWidth(0, STEP_COL0_WIDTH);
    ui->tableViewStep->setColumnWidth(1, STEP_COL0_WIDTH);
    ui->tableViewStep->setColumnWidth(3, STEP_COL3_WIDTH);
#else
    ui->tableViewStep->setColumnWidth(0, STEP_COL0_WIDTH);
    ui->tableViewStep->setColumnWidth(1, STEP_COL0_WIDTH);
    ui->tableViewStep->setColumnWidth(2, STEP_COL2_WIDTH);
    ui->tableViewStep->setColumnWidth(3, STEP_COL3_WIDTH);
#endif

    ui->tableViewStep->verticalHeader()->setDefaultSectionSize(STAGE_STEP_ITEM_HEIGHT);
    ui->tableViewStep->hideColumn(4);  //隐藏步骤的类型
    //去掉headerview的高亮功能
    ui->tableViewStep->horizontalHeader()->setHighlightSections(false);
    ui->tableViewStep->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewStep->horizontalHeader()->setFixedHeight(HORIZONTAL_HEADER_HEIGHT);
    ui->tableViewStep->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_d->stepCheckDelegate = new GCheckIndicatorDeletgate;
    ui->tableViewStep->setItemDelegateForColumn(2, m_d->stepCheckDelegate);
    connect(m_d->stepCheckDelegate, SIGNAL(checkChanged(int,bool)), this, SLOT(slot_stepCheckChanged(int,bool)));
    m_d->stepEditDelegate = new GEditIndicatorDeletgate;

#ifdef DEVICE_TYPE_TL13
    ui->tableViewStep->setItemDelegateForColumn(2, m_d->stepEditDelegate);
#else
    ui->tableViewStep->setItemDelegateForColumn(3, m_d->stepEditDelegate);
#endif

    connect(m_d->stepEditDelegate, SIGNAL(beginEditing()), this, SLOT(slot_buttonStepEdit_clicked()));

    connect(ui->tableViewStep->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), \
            this, SLOT(slot_tableViewStep_selectionChanged()));
    // connect(ui->tableViewStep->verticalScrollBar(), SIGNAL(actionTriggered(int)), m_pool, SLOT(screen_sound()));

    //设置过程添加的模板和代理
    ui->tableViewStageAdd->setModel(m_d->stageAddModel);
    ui->tableViewStageAdd->setItemDelegate(m_d->stageAddDelegate);
    for(int i=0; i<STAGEADD_COLUMN_COUNT; i++)
        ui->tableViewStageAdd->setColumnWidth(i, STAGEADD_ITEM_WIDTH);

    ui->tableViewStageAdd->verticalHeader()->setDefaultSectionSize(STAGEADD_ITEM_HEIGHT);

    connect(ui->tableViewStageAdd->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slot_stageAddChanged(QItemSelection)));

#ifdef DEVICE_TYPE_TL22
    //显示温控曲线
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_d->plot);
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0,0,0,3);

    ui->widget->setLayout(hLayout);
#endif

    //其他连接
    connect(ui->RunEditor, SIGNAL(currentChanged(int)), this, SLOT(slot_currentChanged(int)));
}

bool GRunEditor::currentStageIndex(int &index)
{
    if(m_d->stageModel->rowCount() != m_runMethod->count()) return false;
    if(!ui->tableViewStage->selectionModel()->hasSelection()) return false;
    index = ui->tableViewStage->selectionModel()->selectedRows().first().row();
    if(m_runMethod->at(index) == NULL) return false;
    if(m_d->stepModel->rowCount() != m_runMethod->at(index)->SubNum) return false;
    return true;
}

void GRunEditor::refreshStepDescribe(int stageNo, int stepNo)
{
    if(stageNo < 0 || stepNo < 0) return;
    if(m_d->stageModel->rowCount() != m_runMethod->count()) return;
    if(m_runMethod->at(stageNo) == NULL) return;
    if(m_d->stepModel->rowCount() != m_runMethod->at(stageNo)->SubNum) return;

    //目标温度
    QString txt;
    switch(m_runMethod->at(stageNo)->SubProperty[stepNo]){
    case 1:
        txt = trUtf8("%1»%2℃").arg(QString::number(m_runMethod->at(stageNo)->Temp[stepNo],'f',1)) \
                .arg(QString::number(m_runMethod->at(stageNo)->TarValue[stepNo],'f',1));
        break;
    case 2:
        txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(stageNo)->Temp[stepNo],'f',1));
        break;
    case 3:{
        double deta = qAbs(m_runMethod->at(stageNo)->Temp[stepNo]-m_runMethod->at(stageNo)->Temp[stepNo]);
        txt = trUtf8("%1±%2℃").arg(QString::number(m_runMethod->at(stageNo)->Temp[stepNo],'f',1)) \
                .arg(QString::number(deta/100.0,'f',1));

        qDebug()<<" ------------------3:------00000-----------------------"<<deta;
        break;}
    case 4:break;
    default:
        txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(stageNo)->Temp[stepNo],'f',1));
        break;
    }
    m_d->stepModel->setData(m_d->stepModel->index(stepNo,0), txt, Qt::DisplayRole);

    //恒温时间
    txt.clear();
    switch(m_runMethod->at(stageNo)->SubProperty[stepNo]){
    case 2:{
        int minute1 = m_runMethod->at(stageNo)->Time[stepNo] / 60;
        int second1 = m_runMethod->at(stageNo)->Time[stepNo] % 60;
        int minute2 = m_runMethod->at(stageNo)->TarValue[stepNo] / 60;
        int second2 = (int)(m_runMethod->at(stageNo)->TarValue[stepNo]) % 60;
        txt = tr("%1:%2»%3:%4").arg(QString::number(minute1).rightJustified(2,'0',true)) \
                .arg(QString::number(second1).rightJustified(2,'0',true))
                .arg(QString::number(minute2).rightJustified(2,'0',true))
                .arg(QString::number(second2).rightJustified(2,'0',true));
        break;
    }
    case 4:break;
    default:{
        int minutes = m_runMethod->at(stageNo)->Time[stepNo] / 60;
        int seconds = m_runMethod->at(stageNo)->Time[stepNo] % 60;
        if(minutes==0 && seconds==0){
            txt = "∞";
        }else{
            txt = tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true));
        }
        break;
    }
    }
    m_d->stepModel->setData(m_d->stepModel->index(stepNo,1), txt, Qt::DisplayRole);
    int isSample = m_runMethod->at(stageNo)->ReadFluor[stepNo];
    m_d->stepModel->setData(m_d->stepModel->index(stepNo,2), isSample, Qt::DisplayRole);
    m_d->stepModel->setData(m_d->stepModel->index(stepNo,4), QVariant::fromValue(0), Qt::EditRole);
}

void GRunEditor::saveMethodFile()
{
    qDebug()<<"-----------------saveMethodFile------------------001---------"<<m_pool->fileOpType;
    if(m_pool->fileOpType == 1){
        //新建实验时，保存文件
        emit methodChanged();

        int ret = -2;
        if(!m_pool->expFile->fileName().isEmpty())
            ret = m_pool->expFile->save();
        qDebug() << "save result:" << ret << m_runMethod->count();
    }else if(m_pool->fileOpType == 2){
        //打开实验时，设置编辑状态
        emit methodEdited();
    }
}

void GRunEditor::setShowDetail(bool normal)
{
    bool isOk = normal ? true : m_d->isSingleStep;
    ui->labelMeltDuration->setVisible(isOk);
    ui->lineEditMeltDuration->setVisible(isOk);

    ui->labelMeltRamp->setVisible(normal);
    ui->lineEditMeltRamp->setVisible(normal);
    ui->labelMeltRampUnit->setVisible(normal);

    ui->stepMeltAcquisition->setVisible(!normal);

    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
        qDebug()<<"-------------m_d->isSingleStep -----------oooooooooooo--------------"<<m_d->isSingleStep<<normal;
        m_runMethod->at(m_d->selectedStageRow)->ReadMode[m_d->selectedStepRow] = (normal ? 0 : (m_d->isSingleStep ? 1 : 2));
        m_runMethod->at(m_d->selectedStageRow)->ReadFluor[m_d->selectedStepRow] = (normal ? 0 : 1);
    }
}

bool GRunEditor::initCheckup(bool checkTlpd)
{
    qDebug() << Q_FUNC_INFO << "step1" << m_pool;
    if(m_pool == NULL){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("No data storage."));
        return false;
    }
    qDebug() << Q_FUNC_INFO << "step2";
    if(m_pool->expFile==NULL || m_pool->expFile->fileName().isEmpty()){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Please select or create a new file first."));
        return false;
    }
    qDebug() << Q_FUNC_INFO << checkTlpd << m_pool->expFile->is_compressed_file << m_pool->expFile->fileName();
    if(checkTlpd && m_pool->expFile->is_compressed_file!=0){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Tlpd file can not be modified."));
        return false;
    }

    return true;
}

#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
void GRunEditor::on_buttonTubeTypeNext_clicked()
{
    m_pool->screen_sound();
    if(!initCheckup(true)) return;
    QStringList titles, simplifys;

    titles << tr("Clear") << tr("White") << tr("Frosted");
    simplifys << "C" << "W" << "F";

    QString txt = ui->labelTubeType->property("simplify").toString().trimmed();
    int pos = -1;
    if(simplifys.contains(txt, Qt::CaseInsensitive)){
        pos = simplifys.indexOf(txt);
        pos = pos < 2 ? (pos+1) : 0;
    }
    pos = pos < 0 ? 0 : pos;

    ui->labelTubeType->setText(titles.at(pos));
    ui->labelTubeType->setProperty("simplify", simplifys.at(pos));

    saveMethodFile();
}
#endif

void GRunEditor::on_buttonReactionAdd_clicked()
{
    m_pool->screen_sound();

    QRegExp rx("(\\d+)");
    QString str = ui->labelReaction->text();
    str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;

    int val = str.toInt();
    if(val < MAX_REACTION_VOL){
        val++;

        ui->labelReaction->setText(QString::number(val)+" μL");

        if(m_runInfo){
            m_runInfo->reactionValume = val;
        }
        saveMethodFile();
    }
}

void GRunEditor::on_buttonReactionReduce_clicked()
{
    m_pool->screen_sound();

    QRegExp rx("(\\d+)");
    QString str = ui->labelReaction->text();
    str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;

    int val = str.toInt();
    if(val > MIN_REACTION_VOL){
        val--;

        ui->labelReaction->setText(QString::number(val)+" μL");

        if(m_runInfo){
            m_runInfo->reactionValume = val;
        }
        saveMethodFile();

        if(val == 0){
            My_MessageBox mb;
            mb.gwarning(m_pool, NULL, tr("Warning"), tr("Block control mode"));
        }
    }
}

void GRunEditor::on_checkBoxHotCover_toggled(bool checked)
{
    m_pool->screen_sound();

    qDebug() << "tlpd checkup 3";

    ui->labelHotLid->setEnabled(checked);
    m_runInfo->lidStatus = checked?1:0;

    qDebug() << "save method file 3";
    saveMethodFile();

    //如果实验正在运行,退出
    _PCR_RUN_CTRL_INFO pcrInfo;

    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(!checked && pcrInfo.State.ExpState!=1){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Hot Lid heating function is unused."));
    }
}

void GRunEditor::on_buttonHotlidAdd_clicked()
{
    m_pool->screen_sound();

    QRegExp rx("(\\d+)");
    QString str = ui->labelHotLid->text();
    str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;

    int val = str.toInt();
    if(val < MAX_HOTCOVER_VOL){
        val++;
        ui->labelHotLid->setText(QString::number(val)+"  ℃");
        ui->labelHotLid->setStyleSheet(val<100?"background-color:yellow;":"");
        if(m_runInfo){
            m_runInfo->lidValue = val;
        }
        saveMethodFile();
    }
}

void GRunEditor::on_buttonHotlidReduce_clicked()
{
    m_pool->screen_sound();

    QRegExp rx("(\\d+)");
    QString str = ui->labelHotLid->text();
    str = (rx.indexIn(str) != -1) ? rx.cap(1) : QString::null;

    int val = str.toInt();
    if(val > MIN_HOTCOVER_VOL){
        val--;
        ui->labelHotLid->setText(QString::number(val)+"  ℃");
        ui->labelHotLid->setStyleSheet(val<100?"background-color:yellow;":"");
        if(m_runInfo){
            m_runInfo->lidValue = val;
        }
        saveMethodFile();
    }
}


void GRunEditor::on_buttonStageUp_clicked()
{
    m_pool->screen_sound();
    qDebug() << "selected stage up";

    qDebug() << "tlpd checkup 9";
    if(!initCheckup(true)) return;

    if(m_d->stageModel->rowCount() != m_runMethod->count()) return;
    if(!ui->tableViewStage->selectionModel()->hasSelection()) return;

    QModelIndex index = ui->tableViewStage->selectionModel()->selectedRows().first();
    int row = index.row();
    if(row == 0) return;

    if(row==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 && m_runMethod->last()->Time[m_d->specialIndex]==0){
        m_runMethod->last()->Time[m_d->specialIndex] = 30;
    }

    QList<QStandardItem *> items = m_d->stageModel->takeRow(row);
    m_d->stageModel->insertRow(row-1, items);

    m_runMethod->swap(row, row-1);

    ui->tableViewStage->clearSelection();
    ui->tableViewStage->selectRow(row-1);

    emit operatorLog(tr("Move stage up"));
    qDebug() << "save method file 9";

    saveMethodFile();

    if(m_d->item){
        m_d->item->update();
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->update();
        m_d->plotStageAdd->replot();
    }
}

void GRunEditor::on_buttonStageAdd_clicked()
{
    qDebug() << "tableViewStage & tableViewStep viewport size:" << ui->tableViewStage->viewport()->size() << ui->tableViewStep->viewport()->size();

    m_pool->screen_sound();
    qDebug() << "add new stage";

    if(!initCheckup(true)) return;
    if(m_d->stageModel->rowCount() != m_runMethod->count()) return;

    if(m_runMethod->count() >= MAX_STAGE){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Stage outside range(1~%1)").arg(MAX_STAGE));
        return;
    }

    if(GHelper::total_instrument_id == 6){
        //扩增中只能有一个stage可以采样
        for(int i=0; i<m_runMethod->count(); i++){
            if(m_runMethod->at(i) == NULL) continue;
            _STAGE_INFO *stageInfo = m_runMethod->at(i);
            if(stageInfo->Property != 0) continue;
            for(int j=0; j<stageInfo->SubNum; j++){
                stageInfo->ReadFluor[j] = 0;
            }
        }

        QString text_ = tr("Thermostatic Amplification");
        QString stageName_ = text_;
        int cycleCount_ = 40;

        _STAGE_INFO *stage = new _STAGE_INFO();
        if(stage){
            int len = stageName_.toUtf8().size();
            len = len < 64 ? len : 64;

            int row = 0;
            int repeat = 0;
            while(row < m_d->stageModel->rowCount()){
                if(repeat != 0){
                    stageName_ = text_ + tr("(%1)").arg(repeat);
                    len = stageName_.toUtf8().size() < 64 ? stageName_.toUtf8().size() : 64;
                }
                if(m_d->stageModel->data(m_d->stageModel->index(row,0),Qt::DisplayRole).toString().trimmed() == stageName_.left(len)){
                    row = 0;
                    repeat ++;
                }else
                    row ++;
            }

            memcpy((void*)stage->Name, (const void*)(stageName_.toUtf8().data()), len);
            stage->Cycles = cycleCount_;
            stage->Property = 0;
            stage->SubProperty[0] = 0;
            stage->Temp[0] = 6000;
            stage->Time[0] = 20;
            stage->ReadInterval[0] = 0;
            stage->ReadFluor[0] = 1;
            stage->ReadMode[0] = 0;
            stage->Ramp[0] = m_pool->maxSpeed[0]*100;
            stage->BeginCycle[0] = 0;
            stage->Delta[0] = 0;
            stage->TarValue[0] = 0;
            stage->SubNum = 1;
            m_runMethod->append(stage);
        }

        m_d->stageModel->appendRow(new QStandardItem);
        int row = m_d->stageModel->rowCount() - 1;
        m_d->selectedStageRow = row;
        m_d->stageModel->setData(m_d->stageModel->index(row,0), stageName_, Qt::DisplayRole);
        m_d->stageModel->setData(m_d->stageModel->index(row,1), cycleCount_, Qt::DisplayRole);
        m_d->stageModel->setData(m_d->stageModel->index(row,2), 7, Qt::EditRole);
        ui->tableViewStage->clearSelection();
        ui->tableViewStage->selectRow(row);

        if(m_pool->fileOpType == 1) emit sampleChanged();
        saveMethodFile();

        if(m_d->item){
            m_d->item->currentSelect(m_d->selectedStageRow,0);
            m_d->plot->replot();
        }
    }else{
        //在最后添加
        if(ui->tableViewStage->selectionModel()->hasSelection()){
            m_d->selectedStageRow = m_d->stageModel->rowCount()-1;
        }else
            m_d->selectedStageRow = -1;
#ifndef DEVICE_TYPE_TL13
        ui->RunEditor->setTabEnabled(1, false);
#endif
        emit editting(true);
        ui->stackedWidget->setCurrentIndex(1);

        if(m_d->itemStageAdd){
            m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,0);
            m_d->plotStageAdd->replot();
        }
    }

    emit operatorLog(tr("Add stage"));

    qDebug() << "save method file 10";
}

void GRunEditor::on_buttonStageReduce_clicked()
{
    if(this->property("Stage_Status").toBool()) return;
    this->setProperty("Stage_Status", true);

    m_pool->screen_sound();
    qDebug() << "reduce selected stage";

    qDebug() << "tlpd checkup 12";
    if(!initCheckup(true)){
        this->setProperty("Stage_Status", false);
        return;
    }

    if(m_d->stageModel->rowCount() != m_runMethod->count()){
        this->setProperty("Stage_Status", false);
        return;
    }

    if(!ui->tableViewStage->selectionModel()->hasSelection()){
        this->setProperty("Stage_Status", false);
        return;
    }

    QModelIndex index = ui->tableViewStage->selectionModel()->selectedRows().first();
    m_d->stepModel->removeRows(0, m_d->stepModel->rowCount());
    int row = index.row();
    m_d->stageModel->removeRow(row);

    _STAGE_INFO *stage = m_runMethod->takeAt(row);
    delete stage;

    emit operatorLog(tr("Delete stage"));
    qDebug() << "save method file 11";

    saveMethodFile();

    if(m_d->item){
        int stageNo = row;
        if(row > m_d->stageModel->rowCount()){
            stageNo = m_d->stageModel->rowCount() - 1;
        }
        if(stageNo < 0)
            m_d->item->update();
        else
            m_d->item->currentSelect(stageNo,0);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        int stageNo = row;
        if(row > m_d->stageModel->rowCount()){
            stageNo = m_d->stageModel->rowCount() - 1;
        }
        if(stageNo < 0)
            m_d->itemStageAdd->update();
        else
            m_d->itemStageAdd->currentSelect(stageNo,0);
        m_d->plotStageAdd->replot();
    }

    this->setProperty("Stage_Status", false);
}

void GRunEditor::on_buttonStageDown_clicked()
{
    if(this->property("Stage_Status").toBool()) return;
    this->setProperty("Stage_Status", true);

    m_pool->screen_sound();
    qDebug() << "selected stage down";

    qDebug() << "tlpd checkup 13";
    if(!initCheckup(true)){
        this->setProperty("Stage_Status", false);
        return;
    }

    if(m_d->stageModel->rowCount() != m_runMethod->count()){
        this->setProperty("Stage_Status", false);
        return;
    }
    if(!ui->tableViewStage->selectionModel()->hasSelection()){
        this->setProperty("Stage_Status", false);
        return;
    }

    QModelIndex index = ui->tableViewStage->selectionModel()->selectedRows().first();
    int row = index.row();

    if(row >= m_d->stageModel->rowCount()-1){
        this->setProperty("Stage_Status", false);
        return;
    }

    if(row==m_runMethod->count()-2 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 && m_runMethod->last()->Time[m_d->specialIndex]==0){
        m_runMethod->last()->Time[m_d->specialIndex] = 30;
    }

    QList<QStandardItem *> items = m_d->stageModel->takeRow(row);
    m_d->stageModel->insertRow(row+1, items);

    m_runMethod->swap(row, row+1);

    ui->tableViewStage->clearSelection();
    ui->tableViewStage->selectRow(row+1);

    emit operatorLog(tr("Move stage down"));
    qDebug() << "save method file 12";

    saveMethodFile();

    if(m_d->item){
        m_d->item->update();
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->update();
        m_d->plotStageAdd->replot();
    }

    this->setProperty("Stage_Status", false);
}

void GRunEditor::on_buttonStepUp_clicked()
{
    m_pool->screen_sound();
    qDebug() << "selected step up";

    qDebug() << "tlpd checkup 14";
    if(!initCheckup(true)) return;

    int stageNo = 0;
    if(!currentStageIndex(stageNo)) return;
    if(!ui->tableViewStep->selectionModel()->hasSelection()) return;

    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
    int row = index.row();

    if(row == 0) return;
    if(row == m_d->specialIndex){
        m_d->specialIndex--;
    }else if(row > m_d->specialIndex){
        if(row == m_d->specialIndex+1)
            m_d->specialIndex++;
    }
    bool hasInfinite = false;
    if(stageNo==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 \
            && row==m_runMethod->last()->SubNum-1 && m_runMethod->last()->Time[row]==0){
        m_runMethod->last()->Time[row] = 30;
        hasInfinite = true;
    }

    QList<QStandardItem *> items = m_d->stepModel->takeRow(row);

    int deta = 1;
    //如果是熔解并且是采样步骤下的一个步骤
    if(m_runMethod->at(stageNo)->Property!=0 && row==m_d->specialIndex){
        deta = 2;
        for(int i=m_runMethod->at(stageNo)->SubNum; i>=row-deta; --i){
            m_runMethod->at(stageNo)->Temp[i]         = m_runMethod->at(stageNo)->Temp[i-1];
            m_runMethod->at(stageNo)->Ramp[i]         = m_runMethod->at(stageNo)->Ramp[i-1];
            m_runMethod->at(stageNo)->Time[i]         = m_runMethod->at(stageNo)->Time[i-1];
            m_runMethod->at(stageNo)->SubProperty[i]  = m_runMethod->at(stageNo)->SubProperty[i-1];
            m_runMethod->at(stageNo)->ReadInterval[i] = m_runMethod->at(stageNo)->ReadInterval[i-1];
            m_runMethod->at(stageNo)->ReadFluor[i]    = m_runMethod->at(stageNo)->ReadFluor[i-1];
            m_runMethod->at(stageNo)->ReadMode[i]     = m_runMethod->at(stageNo)->ReadMode[i-1];
            m_runMethod->at(stageNo)->TarValue[i]     = m_runMethod->at(stageNo)->TarValue[i-1];
            m_runMethod->at(stageNo)->BeginCycle[i]   = m_runMethod->at(stageNo)->BeginCycle[i-1];
            m_runMethod->at(stageNo)->Delta[i]        = m_runMethod->at(stageNo)->Delta[i-1];
        }
        m_runMethod->at(stageNo)->SubNum++;
    }else{
        float   Temp         = m_runMethod->at(stageNo)->Temp[row];
        quint32 Ramp         = m_runMethod->at(stageNo)->Ramp[row];
        quint32 Time         = m_runMethod->at(stageNo)->Time[row];
        quint32 SubProperty  = m_runMethod->at(stageNo)->SubProperty[row];
        quint32 ReadInterval = m_runMethod->at(stageNo)->ReadInterval[row];
        quint32 ReadFluor    = m_runMethod->at(stageNo)->ReadFluor[row];
        quint32 ReadMode     = m_runMethod->at(stageNo)->ReadMode[row];
        quint32 TarValue     = m_runMethod->at(stageNo)->TarValue[row];
        quint32 BeginCycle   = m_runMethod->at(stageNo)->BeginCycle[row];
        quint32 Delta        = m_runMethod->at(stageNo)->Delta[row];
        m_runMethod->at(stageNo)->Temp[row]         = m_runMethod->at(stageNo)->Temp[row-deta];
        m_runMethod->at(stageNo)->Ramp[row]         = m_runMethod->at(stageNo)->Ramp[row-deta];
        m_runMethod->at(stageNo)->Time[row]         = m_runMethod->at(stageNo)->Time[row-deta];
        m_runMethod->at(stageNo)->SubProperty[row]  = m_runMethod->at(stageNo)->SubProperty[row-deta];
        m_runMethod->at(stageNo)->ReadInterval[row] = m_runMethod->at(stageNo)->ReadInterval[row-deta];
        m_runMethod->at(stageNo)->ReadFluor[row]    = m_runMethod->at(stageNo)->ReadFluor[row-deta];
        m_runMethod->at(stageNo)->ReadMode[row]     = m_runMethod->at(stageNo)->ReadMode[row-deta];
        m_runMethod->at(stageNo)->TarValue[row]     = m_runMethod->at(stageNo)->TarValue[row-deta];
        m_runMethod->at(stageNo)->BeginCycle[row]   = m_runMethod->at(stageNo)->BeginCycle[row-deta];
        m_runMethod->at(stageNo)->Delta[row]        = m_runMethod->at(stageNo)->Delta[row-deta];
        m_runMethod->at(stageNo)->Temp[row-deta]         = Temp;
        m_runMethod->at(stageNo)->Ramp[row-deta]         = Ramp;
        m_runMethod->at(stageNo)->Time[row-deta]         = Time;
        m_runMethod->at(stageNo)->SubProperty[row-deta]  = SubProperty;
        m_runMethod->at(stageNo)->ReadInterval[row-deta] = ReadInterval;
        m_runMethod->at(stageNo)->ReadFluor[row-deta]    = ReadFluor;
        m_runMethod->at(stageNo)->ReadMode[row-deta]     = ReadMode;
        m_runMethod->at(stageNo)->TarValue[row-deta]     = TarValue;
        m_runMethod->at(stageNo)->BeginCycle[row-deta]   = BeginCycle;
        m_runMethod->at(stageNo)->Delta[row-deta]        = Delta;
    }

    m_d->stepModel->insertRow(row-deta, items);
    ui->tableViewStep->selectRow(row-deta);

    if(hasInfinite){
        refreshStepDescribe(stageNo,row-1);
    }

    emit operatorLog(tr("Move step up"));

    qDebug() << Q_FUNC_INFO << "Special index:" << m_d->specialIndex;

    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,row-1);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,row-1);
        m_d->plotStageAdd->replot();
    }
}

void GRunEditor::on_buttonStepAdd_clicked()
{
    m_pool->screen_sound();
    qDebug() << "add new step";

    qDebug() << "tlpd checkup 15";
    if(!initCheckup(true)) return;

    int stageNo = 0;
    if(!currentStageIndex(stageNo)) return;

    if(m_runMethod->at(stageNo)->SubNum >= MAX_STEP){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Step outside range(1~%1)").arg(MAX_STEP));
        return;
    }

    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
    int currentRow = index.row();

    m_d->stepModel->appendRow(new QStandardItem);
    int row = m_d->stepModel->rowCount() - 1;

    int readFluor = 0;
    m_runMethod->at(stageNo)->SubProperty[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->Temp[m_runMethod->at(stageNo)->SubNum] = m_runMethod->at(stageNo)->Temp[currentRow];
    m_runMethod->at(stageNo)->Time[m_runMethod->at(stageNo)->SubNum] = m_runMethod->at(stageNo)->Time[currentRow];
    m_runMethod->at(stageNo)->ReadInterval[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->ReadFluor[m_runMethod->at(stageNo)->SubNum] = m_runMethod->at(stageNo)->ReadFluor[currentRow];
    m_runMethod->at(stageNo)->ReadMode[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->Ramp[m_runMethod->at(stageNo)->SubNum] = m_runMethod->at(stageNo)->Ramp[currentRow];
    m_runMethod->at(stageNo)->BeginCycle[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->Delta[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->TarValue[m_runMethod->at(stageNo)->SubNum] = 0;
    m_runMethod->at(stageNo)->SubNum++;

    if(stageNo==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0){
        int stepNo = m_runMethod->last()->SubNum-2;
        if(stepNo>=0 && m_runMethod->last()->Time[stepNo]==0){
            m_runMethod->last()->Time[stepNo] = 30;
            refreshStepDescribe(stageNo,stepNo);
        }
    }

    m_d->stepModel->setData(m_d->stepModel->index(row,0), trUtf8("%1℃").arg(QString::number(m_runMethod->at(stageNo)->Temp[currentRow],'f',1)), Qt::DisplayRole);

    int minutes = m_runMethod->at(stageNo)->Time[row] / 60;
    int seconds = m_runMethod->at(stageNo)->Time[row] % 60;
    QString txt = (minutes==0 && seconds==0) ? "∞" : tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true));
    m_d->stepModel->setData(m_d->stepModel->index(row,1), txt, Qt::DisplayRole);
    m_d->stepModel->setData(m_d->stepModel->index(row,2), readFluor, Qt::DisplayRole);
    m_d->stepModel->setData(m_d->stepModel->index(row,4), QVariant::fromValue(0), Qt::EditRole);
    ui->tableViewStep->viewport()->update();
    ui->tableViewStep->selectRow(row);

    //修改Stage描述为自定义,熔解时不变
    if(m_runMethod->at(stageNo)->Property == 0){
        QString curStageName = tr("Custom Stage");
        memset((void*)m_runMethod->at(stageNo)->Name,'\0', 64);
        memcpy((void*)m_runMethod->at(stageNo)->Name, (const void*)(curStageName.toUtf8().data()), curStageName.toUtf8().size());
        m_d->stageModel->setData(m_d->stageModel->index(stageNo,0),curStageName,Qt::DisplayRole);
    }

    emit operatorLog(tr("Add step"));
    qDebug() << Q_FUNC_INFO << "Special index:" << m_d->specialIndex;

    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,row);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,row);
        m_d->plotStageAdd->replot();
    }
}

void GRunEditor::on_buttonStepReduce_clicked()
{
    if(this->property("Step_Status").toBool()) return;
    this->setProperty("Step_Status", true);

    m_pool->screen_sound();
    qDebug() << "reduce selected step";

    qDebug() << "tlpd checkup 17";
    if(!initCheckup(true)){
        this->setProperty("Step_Status", false);
        return;
    }
    int stageNo = 0;
    if(!currentStageIndex(stageNo)){
        this->setProperty("Step_Status", false);
        return;
    }
    if(!ui->tableViewStep->selectionModel()->hasSelection()){
        this->setProperty("Step_Status", false);
        return;
    }
    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
    int row = index.row();

    //如果是熔解时
    if(m_runMethod->at(stageNo)->Property != 0){
        //属性为SP_PR_Melt的step不能删除，否则删除整个stage,或者step的数量少于2时删除整个stage
        if(m_runMethod->at(stageNo)->SubNum <= 2 || row == m_d->specialIndex){
            My_MessageBox mb;
            if(mb.gquestion(m_pool, NULL, tr("Inquiry"), tr("The whole melt stage will be remove, are you sure to delete?")) == 0){
                m_d->stageModel->removeRow(stageNo);
                _STAGE_INFO *stage = m_runMethod->takeAt(stageNo);
                delete stage;
                if(m_pool->fileOpType == 1) emit sampleChanged();
                saveMethodFile();
                emit operatorLog(tr("Delete stage"));
            }
            if(m_d->item){
                int selectedRow = -1;
                if(ui->tableViewStep->selectionModel()->hasSelection()){
                    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
                    selectedRow = index.row();
                }
                m_d->item->currentSelect(m_d->selectedStageRow,selectedRow);
                m_d->plot->replot();
            }

            this->setProperty("Step_Status", false);
            return;
        }
    }else{
        //如果删除最后一个step,将删除整个stage
        if(m_runMethod->at(stageNo)->SubNum <= 1){
            m_d->stageModel->removeRow(stageNo);
            _STAGE_INFO *stage = m_runMethod->takeAt(stageNo);
            delete stage;

            if(m_pool->fileOpType == 1) emit sampleChanged();
            saveMethodFile();
            emit operatorLog(tr("Delete stage"));

            if(m_d->item){
                int selectedRow = -1;
                if(ui->tableViewStep->selectionModel()->hasSelection()){
                    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
                    selectedRow = index.row();
                }
                m_d->item->currentSelect(m_d->selectedStageRow,selectedRow);
                m_d->plot->replot();
            }

            this->setProperty("Step_Status", false);
            return;
        }
    }
    if(row < m_d->specialIndex){
        m_d->specialIndex --;
    }else if(row == m_d->specialIndex){
        m_d->specialIndex = -1;
    }
    m_runMethod->at(stageNo)->SubNum--;
    for(int i=row-1; i<m_runMethod->at(stageNo)->SubNum; ++i){
        m_runMethod->at(stageNo)->Temp[i]         = m_runMethod->at(stageNo)->Temp[i+1];
        m_runMethod->at(stageNo)->Ramp[i]         = m_runMethod->at(stageNo)->Ramp[i+1];
        m_runMethod->at(stageNo)->Time[i]         = m_runMethod->at(stageNo)->Time[i+1];
        m_runMethod->at(stageNo)->SubProperty[i]  = m_runMethod->at(stageNo)->SubProperty[i+1];
        m_runMethod->at(stageNo)->ReadInterval[i] = m_runMethod->at(stageNo)->ReadInterval[i+1];
        m_runMethod->at(stageNo)->ReadFluor[i]    = m_runMethod->at(stageNo)->ReadFluor[i+1];
        m_runMethod->at(stageNo)->ReadMode[i]     = m_runMethod->at(stageNo)->ReadMode[i+1];
        m_runMethod->at(stageNo)->TarValue[i]     = m_runMethod->at(stageNo)->TarValue[i+1];
        m_runMethod->at(stageNo)->BeginCycle[i]   = m_runMethod->at(stageNo)->BeginCycle[i+1];
        m_runMethod->at(stageNo)->Delta[i]        = m_runMethod->at(stageNo)->Delta[i+1];
    }

    m_d->stepModel->removeRow(row);
    //修改Stage描述为自定义,熔解时不变
    if(m_runMethod->at(stageNo)->Property == 0){
        QString curStageName = tr("Custom Stage");
        memset((void*)m_runMethod->at(stageNo)->Name,'\0', 64);
        memcpy((void*)m_runMethod->at(stageNo)->Name, (const void*)(curStageName.toUtf8().data()), curStageName.toUtf8().size());
        m_d->stageModel->setData(m_d->stageModel->index(stageNo,0),curStageName,Qt::DisplayRole);
    }

    emit operatorLog(tr("Delete Step"));
    qDebug() << Q_FUNC_INFO << "Special index:" << m_d->specialIndex;

    if(m_pool->fileOpType == 1) emit sampleChanged();
    saveMethodFile();

    if(m_d->item){
        int selectedRow = -1;
        if(ui->tableViewStep->selectionModel()->hasSelection()){
            QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
            selectedRow = index.row();
        }

        m_d->item->currentSelect(m_d->selectedStageRow,selectedRow);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        int selectedRow = -1;
        if(ui->tableViewStep->selectionModel()->hasSelection()){
            QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
            selectedRow = index.row();
        }

        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,selectedRow);
        m_d->plotStageAdd->replot();
    }

    this->setProperty("Step_Status", false);
}

void GRunEditor::on_buttonStepDown_clicked()
{
    if(this->property("Step_Status").toBool()) return;
    this->setProperty("Step_Status", true);

    m_pool->screen_sound();
    qDebug() << "selected step down";

    qDebug() << "tlpd checkup 18";
    if(!initCheckup(true)){
        this->setProperty("Step_Status", false);
        return;
    }

    int stageNo = 0;
    if(!currentStageIndex(stageNo)){
        this->setProperty("Step_Status", false);
        return;
    }
    if(!ui->tableViewStep->selectionModel()->hasSelection()){
        this->setProperty("Step_Status", false);
        return;
    }

    QModelIndex index = ui->tableViewStep->selectionModel()->selectedRows().first();
    int row = index.row();

    if(row >= m_d->stepModel->rowCount()-1){
        this->setProperty("Step_Status", false);
        return;
    }

    if(row == m_d->specialIndex){
        m_d->specialIndex++;
    }else if(row < m_d->specialIndex){
        if(row+1 == m_d->specialIndex){
            m_d->specialIndex--;
        }
    }

    bool hasInfinite = false;
    if(stageNo==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 \
            && row==m_runMethod->last()->SubNum-2 && m_runMethod->last()->Time[m_d->specialIndex]==0){
        m_runMethod->last()->Time[m_d->specialIndex] = 30;
        hasInfinite = true;
    }

    QList<QStandardItem *> items = m_d->stepModel->takeRow(row);

    int deta = 1;

    m_d->stepModel->insertRow(row+deta, items);
    ui->tableViewStep->selectRow(row+deta);

    if(hasInfinite){
        refreshStepDescribe(stageNo,row);
    }

    emit operatorLog(tr("Move step down"));
    qDebug() << Q_FUNC_INFO << "Special index:" << m_d->specialIndex;

    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,row+1);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,row+1);
        m_d->plotStageAdd->replot();
    }
    this->setProperty("Step_Status", false);
}

void GRunEditor::on_buttonStageAddOk_clicked()
{
    int metnum=0;
    qDebug() << "tableViewStageAdd viewport size:" << ui->tableViewStageAdd->viewport()->size();

    m_pool->screen_sound();
    if(m_d->stageModel->rowCount() != m_runMethod->count()) return;
    if(ui->tableViewStageAdd->selectionModel()->selectedIndexes().count() == 0) return;

    if(m_runMethod->count() >= MAX_STAGE){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Stage outside range(1~%1)").arg(MAX_STAGE));
        return;
    }

    QModelIndex index = ui->tableViewStageAdd->selectionModel()->selectedIndexes().first();

    int _type = index.row() * STAGEADD_COLUMN_COUNT + index.column();
    if(_type > 7) return;

#ifndef DEVICE_TYPE_TL13
    if(_type == 4 || _type == 5){
        //一个实验只能有一个熔解
        //        bool hasMelt = false;
        for(int i=0; i<m_d->stageModel->rowCount(); i++){
            int type = m_d->stageModel->data(m_d->stageModel->index(i,2),Qt::EditRole).toInt();
            if(type == 4 || type == 5){
                //                hasMelt = true;
                metnum++;
                //#if (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
                //                break;
                //#endif
            }
        }
        //#if (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
        //        if(hasMelt){
        //            My_MessageBox mb;
        //            mb.gwarning(m_pool, NULL, tr("Warning"), tr("Only one melting stage is allowed in the temperature program."));
        //            return;
        //        }
        //#else
        qDebug()<<"-------------metnum   is   =--------------"<<metnum;
        if(metnum>4){
            My_MessageBox mb;
            mb.gwarning(m_pool, NULL, tr("Warning"), tr("Only five melting stages are allowed in the temperature program."));
            return;
        }
        //#endif
    }
#endif
    //#if (defined DEVICE_TYPE_TL23) || (defined DEVICE_TYPE_TL12)
    //    else  if(_type == 2 || _type == 3){
    //        //扩增中只能有一个stage可以采样
    //        for(int i=0; i<m_runMethod->count(); i++){
    //            if(m_runMethod->at(i) == NULL) continue;
    //            _STAGE_INFO *stageInfo = m_runMethod->at(i);
    //            if(stageInfo->Property != 0) continue;
    //            for(int j=0; j<stageInfo->SubNum; j++){
    //                if(stageInfo->Steps.at(j) == NULL) continue;
    //                stageInfo->ReadFluor[j] = 0;
    //            }
    //        }
    //    }
    //#else
    else {
        int ampstagenum =0;
        int ampstepnum =0;
        if(_type == 2 || _type == 3){
            ampstagenum ++;
            ampstepnum ++;
        }
        //扩增中只能有一个stage可以采样
        for(int i=0; i<m_runMethod->count(); i++){
            bool readfluor =false;
#ifndef DEVICE_TYPE_TL13
            if(_type !=  4 || _type != 5){
                //                if(m_runMethod->at(i) == NULL) continue;
                _STAGE_INFO *stageInfo = m_runMethod->at(i);
                if(stageInfo->Property != 0) continue;
                for(int j=0; j<stageInfo->SubNum; j++){
                    //                    if(stageInfo->Steps.at(j) == NULL) continue;
                    if( stageInfo->ReadFluor[j]==1){
                        ampstepnum++;
                        readfluor =true;
                    }
                }
                if(readfluor ==true)
                    ampstagenum++;
            }
#endif
            if(ampstagenum>5){
                for(int i=0; i<m_runMethod->count(); i++){
                    bool stepreadfluor = false;
                    _STAGE_INFO *stageInfo = m_runMethod->at(i);
                    if(stageInfo->Property != 0) continue;
                    for(int j=0; j<stageInfo->SubNum; j++){
                        if(stageInfo->ReadFluor[j] == 1){
                            stageInfo->ReadFluor[j] = 0;
                            stepreadfluor=true;
                            ampstagenum--;
                        }
                    }
                    if(stepreadfluor==true)   break;
                }
            }
        }
    }
    //#endif
    QString text = m_d->stageAddModel->data(index, Qt::DisplayRole).toString().trimmed();
    QString stageName = text;

    int cycleCount = (_type == 2 || _type == 3) ? 40 : 1;

    _STAGE_INFO *stage = new _STAGE_INFO();
    if(stage){
        int len = stageName.toUtf8().size();
        len = len < 64 ? len : 64;

        int row = 0;
        int repeat = 0;
        while(row < m_d->stageModel->rowCount()){
            if(repeat != 0){
                stageName = text + tr("(%1)").arg(repeat);
                len = stageName.toUtf8().size() < 64 ? stageName.toUtf8().size() : 64;
            }
            if(m_d->stageModel->data(m_d->stageModel->index(row,0),Qt::DisplayRole).toString().trimmed() == stageName.left(len)){
                row = 0;
                repeat ++;
            }else
                row ++;
        }

        memcpy((void*)stage->Name, (const void*)(stageName.toUtf8().data()), len);
        stage->Cycles = cycleCount;
        stage->Property = (_type == 4)?1:((_type == 5)?2:0);

        switch(_type)
        {
        case 0:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9500;
                stage->Time[stage->SubNum] = 180;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
        case 1:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 4200;
                stage->Time[stage->SubNum] = 1800;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;

                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 8500;
                stage->Time[stage->SubNum] = 300;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
        case 2:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9400;
                stage->Time[stage->SubNum] = 10;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;

                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 5800;
                stage->Time[stage->SubNum] = 30;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 1;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
        case 3:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9400;
                stage->Time[stage->SubNum] = 15;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;

                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 5500;
                stage->Time[stage->SubNum] = 10;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;

                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 7200;
                stage->Time[stage->SubNum] = 30;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 1;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
#ifndef DEVICE_TYPE_TL13
        case 4:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9500;
                stage->Time[stage->SubNum] = 60;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 6000;
                stage->Time[stage->SubNum] = 15;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
                stage->SubProperty[stage->SubNum] = 4;
                stage->Temp[stage->SubNum] = 9800;
                stage->Time[stage->SubNum] = 5;
                stage->ReadInterval[stage->SubNum] = 50;
                stage->ReadFluor[stage->SubNum] = 1;
                stage->ReadMode[stage->SubNum] = 1;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
                m_d->isSingleStep = true;
            }
            break;
        }
        case 5:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9500;
                stage->Time[stage->SubNum] = 60;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 4000;
                stage->Time[stage->SubNum] = 60;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 6500;
                stage->Time[stage->SubNum] = 1;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            {
                stage->SubProperty[stage->SubNum] = 4;
                stage->Temp[stage->SubNum] = 9700;
                stage->Time[stage->SubNum] = 1;
                stage->ReadInterval[stage->SubNum] = 10;
                stage->ReadFluor[stage->SubNum] = 1;
                stage->ReadMode[stage->SubNum] = 2;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;

                m_d->isSingleStep = false;
            }
            break;
        }
#endif
        case 6:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 3500;
                stage->Time[stage->SubNum] = 30;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
        case 7:{
            {
                stage->SubProperty[stage->SubNum] = 0;
                stage->Temp[stage->SubNum] = 9400;
                stage->Time[stage->SubNum] = 10;
                stage->ReadInterval[stage->SubNum] = 0;
                stage->ReadFluor[stage->SubNum] = 0;
                stage->ReadMode[stage->SubNum] = 0;
                stage->Ramp[stage->SubNum] = m_pool->maxSpeed[0]*100;
                stage->BeginCycle[stage->SubNum] = 0;
                stage->Delta[stage->SubNum] = 0;
                stage->TarValue[stage->SubNum] = 0;
                stage->SubNum++;
            }
            break;
        }
        default:break;
        }

        m_runMethod->append(stage);

        if(m_runMethod->count()>1){
            int stageNo = m_runMethod->count()-2;
            if(m_runMethod->at(stageNo)->Cycles==1 && m_runMethod->at(stageNo)->Property==0 && m_runMethod->at(stageNo)->Time[m_d->specialIndex]==0){
                m_runMethod->at(stageNo)->Time[m_d->specialIndex] = 30;
            }
        }
    }

    m_d->stageModel->appendRow(new QStandardItem);
    int row = m_d->stageModel->rowCount() - 1;
    m_d->selectedStageRow = row;
    m_d->stageModel->setData(m_d->stageModel->index(row,0), stageName, Qt::DisplayRole);
    m_d->stageModel->setData(m_d->stageModel->index(row,1), cycleCount, Qt::DisplayRole);
    m_d->stageModel->setData(m_d->stageModel->index(row,2), _type, Qt::EditRole);
    ui->tableViewStage->clearSelection();
    ui->tableViewStage->selectRow(row);

    //    emit methodChanged();

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,0);
        m_d->plotStageAdd->replot();

    }
}

void GRunEditor::on_buttonStageAddBack_clicked()
{
    m_pool->screen_sound();
#ifndef DEVICE_TYPE_TL13
    ui->RunEditor->setTabEnabled(1, true);
#endif

    emit editting(false);
    ui->stackedWidget->setCurrentIndex(0);

    qDebug() << "save method file 18";

    if(m_pool->fileOpType == 1) emit sampleChanged();
    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,0);
        m_d->plot->replot();
    }
}

void GRunEditor::on_buttonStepEditAmpBack_clicked()
{
    m_pool->screen_sound();
    //判断Gradient,Touchdown与Long模式下设置值有没有冲突
    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL)){
        if(ui->radioButtonStepGradient->isChecked()){
            double center = ui->lineEditStepCenter->text().toDouble();
            double offset = ui->lineEditStepOffset->text().toDouble();

            if(center-offset < MIN_GRADIENT_TEMP){
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("The gradient temperature is below %1 %2").arg(MIN_GRADIENT_TEMP).arg(trUtf8("℃")));
                return;
            }else if(center+offset > MAX_GRADIENT_TEMP){
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("The gradient temperature is higher than %1 %2").arg(MAX_GRADIENT_TEMP).arg(trUtf8("℃")));
                return;
            }

            int val = center * 100.0;
            if(m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] != val)
                m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = val;
        }else{
            if(ui->radioButtonStepTouchdown->isChecked()){
                int dd = ui->lineEditStepDelta->text().toDouble() * 100;

                int s1 = ui->lineEditStepTarget->text().toDouble() * 100;
                int s2 = ui->lineEditStepTarValue->text().toDouble() * 100;
                int val = ui->lineEditStepBeginCycle->text().toInt();

                if(s1 == s2){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("The target temperature is equal to the initial temperature."));
                }

                if(qAbs(s1-s2) > dd*(m_runMethod->at(m_d->selectedStageRow)->Cycles-val)){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Cannot reach the target temperature within cycles."));
                }
            }else if(ui->radioButtonStepLong->isChecked()){
                int minutes = 0, seconds = 0;
                QRegExp rx("(\\d*):(\\d*)");
                int pos = rx.indexIn(ui->lineEditStepDelta->text());
                if (pos > -1) {
                    minutes = rx.cap(1).toInt();
                    seconds = rx.cap(2).toInt();
                }

                int dd = minutes * 60 + seconds;

                minutes = 0;
                seconds = 0;
                pos = rx.indexIn(ui->lineEditStepDuration->text());
                if (pos > -1) {
                    minutes = rx.cap(1).toInt();
                    seconds = rx.cap(2).toInt();
                }

                int s1 = minutes * 60 + seconds;

                minutes = 0;
                seconds = 0;
                pos = rx.indexIn(ui->lineEditStepTarValue->text());
                if (pos > -1) {
                    minutes = rx.cap(1).toInt();
                    seconds = rx.cap(2).toInt();
                }
                int s2 = minutes * 60 + seconds;

                int val = ui->lineEditStepBeginCycle->text().toInt();

                if(s1 == s2){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("The target time is equal to the initial time."));
                }
                if(qAbs(s1-s2) > dd*(m_runMethod->at(m_d->selectedStageRow)->Cycles-val)){
                    My_MessageBox mb;
                    mb.gwarning(m_pool, NULL, tr("Warning"), tr("Cannot reach the target time within cycles."));
                }
            }
        }
    }

    //刷新step显示
    refreshStepDescribe(m_d->selectedStageRow, m_d->selectedStepRow);
#ifdef DEVICE_TYPE_TL22
    ui->RunEditor->setTabEnabled(1, true);
#endif
    emit editting(false);
    ui->stackedWidget->setCurrentIndex(0);

    qDebug() << "save method file 20";
    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,m_d->selectedStepRow);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,m_d->selectedStepRow);
        m_d->plotStageAdd->replot();
    }    
}

void GRunEditor::on_buttonStepEditMeltBack_clicked()
{
    m_pool->screen_sound();

    //判断熔解设置是否合适
    if(m_d->selectedStageRow>=0 && m_d->selectedStepRow>=0 && m_d->selectedStepRow<m_runMethod->at(m_d->selectedStageRow)->SubNum){
        bool isCompare = false;
        int temp = 0;
        if(m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 4){
            temp = qAbs(m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] - m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->selectedStepRow-1]);
            isCompare = true;
        }else if((m_d->selectedStepRow<m_runMethod->at(m_d->selectedStageRow)->SubNum-1) \
                 && (m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->selectedStepRow+1] == 4)){
            temp = qAbs(m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] - m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->selectedStepRow+1]);
            isCompare = true;
        }

        if(isCompare){
            if(temp == 0){
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("The melting initial temperature is equal to its final temperature."));
                return;
            }else if(temp < 300){
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("The melting temperature difference is too small"));
            }

            if(m_d->isSingleStep){
                //如果是单步熔解
                temp /= m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex];
            }else{
                //如果时连续熔解
                temp *= m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex];
                temp /= 100;
            }

            if(temp < 5){
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("Fluorescence reading times are too few."));
            }
        }
    }

    //刷新step显示
    if(m_d->selectedStageRow >= 0 && m_d->selectedStepRow >= 0){
        if(m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 0){
            QString txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex],'f',1));
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,0), txt, Qt::DisplayRole);
            int minutes = m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex] / 60;
            int seconds = m_runMethod->at(m_d->selectedStageRow)->Time[m_d->specialIndex] % 60;
            txt = (minutes==0 && seconds==0) ? "∞" : tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true));
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,1), txt, Qt::DisplayRole);
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,4), QVariant::fromValue(0), Qt::EditRole);
        }else if(m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 4){
            QString modeStr;
            switch(m_runMethod->at(m_d->selectedStageRow)->ReadMode[m_d->specialIndex]){
            case 1:
                modeStr = tr("%1℃").arg(QString::number(m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex],'f',1));
                m_d->isSingleStep=true;
                break;
            case 2:
                modeStr = tr("%1 Readings/℃").arg(QString::number(m_runMethod->at(m_d->selectedStageRow)->ReadInterval[m_d->specialIndex]));
                m_d->isSingleStep=false;
                break;
            default:
                My_MessageBox mb;
                mb.gwarning(m_pool, NULL, tr("Warning"), tr("Inaccurate reading mode setting"));
                return;
            }

            QString txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex],'f',1));
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,0), txt, Qt::DisplayRole);
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,1), modeStr, Qt::DisplayRole);
            m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,4), QVariant::fromValue(4), Qt::EditRole);
        }
        m_d->stepModel->setData(m_d->stepModel->index(m_d->selectedStepRow,2), m_runMethod->at(m_d->selectedStageRow)->ReadFluor[m_d->specialIndex], Qt::DisplayRole);
    }
#ifdef DEVICE_TYPE_TL22
    ui->RunEditor->setTabEnabled(1, true);
#endif
    emit editting(false);
    ui->stackedWidget->setCurrentIndex(0);

    qDebug() << "save method file 21";
    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(m_d->selectedStageRow,m_d->selectedStepRow);
        m_d->plot->replot();
    }

    if(m_d->itemStageAdd){
        m_d->itemStageAdd->currentSelect(m_d->selectedStageRow,m_d->selectedStepRow);
        m_d->plotStageAdd->replot();
    }

    this->clearFocus();
}

void GRunEditor::slot_stage_insertOrRemoved()
{
    //    qDebug() << "slot_stage_insertOrRemoved";
    //如果运行中,不能打开按键
    _PCR_RUN_CTRL_INFO pcrInfo;

    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    //改变按键状态
    bool isEnabled = (pcrInfo.State.ExpState==0) && (m_d->stageModel->rowCount()>0);
    if(ui->buttonStageUp->isEnabled() != isEnabled){
        ui->buttonStageUp->setEnabled(isEnabled);
        ui->buttonStageReduce->setEnabled(isEnabled);
        ui->buttonStageDown->setEnabled(isEnabled);

        if(GHelper::total_instrument_id != 6){
            ui->buttonStepUp->setEnabled(isEnabled);
            ui->buttonStepAdd->setEnabled(isEnabled);
            ui->buttonStepReduce->setEnabled(isEnabled);
            ui->buttonStepDown->setEnabled(isEnabled);
        }
    }
    //    qDebug() << "slot_stage_insertOrRemoved end";
}

void GRunEditor::on_radioButtonStepStd_toggled(bool checked)
{
    if(!checked) return;
    m_pool->screen_sound();

    if(m_d->specialIndex >= 0){
        if(m_d->specialIndex == m_d->selectedStepRow)
            m_d->specialIndex = -1;
    }


    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){

        m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 0;

        m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex] = 0;
        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = 0;
        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = 0;
    }
    ui->labelTarget->setText(tr("Temperature:"));
    ui->labelDuration->setText(tr("Time:"));

    ui->labelTarget->setVisible(true);
    ui->lineEditStepTarget->setVisible(true);
    ui->labelTargetUnit->setVisible(true);
    ui->stepGradient->setVisible(false);
    ui->stepTouchdown->setVisible(false);
}

void GRunEditor::on_radioButtonStepGradient_toggled(bool checked)
{
    if(!checked) return;
    m_pool->screen_sound();

    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){

        if(m_d->specialIndex >= 0 && m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 0){
            My_MessageBox mb;
            if(mb.gquestion(m_pool, NULL, tr("Inquiry"), tr("Nonstandard mode already exists, do you want to change?")) == 1){
                ui->radioButtonStepGradient->blockSignals(true);
                ui->radioButtonStepGradient->setChecked(false);
                ui->radioButtonStepGradient->blockSignals(false);

                //////////////////////////////////////////////////////////////
                //  ui->radioButtonStepGradient->setEnabled(false);
                //得到原来的设置指针
                QRadioButton *radioButton = NULL;
                switch(m_runMethod->at(m_d->selectedStageRow)->Property){
                case 0:radioButton = ui->radioButtonStepStd;break;
                case 3:radioButton = ui->radioButtonStepGradient;break;
                case 1:radioButton = ui->radioButtonStepTouchdown;break;
                case 2:radioButton = ui->radioButtonStepLong;break;
                default:break;
                }
                if(radioButton){
                    radioButton->blockSignals(true);
                    radioButton->setChecked(true);
                    radioButton->blockSignals(false);
                }
                return;
            }
        }

        m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 3;

        m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex] = 0;
        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = 0;
        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = 0;
        double centerVal = m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex];

        if(centerVal < MIN_CENTER_TEMP){
            centerVal = MIN_CENTER_TEMP;
            m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = MIN_CENTER_TEMP;
        }else if(centerVal > MAX_CENTER_TEMP){
            centerVal = MAX_CENTER_TEMP;
            m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = MAX_CENTER_TEMP;
        }

        ui->lineEditStepCenter->setText(QString::number(centerVal,'f',1));
        ui->lineEditStepTarget->setText(QString::number(centerVal,'f',1));
        ui->lineEditStepOffset->setText("5.0");

        if((m_d->selectedStageRow==m_runMethod->count()-1) && (m_runMethod->at(m_d->selectedStageRow)->Cycles==1) \
                && (m_d->selectedStepRow==m_runMethod->last()->SubNum-1) && (m_runMethod->last()->Time[m_d->specialIndex]==0)){
            m_runMethod->last()->Time[m_d->specialIndex] = 30;
            int minutes = 0;
            int seconds = 30;
            ui->lineEditStepDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));

        }

        if(m_d->specialIndex >= 0 && (m_d->specialIndex != m_d->selectedStepRow) \
                && (m_d->specialIndex < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
            m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 0;
            refreshStepDescribe(m_d->selectedStageRow, m_d->specialIndex);
        }
    }
    m_d->specialIndex = m_d->selectedStepRow;

    ui->labelTarget->setText(tr("Temperature:"));
    ui->labelDuration->setText(tr("Time:"));

    ui->labelTarget->setVisible(false);
    ui->lineEditStepTarget->setVisible(false);
    ui->labelTargetUnit->setVisible(false);
    ui->stepGradient->setVisible(true);
    ui->stepTouchdown->setVisible(false);
}

void GRunEditor::on_radioButtonStepTouchdown_toggled(bool checked)
{
    if(!checked) return;
    m_pool->screen_sound();

    int tarvalue = 5000, delta = 100;

    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){

        if(m_d->specialIndex >= 0 && m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 0){
            My_MessageBox mb;
            if(mb.gquestion(m_pool, NULL, tr("Inquiry"), tr("Nonstandard mode already exists, do you want to change?")) == 1){
                ui->radioButtonStepTouchdown->blockSignals(true);
                ui->radioButtonStepTouchdown->setChecked(false);
                ui->radioButtonStepTouchdown->blockSignals(false);

                //得到原来的设置指针
                QRadioButton *radioButton = NULL;
                switch(m_runMethod->at(m_d->selectedStageRow)->Property){
                case 0:radioButton = ui->radioButtonStepStd;break;
                case 3:radioButton = ui->radioButtonStepGradient;break;
                case 1:radioButton = ui->radioButtonStepTouchdown;break;
                case 2:radioButton = ui->radioButtonStepLong;break;
                default:break;
                }
                if(radioButton){
                    radioButton->blockSignals(true);
                    radioButton->setChecked(true);
                    radioButton->blockSignals(false);
                }
                return;
            }
        }
        double temp = m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex];
        if(temp < MIN_TOUCHDOWN_TEMP){
            temp=MIN_TOUCHDOWN_TEMP;
            m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = MIN_TOUCHDOWN_TEMP;
        }else if(temp > MAX_TOUCHDOWN_TEMP){
            temp=MAX_TOUCHDOWN_TEMP;
            m_runMethod->at(m_d->selectedStageRow)->Temp[m_d->specialIndex] = MAX_TOUCHDOWN_TEMP;
        }
        ui->lineEditStepTarget->setText(QString::number(temp,'f',1));

        m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 1;

        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = 5000;
        tarvalue = m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex];

        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = 100;
        delta = m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex];

        m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex] = 1;

        ui->lineEditStepBeginCycle->setText(QString::number(m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex]));

        if((m_d->selectedStageRow==m_runMethod->count()-1) && (m_runMethod->at(m_d->selectedStageRow)->Cycles==1) \
                && (m_d->selectedStepRow==m_runMethod->last()->SubNum-1) && (m_runMethod->last()->Time[m_d->specialIndex]==0)){
            m_runMethod->last()->Time[m_d->specialIndex] = 30;
            int minutes = 0;
            int seconds = 30;
            ui->lineEditStepDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
        }

        if(m_d->specialIndex >= 0  && (m_d->specialIndex != m_d->selectedStepRow) \
                && (m_d->specialIndex < m_runMethod->at(m_d->selectedStageRow)->SubNum) ){
            m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 0;
            refreshStepDescribe(m_d->selectedStageRow, m_d->specialIndex);
        }
    }
    m_d->specialIndex = m_d->selectedStepRow;

    ui->lineEditStepTarValue->setText(QString::number(tarvalue/100.0,'f',1));
    ui->lineEditStepDelta->setText(QString::number(delta/100.0,'f',1));

    ui->labelTarget->setText(tr("Initial Temp.:"));
    ui->labelDuration->setText(tr("Time:"));
    ui->labelTargetValue->setText(tr("Target Temp.:"));
    //    ui->labelTarValueUnit->setText(trUtf8("℃"));
    ui->labelTarValueUnit->setVisible(true);
    ui->labelDelta->setText(tr("Delta Temp.:"));
    ui->labelDeltaUnit->setText(trUtf8("℃/Cycle"));

    ui->labelTarget->setVisible(true);
    ui->lineEditStepTarget->setVisible(true);
    ui->labelTargetUnit->setVisible(true);
    ui->stepGradient->setVisible(false);
    ui->stepTouchdown->setVisible(true);
}

void GRunEditor::on_radioButtonStepLong_toggled(bool checked)
{
    if(!checked) return;
    m_pool->screen_sound();

    int tarvalue = 120, delta = 5;

    if(m_runMethod && (m_d->selectedStageRow < m_runMethod->count()) && (m_runMethod->at(m_d->selectedStageRow) != NULL) \
            && (m_d->selectedStepRow < m_runMethod->at(m_d->selectedStageRow)->SubNum) \
            && (m_runMethod->at(m_d->selectedStageRow) != NULL)){

        if(m_d->specialIndex >= 0 && m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] == 0){
            My_MessageBox mb;
            if(mb.gquestion(m_pool, NULL, tr("Inquiry"), tr("Nonstandard mode already exists, do you want to change?")) == 1){
                ui->radioButtonStepLong->blockSignals(true);
                ui->radioButtonStepLong->setChecked(false);
                ui->radioButtonStepLong->blockSignals(false);

                //得到原来的设置指针
                QRadioButton *radioButton = NULL;
                switch(m_runMethod->at(m_d->selectedStageRow)->Property){
                case 0:radioButton = ui->radioButtonStepStd;break;
                case 3:radioButton = ui->radioButtonStepGradient;break;
                case 1:radioButton = ui->radioButtonStepTouchdown;break;
                case 2:radioButton = ui->radioButtonStepLong;break;
                default:break;
                }
                if(radioButton){
                    radioButton->blockSignals(true);
                    radioButton->setChecked(true);
                    radioButton->blockSignals(false);
                }
                return;
            }
        }

        m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 2;

        m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex] = 120;
        tarvalue = m_runMethod->at(m_d->selectedStageRow)->TarValue[m_d->specialIndex];

        m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex] = 5;
        delta = m_runMethod->at(m_d->selectedStageRow)->Delta[m_d->specialIndex];

        m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex] = 1;

        ui->lineEditStepBeginCycle->setText(QString::number(m_runMethod->at(m_d->selectedStageRow)->BeginCycle[m_d->specialIndex]));

        if((m_d->selectedStageRow==m_runMethod->count()-1) && (m_runMethod->at(m_d->selectedStageRow)->Cycles==1) \
                && (m_d->selectedStepRow==m_runMethod->last()->SubNum-1) && (m_runMethod->last()->Time[m_d->specialIndex]==0)){
            m_runMethod->last()->Time[m_d->specialIndex] = 30;
            int minutes = 0;
            int seconds = 30;
            ui->lineEditStepDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
        }

        if(m_d->specialIndex >= 0  && (m_d->specialIndex != m_d->selectedStepRow) \
                && (m_d->specialIndex < m_runMethod->at(m_d->selectedStageRow)->SubNum)){
            m_runMethod->at(m_d->selectedStageRow)->SubProperty[m_d->specialIndex] = 0;
            refreshStepDescribe(m_d->selectedStageRow, m_d->specialIndex);
        }
    }
    m_d->specialIndex = m_d->selectedStepRow;

    int minutes = tarvalue / 60;
    int seconds = tarvalue % 60;
    ui->lineEditStepTarValue->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
    minutes = delta / 60;
    seconds = delta % 60;
    ui->lineEditStepDelta->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));

    ui->labelTarget->setText(tr("Temperature:"));
    ui->labelDuration->setText(tr("Initial Time:"));
    ui->labelTargetValue->setText(tr("Target Time:"));
    //    ui->labelTarValueUnit->setText(trUtf8("s"));
    ui->labelTarValueUnit->setVisible(false);
    ui->labelDelta->setText(tr("Delta Time:"));
    ui->labelDeltaUnit->setText(tr("/Cycle"));

    ui->labelTarget->setVisible(true);
    ui->lineEditStepTarget->setVisible(true);
    ui->labelTargetUnit->setVisible(true);
    ui->stepGradient->setVisible(false);
    ui->stepTouchdown->setVisible(true);
}

void GRunEditor::on_buttonStepGradientDetail_clicked()
{
    m_pool->screen_sound();

    if(ui->lineEditStepCenter->text().isEmpty()){
        ui->lineEditStepCenter->setFocus();
        return;
    }

    if(ui->lineEditStepOffset->text().isEmpty()){
        ui->lineEditStepOffset->setFocus();
        return;
    }

    double cc = ui->lineEditStepCenter->text().toDouble();
    double oo = ui->lineEditStepOffset->text().toDouble();

    GGradientDetail detail(cc, oo, this);
    connect(&detail, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    detail.exec();
    this->clearFocus();
}

void GRunEditor::slot_buttonStepEdit_clicked()
{
    m_pool->screen_sound();
    qDebug() << "edit selected step";

    if(!initCheckup(true)) return;

    int stageNo = 0;
    if(!currentStageIndex(stageNo)) return;
    if(!ui->tableViewStep->selectionModel()->hasSelection()) return;

    QModelIndex stageIndex = ui->tableViewStage->selectionModel()->selectedRows().first();
    int stageRow = stageIndex.row();
    int type = m_d->stageModel->data(m_d->stageModel->index(stageRow,2), Qt::EditRole).toInt();

    QModelIndex stepIndex = ui->tableViewStep->selectionModel()->selectedRows().first();
    int stepRow = stepIndex.row();

    m_d->selectedStageRow = stageNo;
    m_d->selectedStepRow = stepRow;

    //设置数据
#ifndef DEVICE_TYPE_TL13
    if(type == 4 || type == 5){
        double temp = m_runMethod->at(stageNo)->Time[stepRow]/100.0;
        double minTemp = MIN_MELT_TEMP1;
        bool isMeltStep = m_runMethod->at(stageNo)->SubProperty[stepRow] == 4;
        if(isMeltStep || ((stepRow < m_runMethod->at(stageNo)->SubNum-1) \
                          && (m_runMethod->at(stageNo)->SubProperty[stepRow] == 4))){
            minTemp = MIN_MELT_TEMP2;
        }
        qDebug()<<"--------------m_d->isSingleStep------------------00000-------type--------------"<<type;

        if(type==4)m_d->isSingleStep=true;
        if(type==5)m_d->isSingleStep=false;

        if(temp < minTemp){
            temp = minTemp;
            m_runMethod->at(stageNo)->Time[stepRow] = temp * 100;
        }else if(temp > MAX_MELT_TEMP){
            temp = MAX_MELT_TEMP;
            m_runMethod->at(stageNo)->Time[stepRow] = temp * 100;
        }
        ui->lineEditMeltTarget->setText(QString::number(temp,'f',1));
        int minutes = m_runMethod->at(stageNo)->Time[stepRow] / 60;
        int seconds = m_runMethod->at(stageNo)->Time[stepRow] % 60;
        ui->lineEditMeltDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));

        bool isEnable = !(m_runMethod->at(stageNo)->SubProperty[stepRow] == 4 && m_runMethod->at(stageNo)->ReadMode[stepRow] == 2);
        ui->labelMeltDuration->setVisible(isEnable);
        ui->lineEditMeltDuration->setVisible(isEnable);
        //变温速率
        int z = 0;


        double ramp = m_runMethod->at(stageNo)->Ramp[stepRow];
        if(ramp < MIN_RAMP){
            ramp = MIN_RAMP;
        }else if(ramp > m_pool->maxSpeed[z]){
            ramp = m_pool->maxSpeed[z];
        }
        ui->lineEditMeltRamp->setText(QString::number(ramp,'f',1));

        ui->labelMeltRamp->setEnabled(!isMeltStep);
        ui->labelMeltRampUnit->setEnabled(!isMeltStep);

        if(type == 4){
            double val = m_runMethod->at(stageNo)->ReadInterval[stepRow]/100.0;
            if(val < MIN_SINGLESTEP_TEMP){
                val = MIN_SINGLESTEP_TEMP;
                m_runMethod->at(stageNo)->ReadInterval[stepRow] = (val * 100);
            }else if(val > MAX_SINGLESTEP_TEMP){
                val = MAX_SINGLESTEP_TEMP;
                m_runMethod->at(stageNo)->ReadInterval[stepRow] = (val * 100);
            }
            ui->lineEditMeltCon->setText(QString::number(val,'f',1));
        }else{
            int val = m_runMethod->at(stageNo)->ReadInterval[stepRow];
            if(val < MIN_CONTINUE_TIMES){
                val = MIN_CONTINUE_TIMES;
                m_runMethod->at(stageNo)->ReadInterval[stepRow] = val;
            }else if(val > MAX_CONTINUE_TIMES){
                val = MAX_CONTINUE_TIMES;
                m_runMethod->at(stageNo)->ReadInterval[stepRow] = val;
            }
            ui->lineEditMeltCon->setText(QString::number(val));
        }
        qDebug()<<"--------------m_d->isSingleStep------------------00000---------------------"<<m_d->isSingleStep;
        setShowDetail(m_runMethod->at(stageNo)->ReadMode[stepRow] == 0);

        ui->labelMeltUnit->setText(m_d->isSingleStep?trUtf8("℃"):tr("Readings/%1").arg(trUtf8("℃")));
        ui->labelMeltStep->setText(m_d->isSingleStep?tr("Increment:"):tr("Readings:"));

        bool hasMelt = false;
        //熔解段第一个step不能为熔解，全段只能有一个熔解step

        for(int i=0; i<m_d->stepModel->rowCount(); i++){
            if(i == stepRow) continue;
            int type = m_d->stepModel->data(m_d->stepModel->index(i,4),Qt::EditRole).toInt();
            if(type == 4 || type == 5){
                hasMelt = true;
                break;
            }
        }

        //为了屏蔽最后一个焦点导致的数据输入框的自动弹出
        ui->buttonStepEditMeltBack->setFocus();
    }else{
#endif
        double temp = m_runMethod->at(stageNo)->Time[stepRow]/100.0;
        if(m_runMethod->at(stageNo)->SubProperty[stepRow] == 1 && temp < MIN_TOUCHDOWN_TEMP){
            temp = MIN_TOUCHDOWN_TEMP;
            m_runMethod->at(stageNo)->Time[stepRow] = temp*100;
        }
        ui->lineEditStepTarget->setText(QString::number(temp,'f',1));
        int minutes = m_runMethod->at(stageNo)->Time[stepRow] / 60;
        int seconds = m_runMethod->at(stageNo)->Time[stepRow] % 60;
        if(minutes==0 && seconds==0){
            ui->lineEditStepDuration->setText("∞");
        }else{
            ui->lineEditStepDuration->setText(tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true)));
        }

        int z = 0;
        /*    if((stepRow > 0) && (stepRow < m_runMethod->at(stageNo)->SubNum)&&\
                (m_runMethod->at(stageNo)->Steps.at(stepRow-1) != NULL) && (m_runMethod->at(stageNo)->Steps.at(stepRow) != NULL) \
                && (m_runMethod->at(stageNo)->Steps.at(stepRow-1)->Temp > m_runMethod->at(stageRow)->Time[stepRow])){
            z = 1;
        }   */
        double ramp = m_runMethod->at(stageNo)->Ramp[stepRow];
        if(ramp < MIN_RAMP){
            ramp = MIN_RAMP;
        }else if(ramp > m_pool->maxSpeed[z]){
            ramp = m_pool->maxSpeed[z];
        }
        ui->lineEditStepRamp->setText(QString::number(ramp,'f',1));

        ui->radioButtonStepGradient->setChecked(m_runMethod->at(stageNo)->SubProperty[stepRow] == 3);
        ui->radioButtonStepTouchdown->setChecked(m_runMethod->at(stageNo)->SubProperty[stepRow] == 1);
        ui->radioButtonStepLong->setChecked(m_runMethod->at(stageNo)->SubProperty[stepRow] == 2);
        ui->radioButtonStepStd->setChecked(m_runMethod->at(stageNo)->SubProperty[stepRow] == 0);

        //为了屏蔽最后一个焦点导致的数据输入框的自动弹出
        ui->buttonStepEditAmpBack->setFocus();
#ifndef DEVICE_TYPE_TL13
    }
#endif

    //界面变化
#ifdef DEVICE_TYPE_TL22
    ui->RunEditor->setTabEnabled(1, false);
#endif
    emit editting(true);
#ifndef DEVICE_TYPE_TL13
    int page = (type == 4 || type == 5) ? 3 : 2;
#else
    int page =  2;
#endif
    ui->stackedWidget->setCurrentIndex(page);

    emit operatorLog(tr("Edit step"));

    qDebug() << "save method file 15";
    saveMethodFile();
}

void GRunEditor::slot_tableViewStage_selectionChanged()
{
    m_d->specialIndex = -1;
    m_d->stepModel->removeRows(0, m_d->stepModel->rowCount());

    if(!ui->tableViewStage->selectionModel()->hasSelection()) return;

    QModelIndex index = ui->tableViewStage->selectionModel()->selectedRows().first();
    int row = index.row();
    if(m_runMethod->at(row) == NULL) return;

    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

#ifndef DEVICE_TYPE_TL13
    int currentStageType = m_d->stageModel->data(m_d->stageModel->index(row,2),Qt::EditRole).toInt();
    bool isNotMelt =  (currentStageType!=4) && (currentStageType!=5);
#else
    bool isNotMelt = true;
#endif

    m_d->stepEditDelegate->setEnable((pcrInfo.State.ExpState==0) && m_pool->expFile->is_compressed_file==0);
    m_d->stepCheckDelegate->setEnable(isNotMelt && (pcrInfo.State.ExpState==0) && m_pool->expFile->is_compressed_file==0);

    int specialCount = 0;
    for(int i=0; i<m_runMethod->at(row)->SubNum; i++){
        m_d->stepModel->appendRow(new QStandardItem);
        m_d->stepModel->setData(m_d->stepModel->index(i,4), QVariant::fromValue(0), Qt::EditRole);

        QString txt;
        bool isSpecial = true;
        switch(m_runMethod->at(row)->SubProperty[i]){
        case 1:
            txt = trUtf8("%1»%2℃").arg(QString::number(m_runMethod->at(row)->Temp[i],'f',1)) \
                    .arg(QString::number(m_runMethod->at(row)->TarValue[i],'f',1));
            break;
        case 2:
            txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(row)->Temp[i],'f',1));
            break;
        case 3:{
            double deta = qAbs(m_runMethod->at(row)->Temp[i]-m_runMethod->at(row)->Temp[i]);
            txt = trUtf8("%1±%2℃").arg(QString::number(m_runMethod->at(row)->Temp[i],'f',1)) \
                    .arg(QString::number(deta/100.0,'f',1));

            break;}
        case 4:{
            txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(row)->Temp[i],'f',1));
            break;
        }
        default:
            isSpecial = false;
            txt = trUtf8("%1℃").arg(QString::number(m_runMethod->at(row)->Temp[i],'f',1));
            break;
        }
        m_d->stepModel->setData(m_d->stepModel->index(i,0), txt, Qt::DisplayRole);

        txt.clear();
        switch(m_runMethod->at(row)->SubProperty[i]){
        case 2:{
            int minute1 = m_runMethod->at(row)->Time[i] / 60;
            int second1 = m_runMethod->at(row)->Time[i] % 60;
            int minute2 = m_runMethod->at(row)->TarValue[i] / 60;
            int second2 = (int)m_runMethod->at(row)->TarValue[i] % 60;
            txt = tr("%1:%2»%3:%4").arg(QString::number(minute1).rightJustified(2,'0',true)) \
                    .arg(QString::number(second1).rightJustified(2,'0',true))
                    .arg(QString::number(minute2).rightJustified(2,'0',true))
                    .arg(QString::number(second2).rightJustified(2,'0',true));
            break;
        }
        case 4:{
            QString modeStr;
            switch(m_runMethod->at(row)->ReadMode[i]){
            case 1:
                modeStr = tr("%1℃").arg(QString::number(m_runMethod->at(row)->ReadInterval[i],'f',1));
                m_d->stepModel->setData(m_d->stepModel->index(i,4), QVariant::fromValue(4), Qt::EditRole);
                break;
            case 2:
                modeStr = tr("%1 Readings/℃").arg(QString::number(m_runMethod->at(row)->ReadInterval[i]));
                m_d->stepModel->setData(m_d->stepModel->index(i,4), QVariant::fromValue(5), Qt::EditRole);
                break;
            default:isSpecial = false;break;
            }
            txt = modeStr.isEmpty() ? tr("Wrong") : trUtf8("%1").arg(modeStr);
            break;
        }
        default:{
            int minutes = m_runMethod->at(row)->Time[i] / 60;
            int seconds = m_runMethod->at(row)->Time[i] % 60;
            if(minutes==0 && seconds==0){
                txt = "∞";
            }else{
                txt = tr("%1:%2").arg(QString::number(minutes).rightJustified(2,'0',true)).arg(QString::number(seconds).rightJustified(2,'0',true));
            }
            break;
        }
        }
        m_d->stepModel->setData(m_d->stepModel->index(i,1), txt, Qt::DisplayRole);
        int isSample = m_runMethod->at(row)->ReadFluor[m_d->specialIndex];
        m_d->stepModel->setData(m_d->stepModel->index(i,2), isSample, Qt::DisplayRole);

        //如果是梯度，Touchdown，Long或熔解中的Melt
        if(isSpecial){
            m_d->specialIndex = i;
            specialCount++;
        }
    }

    //如果多个梯度，Touchdown，Long或熔解中多个Melt
    if(specialCount > 1){
        My_MessageBox mb;
        mb.gwarning(m_pool, NULL, tr("Warning"), tr("Inaccurate step setting"));
        return;
    }

    if(m_d->stepModel->rowCount() > 0){
        ui->tableViewStep->selectRow(0);

        if(m_d->item){
            m_d->item->currentSelect(row,0);
            m_d->plot->replot();
        }
    }
}

void GRunEditor::slot_tableViewStep_selectionChanged()
{
    //    m_pool->screen_sound();

    if(!ui->tableViewStage->selectionModel()->hasSelection()) return;
    if(!ui->tableViewStep->selectionModel()->hasSelection()) return;
    ////
#ifndef DEVICE_TYPE_TL13    
    QModelIndex stageIndex = ui->tableViewStage->selectionModel()->selectedRows().first();
    int stageRow = stageIndex.row();

    QModelIndex stepIndex = ui->tableViewStep->selectionModel()->selectedRows().first();
    int stepRow = stepIndex.row();

    if(GHelper::total_instrument_id != 6){
        //如果运行中,不能打开按键
        _PCR_RUN_CTRL_INFO pcrInfo;

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        if(pcrInfo.State.ExpState==0){
            //更新熔解时编辑按键的有效与否
            bool isFlesh = false;
            int stageType = m_d->stageModel->data(m_d->stageModel->index(stageRow,2),Qt::EditRole).toInt();
            if(stageType==4 || stageType==5){
                int stepType = m_d->stepModel->data(m_d->stepModel->index(stepRow,4),Qt::EditRole).toInt();
                if (stepType==4 || stepType==5){
                    ui->buttonStepUp->setEnabled(false);
                    ui->buttonStepDown->setEnabled(false);
                    if(!ui->buttonStepAdd->isEnabled())
                        ui->buttonStepAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
                    if(!ui->buttonStepReduce->isEnabled())
                        ui->buttonStepReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);
                    isFlesh = true;
                }else if(stepRow<m_d->stepModel->rowCount()-1){
                    int nextStepType = m_d->stepModel->data(m_d->stepModel->index(stepRow+1,4),Qt::EditRole).toInt();
                    if(nextStepType==4 || nextStepType==5){
                        ui->buttonStepUp->setEnabled(false);
                        ui->buttonStepAdd->setEnabled(false);
                        ui->buttonStepReduce->setEnabled(false);
                        ui->buttonStepDown->setEnabled(false);
                        isFlesh = true;
                    }
                }
            }

            if(!isFlesh){
                if(!ui->buttonStepUp->isEnabled())
                    ui->buttonStepUp->setEnabled(m_pool->expFile->is_compressed_file == 0);
                if(!ui->buttonStepAdd->isEnabled())
                    ui->buttonStepAdd->setEnabled(m_pool->expFile->is_compressed_file == 0);
                if(!ui->buttonStepReduce->isEnabled())
                    ui->buttonStepReduce->setEnabled(m_pool->expFile->is_compressed_file == 0);
                if(!ui->buttonStepDown->isEnabled())
                    ui->buttonStepDown->setEnabled(m_pool->expFile->is_compressed_file == 0);
            }
        }
    }

    //更新绘图
    if(m_d->item){
        m_d->item->currentSelect(stageRow,stepRow);
        m_d->plot->replot();
    }
#endif
}

void GRunEditor::slot_itemEdited(int row, const QByteArray &probe)
{   
    qDebug() << Q_FUNC_INFO << "step1";
    if(!initCheckup()) return;
    m_pool->screen_sound();

    bool isSelect = m_d->channelModel->data(m_d->channelModel->index(row,0),Qt::EditRole).toInt() ;//!= 0;
    QString probeStr;
    for(int i=0; i<probe.size(); i++){
        probeStr += QString::number(probe.at(i));
        qDebug()<<"-------------probeStr---------------"<<probeStr<<probe.at(i);
        if(i < probe.size()-1) probeStr+=",";
    }

    int mutex = 0;
#ifdef DEVICE_TYPE_TL22
    if(GHelper::total_instrument_id==202 || GHelper::total_instrument_id==203 || GHelper::total_instrument_id==205){
        if(isSelect && (row==0 && probe.at(0)>11)) mutex = 1;
    }else{
        if(isSelect && (row==FRET_INDEX || (row==0 && probe.at(0)>11))) mutex = 1;
    }
#else
    if(isSelect && (row==FRET_INDEX || (row==0 && probe.at(0)>11))) mutex = 1;
#endif

    qDebug() << Q_FUNC_INFO << "Update Dye infos :" << row << isSelect << probeStr << mutex;

    if(isSelect){
#ifdef DEVICE_TYPE_TL22
        if(row == FRET_INDEX){
            if(GHelper::total_instrument_id!=202 && GHelper::total_instrument_id!=203 && GHelper::total_instrument_id!=205){
                for(int i=0; i<FRET_INDEX; i++){
                    bool select = m_d->channelModel->data(m_d->channelModel->index(i,0),Qt::EditRole).toInt() != 0;
                    if(select){
                        m_d->channelModel->setData(m_d->channelModel->index(i,0),0,Qt::EditRole);
                        QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(i,1),Qt::EditRole).toByteArray();
                        if(m_pool->fileOpType == 1) emit channelItemChanged(i, false, pp);
                        m_d->channelModel->setData(m_d->channelModel->index(i,1),QVariant(),Qt::EditRole);
                    }
                }
            }
        }else{
            if(row==0){
                if(probe.at(0)>11){
                    for(int i=1; i<GHelper::channel_count; i++){
                        bool select = m_d->channelModel->data(m_d->channelModel->index(i,0),Qt::EditRole).toInt() != 0;
                        if(select){
                            m_d->channelModel->setData(m_d->channelModel->index(i,0),0,Qt::EditRole);
                            QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(i,1),Qt::EditRole).toByteArray();
                            if(m_pool->fileOpType == 1) emit channelItemChanged(i, false, pp);
                            m_d->channelModel->setData(m_d->channelModel->index(i,1),QVariant(),Qt::EditRole);
                        }
                    }
                }else if(GHelper::total_instrument_id!=202 && GHelper::total_instrument_id!=203 && GHelper::total_instrument_id!=205 && GHelper::channel_count > FRET_INDEX){
                    bool select = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,0),Qt::EditRole).toInt() != 0;
                    if(select){
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,0),0,Qt::EditRole);
                        QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,1),Qt::EditRole).toByteArray();

                        if(m_pool->fileOpType == 1) emit channelItemChanged(FRET_INDEX, false, probe);
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,1),QVariant(),Qt::EditRole);
                    }
                }
            }else{
                bool select = m_d->channelModel->data(m_d->channelModel->index(0,0),Qt::EditRole).toInt() != 0;
                QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(0,1),Qt::EditRole).toByteArray();

                if(select && (pp.size()>1 || pp.at(0)>11)){
                    m_d->channelModel->setData(m_d->channelModel->index(0,0),0,Qt::EditRole);

                    if(m_pool->fileOpType == 1) emit channelItemChanged(0, false, pp);
                    m_d->channelModel->setData(m_d->channelModel->index(0,1),QVariant(),Qt::EditRole);
                }

                if(GHelper::total_instrument_id!=202 && GHelper::total_instrument_id!=203 && GHelper::total_instrument_id!=205 && GHelper::channel_count > FRET_INDEX){
                    select = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,0),Qt::EditRole).toInt() != 0;
                    if(select){
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,0),0,Qt::EditRole);
                        pp = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,1),Qt::EditRole).toByteArray();

                        if(m_pool->fileOpType == 1) emit channelItemChanged(FRET_INDEX, false, probe);
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,1),QVariant(),Qt::EditRole);
                    }
                }
            }
        }
#else
        if(row == FRET_INDEX){
            for(int i=0; i<FRET_INDEX; i++){
                bool select = m_d->channelModel->data(m_d->channelModel->index(i,0),Qt::EditRole).toInt() != 0;
                if(select){
                    m_d->channelModel->setData(m_d->channelModel->index(i,0),0,Qt::EditRole);
                    QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(i,1),Qt::EditRole).toByteArray();
                    if(m_pool->fileOpType == 1) emit channelItemChanged(i, false, pp);
                    m_d->channelModel->setData(m_d->channelModel->index(i,1),QVariant(),Qt::EditRole);
                }
            }
        }else{
            if(row==0){
                if(probe.at(0)>11){
                    for(int i=1; i<GHelper::channel_count; i++){
                        bool select = m_d->channelModel->data(m_d->channelModel->index(i,0),Qt::EditRole).toInt() != 0;
                        if(select){
                            m_d->channelModel->setData(m_d->channelModel->index(i,0),0,Qt::EditRole);
                            QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(i,1),Qt::EditRole).toByteArray();
                            if(m_pool->fileOpType == 1) emit channelItemChanged(i, false, pp);
                            m_d->channelModel->setData(m_d->channelModel->index(i,1),QVariant(),Qt::EditRole);
                        }
                    }
                }else if(GHelper::channel_count > FRET_INDEX){
                    bool select = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,0),Qt::EditRole).toInt() != 0;
                    if(select){
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,0),0,Qt::EditRole);
                        QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,1),Qt::EditRole).toByteArray();

                        if(m_pool->fileOpType == 1) emit channelItemChanged(FRET_INDEX, false, probe);
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,1),QVariant(),Qt::EditRole);
                    }
                }
            }else{
                bool select = m_d->channelModel->data(m_d->channelModel->index(0,0),Qt::EditRole).toInt() != 0;
                QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(0,1),Qt::EditRole).toByteArray();

                if(select && (pp.size()>1 || pp.at(0)>11)){
                    m_d->channelModel->setData(m_d->channelModel->index(0,0),0,Qt::EditRole);

                    if(m_pool->fileOpType == 1) emit channelItemChanged(0, false, pp);
                    m_d->channelModel->setData(m_d->channelModel->index(0,1),QVariant(),Qt::EditRole);
                }

                if(GHelper::channel_count > FRET_INDEX){
                    select = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,0),Qt::EditRole).toInt() != 0;
                    if(select){
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,0),0,Qt::EditRole);
                        pp = m_d->channelModel->data(m_d->channelModel->index(FRET_INDEX,1),Qt::EditRole).toByteArray();

                        if(m_pool->fileOpType == 1) emit channelItemChanged(FRET_INDEX, false, probe);
                        m_d->channelModel->setData(m_d->channelModel->index(FRET_INDEX,1),QVariant(),Qt::EditRole);
                    }
                }
            }
        }
#endif
    }

    for(int i=0; i<m_d->channelModel->rowCount(); i++){
        bool isEnable = m_d->channelModel->data(m_d->channelModel->index(i,0),Qt::EditRole).toInt() != 0;
        QByteArray pp = m_d->channelModel->data(m_d->channelModel->index(i,1),Qt::EditRole).toByteArray();
        QString ppStr;
        for(int j=0; j<pp.size(); j++){
            ppStr += QString::number(pp.at(j));
            if(j<pp.size()-1) ppStr+=" ";
        }
        qDebug() << Q_FUNC_INFO << "Row =" << (i+1) << ", Enable =" << isEnable << ", Probe =" << ppStr;
    }

#ifndef DEVICE_TYPE_TL13
    //清除选择
    ui->tableViewChannel->clearSelection();
#endif    
    qDebug() <<"-------------m_pool->fileOpType -------------------------------"<<m_pool->fileOpType ;
    if(m_pool->fileOpType == 1){
        emit channelItemChanged(row, isSelect, probe);
    }
    saveMethodFile();
}

void GRunEditor::slot_itemUnableClicked()
{
    qDebug() << "tlpd checkup 19";
    initCheckup();
}

void GRunEditor::slot_closeEditor()
{
#ifndef DEVICE_TYPE_TL13
    if(ui->tableViewChannel->selectionModel())
        ui->tableViewChannel->selectionModel()->clear();
#endif
}

void GRunEditor::slot_stepCheckChanged(int step, bool checked)
{
    qDebug() << Q_FUNC_INFO << step << checked;
    //    ///
    //    step = 0;
    //    checked =false;
    //    ////
    m_pool->screen_sound();

    int stageNo = 0;
    if(!currentStageIndex(stageNo)) return;
    if(step >= m_runMethod->at(stageNo)->SubNum) return;

    m_runMethod->at(stageNo)->ReadFluor[step] = checked;

    bool isSampled = false;
    //如果是设置采样
    if(checked){
        if(GHelper::total_instrument_id == 6){
            for(int i=0; i<m_runMethod->count(); i++){
                if(i == stageNo){
                    //取消其他step的采样
                    for(int j=0; j<m_runMethod->at(i)->SubNum; j++){
                        if(j == step) continue;
                        m_runMethod->at(i)->ReadFluor[j] = 0;
                        m_d->stepModel->setData(m_d->stepModel->index(j,2),0,Qt::EditRole);
                    }
                }else{
                    //取消其他step的采样
                    _STAGE_INFO *stageInfo = m_runMethod->at(i);
                    if(stageInfo == NULL) continue;
                    if(stageInfo->Property != 0) continue;
                    for(int j=0; j<stageInfo->SubNum; j++){
                        stageInfo->ReadFluor[j] = 0;
                    }
                }
            }
            isSampled = true;
            if(stageNo==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 \
                    && step==m_runMethod->last()->SubNum-1 && m_runMethod->last()->Time[m_d->specialIndex]==0){
                m_runMethod->last()->Time[m_d->specialIndex] = 30;
                refreshStepDescribe(stageNo,step);
            }
        }else{
            int stagenum=0;
            if(m_runMethod->at(stageNo)->Property==2||3)  {
                stagenum++;
            }
            for(int i=0; i<m_runMethod->count(); i++){
                bool ss= false;
                int stepnum = 0;
                qDebug()<<"-----bbbbb-------------"<<m_runMethod->at(i)->SubNum;
                bool sampleflag =false;
                //取消其他step的采样
                if(m_runMethod->at(i)->Property!=0) continue;
                for(int j=0; j<m_runMethod->at(i)->SubNum; j++){
                    if(m_runMethod->at(i)->ReadFluor[j] == 1){
                        stepnum++;
                        sampleflag=true;
                    }
                    qDebug()<<"-----bbbbb-------stepnum------"<<stepnum;
                    if(stepnum>5){
                        for(int j=0; j<m_runMethod->at(i)->SubNum; j++){
                            bool readfluor = false;
                            if(j == step) continue;
                            if(m_runMethod->at(i)->ReadFluor[j] == 1){
                                m_runMethod->at(i)->ReadFluor[j] = 0;
                                m_d->stepModel->setData(m_d->stepModel->index(j,2),0,Qt::EditRole);
                                readfluor = true;
                                stepnum--;
                            }
                            if(  readfluor == true)     break;
                        }
                    }
                }
                if(sampleflag==true)   stagenum++;
                if(stagenum>6){
                    for(int m=0; m<m_runMethod->count(); m++){
                        bool readfluor = false;
                        if(stageNo==m)  continue;
                        for(int n =0;n<m_runMethod->at(m)->SubNum;n++){
                            if(m_runMethod->at(m)->ReadFluor[n] == 1){
                                m_runMethod->at(m)->ReadFluor[n] = 0;
                                m_d->stepModel->setData(m_d->stepModel->index(2,1),0,Qt::EditRole);
                                readfluor = true;
                                ss =true;
                            }
                        }
                        stagenum--;
                        if(  readfluor == true)     break;
                    }
                    if(  ss == true)     break;
                }
            }
            isSampled = true;
            if(stageNo==m_runMethod->count()-1 && m_runMethod->last()->Cycles==1 && m_runMethod->last()->Property==0 \
                    && step==m_runMethod->last()->SubNum-1 && m_runMethod->last()->Time[m_d->specialIndex]==0){
                m_runMethod->last()->Time[m_d->specialIndex] = 30;
                refreshStepDescribe(stageNo,step);
            }
        }
        //#endif
    }else{
        //如果没有一个采样时,添加提示
        for(int i=0; i<m_runMethod->count(); i++){
            if(i == stageNo) continue;

            _STAGE_INFO *stageInfo = m_runMethod->at(i);
            if(stageInfo == NULL) continue;
            if(stageInfo->Property != 0) continue;
            for(int j=0; j<stageInfo->SubNum; j++){
                isSampled = stageInfo->ReadFluor[j] != 0;
                if(isSampled) break;
            }
            if(isSampled) break;
        }
    }

    qDebug() << "save method file 24";

    if(m_pool->fileOpType == 1) emit sampleChanged();
    saveMethodFile();

    if(m_d->item){
        m_d->item->currentSelect(stageNo,step);
        m_d->plot->replot();
    }
}

void GRunEditor::slot_stageAddChanged(const QItemSelection& selection)
{
    m_pool->screen_sound();

    if(selection.count() <= 0) return;
    QModelIndex index = selection.indexes().first();
    int _type = index.row() * STAGEADD_COLUMN_COUNT + index.column();
    ui->buttonStageAddOk->setEnabled(_type<8);
}

void GRunEditor::slot_currentChanged(int index)
{
    m_pool->screen_sound();

    ui->RunEditor->widget(index)->clearFocus();
}
