/*!
* \file gruneditor.h
* \brief ARM板软件中运行设置界面头文件
*
*实现了ARM板软件实验控制的设置功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:36
*
*/

#ifndef GRUNEDITOR_H
#define GRUNEDITOR_H

#include <QWidget>
//#include "gtabwidget.h"
#include <QMetaType>
#include <QItemSelection>

#include "gexperimentfile.h"

namespace Ui {
class GRunEditor;
}

class QStandardItem;
class GDataPool;
class GRunEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GRunEditor(GDataPool *dataPool, QWidget *parent = 0);
    ~GRunEditor();

    void showFirstTab();
public slots:
    void updateConfig();

    void experimentState(int state);
signals:
    void channelModeChanged(const QByteArray &probe);
    void channelItemChanged(int channel, bool enable, const QByteArray &probe);
    void sampleChanged();
    void editting(bool);
    void editting2(bool);
    void methodChanged();
    void methodEdited();

    void operatorLog(const QString &log);
protected:
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
#if (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    void on_buttonTubeTypeNext_clicked();
#endif
    void on_buttonReactionAdd_clicked();
    void on_buttonReactionReduce_clicked();

    void on_checkBoxHotCover_toggled(bool checked);
    void on_buttonHotlidAdd_clicked();
    void on_buttonHotlidReduce_clicked();

    void on_buttonStageUp_clicked();
    void on_buttonStageAdd_clicked();    
    void on_buttonStageReduce_clicked();
    void on_buttonStageDown_clicked();

    void on_buttonStepUp_clicked();
    void on_buttonStepAdd_clicked();    
    void on_buttonStepReduce_clicked();
    void on_buttonStepDown_clicked();

    void on_buttonStageAddOk_clicked();
    void on_buttonStageAddBack_clicked();

    void on_buttonStepEditAmpBack_clicked();
    void on_buttonStepEditMeltBack_clicked();

    void on_radioButtonStepStd_toggled(bool checked);
    void on_radioButtonStepGradient_toggled(bool checked);
    void on_radioButtonStepTouchdown_toggled(bool checked);
    void on_radioButtonStepLong_toggled(bool checked);
    void on_buttonStepGradientDetail_clicked();    

    void slot_buttonStepEdit_clicked();

    void slot_stage_insertOrRemoved();
    void slot_tableViewStage_selectionChanged();
    void slot_tableViewStep_selectionChanged();
    void slot_itemEdited(int row, const QByteArray &probe);  
    void slot_itemUnableClicked();
    void slot_closeEditor();

    void slot_stepCheckChanged(int step, bool checked);
    void slot_stageAddChanged(const QItemSelection &selection);

    void slot_currentChanged(int index);    

private:
    void initVariables();
    void initUi();

    bool currentStageIndex(int &index);
    void refreshStepDescribe(int stageNo, int stepNo);
    void saveMethodFile();

    void setShowDetail(bool normal);

    bool initCheckup(bool checkTlpd = false);
private:
    Ui::GRunEditor *ui;

    class PrivateData;
    PrivateData *m_d;           ///<私有成员变量类指针

    GDataPool   *m_pool;
    GExpRunInfo *m_runInfo;
    GExpRunMethod *m_runMethod;
};

#endif // GRUNEDITOR_H
