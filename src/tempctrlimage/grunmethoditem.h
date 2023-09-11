#ifndef GRUNMETHODITEM_H
#define GRUNMETHODITEM_H

#include "qwt_plot_item.h"
#include "pcr_info.h"
#include <QWidget>

class GDataPool;
class GRunMethodItem : public QWidget, public QwtPlotItem
{
    Q_OBJECT
public:
    enum MoveType{
        NoMove = 0,
        LeftStageAlign,
        RightStageAlign,
        LeftStepAlign,
        RightStepAlign
    };

    enum ExpType{
        Stop = 0,
        Selected,
        HotLidHeating,
        Run
    };

    explicit GRunMethodItem(GDataPool *dataPool, QList< _STAGE_INFO* > * const data, bool simplify = false, QWidget *parent = 0);
    ~GRunMethodItem();

    void initFresh();
    void locateFresh();    

    void currentSelect(int stageNo, int stepNo);

    void setHotLidMode();
    void runExp(int cycleNo, int stageNo, int stepNo, bool keepTemp, int leftTime,bool ThState);
    void stopExp();

    void clickButton(const QPoint &point, bool press);
    void moveButton(const QPoint &point);

    void calcNext(double pCanvasWidth) const;
    void calcPrev(double pCanvasWidth) const;

    double calcXStageStart(MoveType type) const;
    double calcXStepStart(MoveType type, int stepIndex) const;
    double pointStart(int stageIndex, int stepIndex) const;

    virtual int rtti() const;
    virtual void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const;

private:
    void initVariable();
private:
    GDataPool   *m_pool;
    QList< _STAGE_INFO* > *g_dataInfo;
    bool m_simplify;    ///< 是否简化显示

    int m_fontPointSize;    ///< 字体的大小    

    int m_showMode;     ///< 0停止状态,1热盖加热中,2实验中
    int m_curCycle;     ///< 实验中当前循环数
    int m_curStage;     ///< 实验中Stage的序号
    int m_curStep;      ///< 实验中Step的序号
    bool m_curKeep;     ///< 实验中是否达到保温状态
    int m_leftTime;     ///< 实验中Step剩余时间
    int m_state;     ///<
    quint8 m_flag;      ///< 实验中闪烁的标志
    int m_step ;       ///
    mutable double  m_sXWidth;        ///< 绘图X轴的长度

    bool    m_clickdown;    ///< 鼠标点下
    int     m_btnClick;     ///< 鼠标点击的按键:0无;1上一页;2下一页
    QPoint  m_clickPoint;   ///< 鼠标点击的点

    mutable bool m_stageAlign;          ///< stage/step边界
    mutable bool m_leftAlign;           ///< 是否左边界

    mutable int m_prevStepIndex;       ///< 上一个向左Step的序号
    mutable int m_nextStepIndex;       ///< 下一个向右Step的序号
    mutable int m_curStepIndex;        ///< 当前对齐的Step序号,分为左和右对齐

    mutable int m_prevStageIndex;      ///< 上一个向左Stage的序号
    mutable int m_nextStageIndex;      ///< 下一个向右Stage的序号
    mutable int m_curStageIndex;        ///< 当前对齐的Stage序号,分为左和右对齐

    mutable QRectF  m_btnPrevStageRectf;    ///< 上一个Stage的像素坐标
    mutable QRectF  m_btnNextStageRectf;    ///< 下一个Stage的像素坐标
    mutable QRectF  m_btnPrevStepRectf;     ///< 上一个Step的像素坐标
    mutable QRectF  m_btnNextStepRectf;     ///< 下一个Step的像素坐标

    mutable QList<QRectF> m_stageRectList;  ///< 所有stage范围集合
    mutable QList<QRectF> m_stepRectList;   ///< 所有step范围集合

    mutable QVector<double> m_stageWidths;  ///< 每个Stage的像素宽度    

    mutable bool m_needLocate;          ///< 是否重新定位

    mutable double m_sXMin;     ///< 保留的X轴最小值
    mutable double m_sXMax;     ///< 保留的X轴最大值

    double pStepTitleHeight; ///< Step的Title描述像素高度
    double pStageHeight;     ///< Stage的像素高度
};

#endif // GRUNMETHODITEM_H
