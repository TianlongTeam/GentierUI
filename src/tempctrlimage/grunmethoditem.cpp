#include "grunmethoditem.h"

#include "gglobal.h"
#include "gdatapool.h"
#include "qwt_painter.h"
#include "qwt_plot.h"

#include <QPainter>
#include <QPainterPath>
#include <QComboBox>
#include <QLineEdit>
#include <QMouseEvent>
#include <QFontMetricsF>
#include <QtMath>

#ifdef DEVICE_TYPE_TL22
#define FONTSIZE    9
#else
#define FONTSIZE    10
#endif

const double pStepWidth = 120;      ///< Step的像素宽度
const double pStepTopResv = 20;     ///< Step的顶部保留区像素高度
const double pStepBtmResv = 40;     ///< Step的底部保留区像素高度

const double showPoint = 30;        ///< 如果Stage或step显示的部分大于等于该值,则设定该stage或step为按键




GRunMethodItem::GRunMethodItem(GDataPool *dataPool, QList<_STAGE_INFO *> * const data, bool simplify, QWidget */*parent*/) :
    m_pool(dataPool),
    g_dataInfo(data),
    m_simplify(simplify)
{
    setZ(40);
    setRenderHint(QwtPlotItem::RenderAntialiased, true);

    initVariable();
}

GRunMethodItem::~GRunMethodItem()
{
    g_dataInfo = NULL;
}

void GRunMethodItem::initFresh()
{
    if(plot() == NULL) return;

    initVariable();

    plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
    update();
}

void GRunMethodItem::locateFresh()
{
    if(plot() == NULL) return;

    plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
    update();
}

void GRunMethodItem::currentSelect(int stageNo, int stepNo)
{
    m_showMode = Selected;
    m_curStage = stageNo;
    m_curStep = stepNo;
    m_needLocate = true;
    update();

}

void GRunMethodItem::setHotLidMode()
{
    m_showMode = HotLidHeating;
}

void GRunMethodItem::runExp(int cycleNo, int stageNo, int stepNo, bool keepTemp, int leftTime,bool ThState)
{
    m_showMode = Run;
    m_curCycle = cycleNo;
    m_curStage = stageNo;
    m_curStep = stepNo;
    m_curKeep = keepTemp;
    m_leftTime = leftTime;
    m_state = ThState;
    m_flag++;
    update();

    //重新定位当前的Stage处于可视范围
    if(g_dataInfo == NULL) return;
    if(plot() == NULL) return;
    //当前运行的Stage处于屏幕中时不动,否则根据它所在的位置确认是左对齐还是右对齐
    if(stageNo >= m_nextStageIndex || stageNo <= m_prevStageIndex){        
        QwtScaleMap xMap = plot()->canvasMap(QwtPlot::xBottom);
        double sXMin = plot()->axisInterval(QwtPlot::xBottom).minValue();
        double sXWidth = plot()->axisInterval(QwtPlot::xBottom).width();
        double pXStart = xMap.transform(sXMin);

        double sCurStart = xMap.invTransform(pointStart(stageNo,stepNo) + pXStart) - sXMin;
        qDebug()<<" ------------sCurStart------------"<<sCurStart<<sXMin;
        if(sCurStart < sXMin){
            int steps = 0;
            for(int i=0; i<stageNo; i++){
                if(i >= g_dataInfo->count()) break;
                if(g_dataInfo->at(i) == NULL) continue;
                steps += g_dataInfo->at(i)->SubNum;
            }
            steps += stepNo;
            qDebug() << "55555555---steps--55555555555555555"<<steps;
            double sXStart = xMap.invTransform(calcXStepStart(LeftStepAlign,steps) + pXStart) - sXMin;
            bool isOk = false;
            if(sXStart+sXWidth < m_sXWidth){
                m_sXMin = sXStart;
                m_sXMax = sXStart+sXWidth;
                isOk = true;
            }else if(sXWidth<m_sXWidth && sXStart+sXWidth>m_sXWidth){
                m_sXMin = m_sXWidth-sXWidth;
                m_sXMax = m_sXWidth;
                isOk = true;
            }

            if(isOk){
                m_curStepIndex = (m_prevStepIndex>0)?m_prevStepIndex:0;
                int curStageIndex = -1;
                int stageCount = 0;
                for(int i=0; i<g_dataInfo->count(); i++){
                    if(g_dataInfo == NULL) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stageCount += g_dataInfo->at(i)->SubNum;
                    if(stageCount >= (m_curStepIndex+1)){
                        curStageIndex = i;
                        break;
                    }
                }
                m_curStageIndex = (curStageIndex < 0) ? 0 : curStageIndex;

                qDebug() << "5555555555555555555" << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
                m_leftAlign = true;
                m_stageAlign = false;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
        }else if(sCurStart > sXMin){
            int steps = 0;
            for(int i=0; i<stageNo; i++){
                if(i >= g_dataInfo->count()) break;
                if(g_dataInfo->at(i) == NULL) continue;
                steps += g_dataInfo->at(i)->SubNum;
            }
            steps += stepNo;
            qDebug() << "6666666666---steps--666666666666"<<steps;
            double sXEnd = xMap.invTransform(calcXStepStart(RightStepAlign,steps+1) + pXStart) - sXMin;
            bool isOk = false;
            if(sXEnd < sXWidth){
                m_sXMin = 0.0;
                m_sXMax = sXWidth;
                isOk = true;
            }else if(sXEnd > sXWidth){
                m_sXMin = sXEnd-sXWidth;
                m_sXMax = sXEnd;
                isOk = true;
            }

            if(isOk){
                m_curStepIndex = steps;
                int curStageIndex = -1;
                int stageCount = 0;
                for(int i=0; i<g_dataInfo->count(); i++){
                    if(g_dataInfo == NULL) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stageCount += g_dataInfo->at(i)->SubNum;
                    if(stageCount >= (m_curStepIndex+1)){
                        curStageIndex = i;
                        break;
                    }
                }
                m_curStageIndex = (curStageIndex < 0) ? 0 : curStageIndex;

                qDebug() << "66666666666666666666666" << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
                m_leftAlign = false;
                m_stageAlign = false;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
        }
        m_curStageIndex = stageNo;
    }
    qDebug() << Q_FUNC_INFO << "end";
}

void GRunMethodItem::stopExp()
{
    m_showMode = Stop;
    m_curCycle = 0;
    m_curStage = 0;
    m_curStep = 0;
    m_curKeep = false;
    m_leftTime = 0;
    m_state = false;

    m_flag = 0;

    update();
}

void GRunMethodItem::clickButton(const QPoint &point, bool press)
{
    //运行时取消翻页功能
    if(m_showMode==Run || m_showMode==HotLidHeating){
        m_btnClick = NoMove;
        return;
    }

    m_clickPoint = point;
    m_clickdown = press;
    qDebug() << Q_FUNC_INFO << point << press;
    if(m_clickdown){
        //如果按下时更新
        m_btnClick = NoMove;

        if(m_btnPrevStageRectf.contains(m_clickPoint)){
            m_btnClick = LeftStageAlign;
        }else if(m_btnPrevStepRectf.contains(m_clickPoint)){
            m_btnClick = LeftStepAlign;
        }else if(m_btnNextStageRectf.contains(m_clickPoint)){
            m_btnClick = RightStageAlign;
        }else if(m_btnNextStepRectf.contains(m_clickPoint)){
            m_btnClick = RightStepAlign;
        }
        qDebug() << Q_FUNC_INFO << "click type:" << m_btnClick << m_btnPrevStageRectf << m_btnNextStageRectf << m_btnPrevStepRectf << m_btnNextStepRectf;
        if(m_btnClick != NoMove){
            update();
        }
    }else{
        if(plot() == NULL) return;

        QwtScaleMap xMap = plot()->canvasMap(QwtPlot::xBottom);
        double sXMin = plot()->axisInterval(QwtPlot::xBottom).minValue();
        double sXWidth = plot()->axisInterval(QwtPlot::xBottom).width();
        double pXStart = xMap.transform(sXMin);

        switch(m_btnClick){
        case LeftStageAlign:{
            double sXStart = xMap.invTransform(calcXStageStart(LeftStageAlign) + pXStart) - sXMin;
            int isOk = 0;
            if(sXStart+sXWidth < m_sXWidth){
                m_sXMin = sXStart;
                isOk = 1;
                m_sXMax = sXStart+sXWidth;
            }else if(sXWidth<m_sXWidth && sXStart+sXWidth>m_sXWidth){
                m_sXMin = m_sXWidth-sXWidth;
                m_sXMax = m_sXWidth;
                isOk = 2;
            }

qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaa" << sXStart << sXWidth << (sXStart+sXWidth) << m_sXWidth;
            if(isOk){
                m_curStageIndex = (m_prevStageIndex>=0)?m_prevStageIndex:0;
                int stepCount = 0;
                for(int i=0; i<m_curStageIndex; i++){
                    if(g_dataInfo == NULL) break;
                    if(i>=g_dataInfo->count()) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stepCount += g_dataInfo->at(i)->SubNum;
                }
                m_curStepIndex = stepCount;

                qDebug() << "1111111111111111111111111" << isOk << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;

                m_leftAlign = true;
                m_stageAlign = true;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
            break;
        }
        case RightStageAlign:{
            double sXEnd = xMap.invTransform(calcXStageStart(RightStageAlign) + pXStart) - sXMin;
            int isOk = 0;
            if(sXEnd < sXWidth){
                m_sXMin = 0.0;
                m_sXMax = sXWidth;
                isOk = 1;
            }else if(sXEnd > sXWidth){
                m_sXMin = sXEnd-sXWidth;
                m_sXMax = sXEnd;
                isOk = 2;
            }
            if(isOk){
                m_curStageIndex = (m_nextStageIndex<m_stageWidths.count())?m_nextStageIndex:(m_stageWidths.count()-1);
                int stepCount = 0;
                for(int i=0; i<=m_curStageIndex; i++){
                    if(g_dataInfo == NULL) break;
                    if(i>=g_dataInfo->count()) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stepCount += g_dataInfo->at(i)->SubNum;
                }
                m_curStepIndex = stepCount-1;

                qDebug() << "2222222222222222222222" << isOk << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
                m_leftAlign = false;
                m_stageAlign = true;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
            break;
        }
        case LeftStepAlign:{
            double sXStart = xMap.invTransform(calcXStepStart(LeftStepAlign, m_prevStepIndex) + pXStart) - sXMin;
            int isOk = 0;
            if(sXStart+sXWidth < m_sXWidth){
                m_sXMin = sXStart;
                m_sXMax = sXStart+sXWidth;
                isOk = 1;
            }else if(sXWidth<m_sXWidth && sXStart+sXWidth>m_sXWidth){
                m_sXMin = m_sXWidth-sXWidth;
                m_sXMax = m_sXWidth;
                isOk = 2;
            }

            if(isOk){
                m_curStepIndex = (m_prevStepIndex>0)?m_prevStepIndex:0;
                int curStageIndex = -1;
                int stageCount = 0;
                for(int i=0; i<g_dataInfo->count(); i++){
                    if(g_dataInfo == NULL) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stageCount += g_dataInfo->at(i)->SubNum;
                    if(stageCount >= (m_curStepIndex+1)){
                        curStageIndex = i;
                        break;
                    }
                }
                m_curStageIndex = (curStageIndex < 0) ? 0 : curStageIndex;

                qDebug() << "333333333333333333333" << isOk << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
                m_leftAlign = true;
                m_stageAlign = false;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
            break;
        }
        case RightStepAlign:{
            double sXEnd = xMap.invTransform(calcXStepStart(RightStepAlign, m_nextStepIndex) + pXStart) - sXMin;
            int isOk = 0;
            if(sXEnd < sXWidth){
                m_sXMin = 0.0;
                m_sXMax = sXWidth;
                isOk = 1;
            }else if(sXEnd > sXWidth){
                m_sXMin = sXEnd-sXWidth;
                m_sXMax = sXEnd;
                isOk = 2;
            }

            if(isOk){
                m_curStepIndex = (m_nextStepIndex<m_stepRectList.count())?m_nextStepIndex:(m_stepRectList.count()-1);
                int curStageIndex = -1;
                int stageCount = 0;
                for(int i=0; i<g_dataInfo->count(); i++){
                    if(g_dataInfo == NULL) break;
                    if(g_dataInfo->at(i) == NULL) continue;
                    stageCount += g_dataInfo->at(i)->SubNum;
                    if(stageCount >= (m_curStepIndex+1)){
                        curStageIndex = i;
                        break;
                    }
                }
                m_curStageIndex = (curStageIndex < 0) ? 0 : curStageIndex;

                qDebug() << "44444444444444444444444" << isOk << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
                m_leftAlign = false;
                m_stageAlign = false;
                plot()->setAxisScale(QwtPlot::xBottom, m_sXMin, m_sXMax);
            }
            break;
        }
        default:break;
        }

        m_btnClick = NoMove;
        update();
    }
}

void GRunMethodItem::moveButton(const QPoint &point)
{
    m_clickPoint = point;

    int current = NoMove;

    if(m_btnPrevStageRectf.contains(m_clickPoint)){
        current = LeftStageAlign;
    }else if(m_btnPrevStepRectf.contains(m_clickPoint)){
        current = LeftStepAlign;
    }else if(m_btnNextStageRectf.contains(m_clickPoint)){
        current = RightStageAlign;
    }else if(m_btnNextStepRectf.contains(m_clickPoint)){
        current = RightStepAlign;
    }

    //如果鼠标移动后按键的点击状态发生变化,则刷新
    if(current != m_btnClick){
        m_btnClick = current;
        update();
    }
}

void GRunMethodItem::calcNext(double pCanvasWidth) const
{
    qDebug() << Q_FUNC_INFO << m_stageAlign << m_leftAlign;

    if(m_stageAlign){
        //如果是按照stage对齐方式（左或右对齐）
        if(m_leftAlign){
            //如果是左对齐时
            //获得屏幕右侧stage的序号
            double points = 0.0;
            int pressStageIndex = -1;   //next按键对应的序号

            m_nextStageIndex = -1;
            for(int i=m_curStageIndex; i<m_stageWidths.count(); i++){
                points += m_stageWidths.at(i);

                if(points > pCanvasWidth){
                    //如果显示部分大于设置值，设为按键
                    pressStageIndex = (m_stageWidths.at(i)+pCanvasWidth-points >= showPoint) ? i : (i-1);
                    m_nextStageIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    pressStageIndex = i;
                    m_nextStageIndex = i+1;
                    break;
                }
            }

            if(pressStageIndex<0 || pressStageIndex>=m_stageRectList.count()){
                m_btnNextStageRectf = QRectF();
            }else{
                m_btnNextStageRectf = m_stageRectList.at(pressStageIndex);
            }

            //获得屏幕右侧step的序号
            m_nextStepIndex = -1;
            int stepCount = 0;
            for(int i=0; i<m_curStageIndex; i++){
                if(g_dataInfo == NULL) break;
                if(i>=g_dataInfo->count()) break;
                if(g_dataInfo->at(i) == NULL) continue;
                stepCount += g_dataInfo->at(i)->SubNum;
            }
            int pressStepIndex = -1;
            points = 0.0;
            for(int i=stepCount; i<m_stepRectList.count(); i++){
                points += pStepWidth;

                if(points > pCanvasWidth){
                    pressStepIndex = (pStepWidth+pCanvasWidth-points >= showPoint)?i:(i+1);
                    m_nextStepIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    m_nextStepIndex = (i+1);
                    pressStepIndex = i;
                    break;
                }
            }

            //step按键(next)范围
            if(pressStepIndex<0 || pressStepIndex >= m_stepRectList.count()){
                m_btnNextStepRectf = QRectF();
            }else{
                m_btnNextStepRectf = m_stepRectList.at(pressStepIndex);
            }
        }else{
            //如果是右对齐时

            //获得下一个stage按键的范围                        
            m_nextStageIndex = m_curStageIndex+1;

            //stage按键(next)范围
            if(m_curStageIndex<0 || m_curStageIndex>=m_stageRectList.count()){
                m_btnNextStageRectf = QRectF();
            }else{
                m_btnNextStageRectf = m_stageRectList.at(m_curStageIndex);
            }

            //获得下一个step按键的范围
            int stepCount = 0;
            for(int i=0; i<=m_curStageIndex; i++){
                if(g_dataInfo == NULL) break;
                if(i>=g_dataInfo->count()) break;
                if(g_dataInfo->at(i) == NULL) continue;
                stepCount += g_dataInfo->at(i)->SubNum;
            }

            //下一个step的序号
            m_nextStepIndex = stepCount;

            stepCount--;

            //step按键(next)范围
            if(stepCount<0 || stepCount>=m_stepRectList.count()){
                m_btnNextStepRectf = QRectF();
            }else{
                m_btnNextStepRectf = m_stepRectList.at(stepCount);
            }
        }

    }else{
        //如果是按照step对齐方式（左或右对齐）
        int pressStepIndex = -1;
        if(m_leftAlign){
            //如果是左对齐时
            //获得下一个step按键的范围
            double points = 0.0;            
            m_nextStepIndex = -1;
            for(int i=m_curStepIndex; i<m_stepRectList.count(); i++){
                points += pStepWidth;

                if(points > pCanvasWidth){
                    //如果显示部分大于设置值，设为按键
                    pressStepIndex = (pStepWidth+pCanvasWidth-points >= showPoint) ? i : (i-1);
                    m_nextStepIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    pressStepIndex = i;
                    m_nextStepIndex = i+1;
                    break;
                }
            }

        }else{
            //如果是右对齐时
            pressStepIndex = m_curStepIndex;
            m_nextStepIndex = m_curStepIndex + 1;
        }

        if(pressStepIndex<0 || pressStepIndex>=m_stepRectList.count()){
            m_btnNextStepRectf = QRectF();
        }else{
            m_btnNextStepRectf = m_stepRectList.at(pressStepIndex);
        }

        //获得当前stage序号
        m_nextStageIndex = -1;
        int stageCount = 0;
        for(int i=0; i<g_dataInfo->count(); i++){
            if(g_dataInfo == NULL) break;
            if(g_dataInfo->at(i) == NULL) continue;
            stageCount += g_dataInfo->at(i)->SubNum;
            if(stageCount > (pressStepIndex+1)){
                m_nextStageIndex = i;
                break;
            }else if(stageCount == (pressStepIndex+1)){
                m_nextStageIndex = i+1;
                break;
            }
        }
    }
}

void GRunMethodItem::calcPrev(double pCanvasWidth) const
{
    qDebug() << Q_FUNC_INFO << m_stageAlign << m_leftAlign;

    if(m_stageAlign){
        //如果是按照stage对齐方式（左或右对齐）
        if(m_leftAlign){
            //如果是左对齐时
            //获得上一个stage按键的范围
            m_prevStageIndex = m_curStageIndex-1;

            //stage按键(next)范围
            if(m_curStageIndex<0 || m_curStageIndex>=m_stageRectList.count()){
                m_btnPrevStageRectf = QRectF();
            }else{
                m_btnPrevStageRectf = m_stageRectList.at(m_curStageIndex);
            }

            //获得上一个step按键的范围

            int stepCount = 0;
            for(int i=0; i<m_curStageIndex; i++){
                if(g_dataInfo == NULL) break;
                if(i>=g_dataInfo->count()) break;
                if(g_dataInfo->at(i) == NULL) continue;
                stepCount += g_dataInfo->at(i)->SubNum;
            }

            m_prevStepIndex = stepCount-1;

            //step按键(next)范围
            if(stepCount<0 || stepCount>=m_stepRectList.count()){
                m_btnPrevStepRectf = QRectF();
            }else{
                m_btnPrevStepRectf = m_stepRectList.at(stepCount);
            }
        }else{
            //如果是右对齐时

            //获得上一个stage按键的范围
            double points = 0.0;            

            int pressStageIndex = -1;   //next按键对应的序号

            m_prevStageIndex = -1;
            for(int i=m_curStageIndex; i>=0; i--){
                points += m_stageWidths.at(i);

                if(points > pCanvasWidth){
                    //如果显示部分大于设置值，设为按键
                    pressStageIndex = (m_stageWidths.at(i)+pCanvasWidth-points >= showPoint) ? i : (i+1);
                    m_prevStageIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    pressStageIndex = i;
                    m_prevStageIndex = i-1;
                    break;
                }
            }

            if(pressStageIndex<0 || pressStageIndex>=m_stageRectList.count()){
                m_btnPrevStageRectf = QRectF();
            }else{
                m_btnPrevStageRectf = m_stageRectList.at(pressStageIndex);
            }

            //获得上一个step按键的范围

            //全部step总数
            int stepCount = 0;
            for(int i=0; i<=m_curStageIndex; i++){
                if(g_dataInfo == NULL) break;
                if(g_dataInfo->at(i) == NULL) continue;
                stepCount += g_dataInfo->at(i)->SubNum;
            }

            int pressStepIndex = -1;
            m_prevStepIndex = -1;
            points = 0.0;
            for(int i=stepCount-1; i>=0; i--){
                points += pStepWidth;
                if(points > pCanvasWidth){
                    pressStepIndex = (pStepWidth+pCanvasWidth-points >= showPoint)?i:(i+1);
                    m_prevStepIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    m_prevStepIndex = (i-1);
                    pressStepIndex = i;
                    break;
                }
            }

            //step按键(next)范围
            if(pressStepIndex<0 || pressStepIndex>=m_stepRectList.count()){
                m_btnPrevStepRectf = QRectF();
            }else{
                m_btnPrevStepRectf = m_stepRectList.at(pressStepIndex);
            }
        }
    }else{
        //如果是按照step对齐方式（左或右对齐）
        int pressStepIndex = -1;
        if(m_leftAlign){
            pressStepIndex = m_curStepIndex;
            m_prevStepIndex = m_curStepIndex - 1;
        }else{
            //如果是左对齐时
            //获得下一个step按键的范围
            double points = 0.0;
            m_prevStepIndex = -1;
            for(int i=m_curStepIndex; i>=0; i--){
                points += pStepWidth;

                if(points > pCanvasWidth){
                    //如果显示部分大于设置值，设为按键
                    pressStepIndex = (pStepWidth+pCanvasWidth-points >= showPoint) ? i : (i+1);
                    m_prevStepIndex = i;
                    break;
                }else if(points == pCanvasWidth){
                    pressStepIndex = i;
                    m_prevStepIndex = i-1;
                    break;
                }
            }
        }

        if(pressStepIndex<0 || pressStepIndex>=m_stepRectList.count()){
            m_btnPrevStepRectf = QRectF();
        }else{
            m_btnPrevStepRectf = m_stepRectList.at(pressStepIndex);
        }

        //获得当前stage序号
        m_prevStageIndex = -1;
        int stageCount = 0;
        for(int i=0; i<g_dataInfo->count(); i++){
            if(g_dataInfo == NULL) break;
            if(g_dataInfo->at(i) == NULL) continue;
            stageCount += g_dataInfo->at(i)->SubNum;
            if(stageCount > (pressStepIndex+1)){
                m_prevStageIndex = i;
                break;
            }else if(stageCount == (pressStepIndex+1)){
                m_prevStageIndex = i-1;
                break;
            }
        }
    }
}

double GRunMethodItem::calcXStageStart(MoveType type) const
{
    //根据当前屏幕stage左右两边的序号和移动方式，计算向左或向右移动一个stage后，X轴从0点到对齐点的像素值
    double points = 0.0;
    int count = 0;
    if(type==LeftStageAlign){
        count = m_prevStageIndex;
    }else if(type==RightStageAlign){
        count = (m_nextStageIndex<m_stageWidths.count()-1) ? (m_nextStageIndex+1) : (m_stageWidths.count());
    }

    for(int i=0; i<count; i++){
        //添加Stage宽度
        points += m_stageWidths.at(i);
    }

    return points;
}

double GRunMethodItem::calcXStepStart(MoveType type, int stepIndex) const
{
    int count = 0;
    if(type == LeftStepAlign){
        count = stepIndex;
    }else if(type == RightStepAlign){
        count = stepIndex+1;
    }

    count = (count<0) ? 0 : (count>m_stepRectList.count()?m_stepRectList.count():count);

    return count*pStepWidth;
}

double GRunMethodItem::pointStart(int stageIndex, int stepIndex) const
{
    double leftpoints = 0.0;

    int size = stageIndex < m_stageWidths.count() ? stageIndex : m_stageWidths.count()-1;
    for(int i=0; i<size; i++){
        leftpoints += m_stageWidths.at(i);
    }
    leftpoints += stepIndex * pStepWidth;

    return leftpoints;
}

int GRunMethodItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotUserItem;
}

void GRunMethodItem::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    qDebug() << Q_FUNC_INFO;
    if(g_dataInfo == NULL) return;
    if(plot() == NULL) return;

    painter->save();

    //得到起始点像素值
    double pOriginX = xMap.transform(0);
    double pOriginY = yMap.transform(0);

    //得到当前X轴显示范围
    double sXMin = plot()->axisInterval(QwtPlot::xBottom).minValue();
    double sXMax = plot()->axisInterval(QwtPlot::xBottom).maxValue();
//    double sXWidth = plot()->axisInterval(QwtPlot::xBottom).width();
    double pXStart = xMap.transform(sXMin);
    double pXWidth = xMap.transform(sXMax) - pXStart;    

    QFont font = painter->font();
    font.setPointSize(m_fontPointSize);
    QFontMetricsF metricsF(font);    

    QPen pen = painter->pen();        

    double pStageStartX = 0.0;          ///< 第一个Stage框的X像素坐标
    double start_temp = 20.0;           ///< 起始温度
    bool notFirstStep = false;          ///< 是否是第一个Step,是否使用起始温度
    double prevTempY = 0.0;             ///< 上一次Step的温度Y像素坐标
    double pXMaxPoints = 0.0;           ///< Ｘ轴上最大的像素点坐标

    m_stageWidths.clear();

    m_stageRectList.clear();
    m_stepRectList.clear();

    for(int i=0; i<g_dataInfo->count(); i++)
    {
        _STAGE_INFO *stageInfo = g_dataInfo->at(i);
        if(stageInfo == NULL) continue;

        pen.setWidth(1);
        pen.setColor(QColor(0x2f75d0));
        painter->setPen(pen);

        int stepCount = stageInfo->SubNum;

        //绘制Stage框
        double pStageWidth = stepCount * pStepWidth;

        //保存Stage的像素宽度
        m_stageWidths.append(pStageWidth);

        QRectF pStageRectF(pOriginX+pStageStartX, pOriginY, pStageWidth, -pStageHeight);
        painter->drawLine(pStageRectF.bottomLeft(),pStageRectF.bottomRight());
        painter->drawLine(pStageRectF.right(), canvasRect.top(), pStageRectF.right(), pStageRectF.top());

        m_stageRectList.append(pStageRectF);

        //当鼠标按下时显示底色
        bool isPressDown = false;
        if(m_clickdown && m_btnClick!=NoMove){
            switch(m_btnClick){
            case LeftStageAlign:
                isPressDown = pStageRectF == m_btnPrevStageRectf;
                break;
            case RightStageAlign:
                isPressDown = pStageRectF == m_btnNextStageRectf;
                break;
            default:break;
            }
        }

        painter->fillRect(pStageRectF.adjusted(1,-1,-1,1), isPressDown?QColor(0xa3b05a):QColor(0xc9e5fe));
        pen.setColor(0x383e83);        

        //绘制Stage的名称        
        painter->setPen(pen);

        font.setPointSize(m_fontPointSize);
        painter->setFont(font);
        metricsF = QFontMetricsF(font);

        QRectF pStageNameF = pStageRectF;

        if(m_simplify){
            QString stageName = metricsF.elidedText(QString(stageInfo->Name)+QString(" x")+QString::number(stageInfo->Cycles), Qt::ElideRight, pStageNameF.width()-4);
            painter->drawText(pStageNameF, Qt::AlignCenter, stageName);
        }else{
            double stageHalfHeight = pStageRectF.height()/2.0;

            pStageNameF.setTop(pStageNameF.bottom());
            pStageNameF.setHeight(-stageHalfHeight);

            QString stageName = metricsF.elidedText(QString(stageInfo->Name), Qt::ElideRight, pStageNameF.width()-4);
            painter->drawText(pStageNameF, Qt::AlignCenter, stageName);

            //绘制Stage的循环数
            QRectF pStageCycleF = pStageRectF;
            pStageCycleF.setHeight(stageHalfHeight);

            QString cycleStr = metricsF.elidedText(QString("x ")+QString::number(stageInfo->Cycles), Qt::ElideRight, pStageCycleF.width()-4);
            painter->drawText(pStageCycleF, Qt::AlignCenter, cycleStr);
        }

        //绘制Step
        for(int j=0; j<stepCount; j++){
            pen.setWidth(1);
            pen.setColor(QColor(0x2f75d0));
            painter->setPen(pen);

            //绘制Step框
            double pStepStartX = pStageRectF.x() + j * pStepWidth;
            double pStepStartY = pStageRectF.bottom();
            QRectF pStepRectF(pStepStartX, pStepStartY, pStepWidth, pStageHeight-canvasRect.height());

            if(j < stepCount-1){
                painter->drawLine(pStepRectF.right(), canvasRect.top(), pStepRectF.right(), pStepRectF.top());
            }

            m_stepRectList.append(pStepRectF);

            //当鼠标按下时显示底色
            isPressDown = false;
            if(m_clickdown && m_btnClick!=NoMove){
                switch(m_btnClick){
                case LeftStepAlign:
                    isPressDown = pStepRectF == m_btnPrevStepRectf;
                    break;
                case RightStepAlign:
                    isPressDown = pStepRectF == m_btnNextStepRectf;
                    break;
                default:break;
                }
            }

            painter->fillRect(pStepRectF.adjusted(1,-1,-1,1), isPressDown?QColor(0xa3b05a):QColor(0xc9e5fe));
            pen.setColor(0x383e83);

            //绘制Step Title的下线
            double pStepHeight = canvasRect.height() - pStageHeight - pStepBtmResv - pStepTopResv - pStepTitleHeight;
            double pStepTitleY = pStepStartY + pStepTitleHeight + pStageHeight - canvasRect.height();
            QRectF pStepTitleRectF(pStepStartX, pStepTitleY, pStepWidth, -pStepTitleHeight);
            painter->drawLine(pStepTitleRectF.topLeft(), pStepTitleRectF.topRight());

            //运行时闪烁功能
            if(m_state && m_curStage == i && m_curStep == j && ((m_showMode == Selected) || ((m_showMode == Run) && (m_flag & 0x01) && m_curKeep))){
                painter->fillRect(pStepTitleRectF.adjusted(1,-1,-1,1), QColor(0x308cc6));

                QRectF pStepHeightRectF(pStepStartX, pStepStartY, pStepWidth, pStageHeight+pStepTitleHeight-canvasRect.height());
                painter->fillRect(pStepHeightRectF.adjusted(1,-1,-1,1), QColor(0x308cc6));

                pen.setColor(QColor(0xffffff));
                painter->setPen(pen);
            }else{
                painter->fillRect(pStepTitleRectF.adjusted(1,-1,-1,1), QColor(0xc9e5fe));
            }            

            //温度变化线使用Step宽度的1/4
            double pTempMidX = pStepStartX + pStepWidth / 4.0;
            double pTempLastX = pStepStartX + pStepWidth - 2;

            double pTempFirstX = 0.0, pTempFirstY = 0.0;

            if(notFirstStep){
                //如果不是是第一个step,使用上一次温度
                pTempFirstX = pStepStartX;
                pTempFirstY = prevTempY;

                //累计X轴像素坐标
                pXMaxPoints += pStepWidth;
            }else{
                notFirstStep = true;

                //如果是第一个step,按起始温度开始
                pTempFirstX = pStepStartX;
                pTempFirstY = pStageRectF.bottom() - pStepBtmResv - (start_temp * pStepHeight / 100.0);

                //累计X轴像素坐标
                pXMaxPoints = pXStart + pStepWidth;
            }

            //根据Step的propery属性值绘不同的曲线
            double pTempMidY1 = 0.0;

            double step_temp = stageInfo->Temp[j] / 100.0;
            pTempMidY1 = pStageRectF.bottom() - pStepBtmResv - (step_temp * pStepHeight / 100.0);

            //绘制温度变化线
            bool bTempChanging = true;
            if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run) && !m_curKeep && (m_flag & 0x01)) bTempChanging = false;

            pen.setColor(bTempChanging?QColor(0x2f75d0):QColor(Qt::red));
            painter->setPen(pen);
            painter->drawLine(QPointF(pTempFirstX,pTempFirstY), QPointF(pTempMidX,pTempMidY1));
            pen.setColor(QColor(0x2f75d0));
            painter->setPen(pen);

            //绘恒温温度线
            pen.setWidth(3);
            painter->setPen(pen);

            painter->drawLine(QPointF(pTempMidX,pTempMidY1), QPointF(pTempLastX,pTempMidY1));

            pen.setWidth(1);
            painter->setPen(pen);

            //绘制Step的Title
            if(m_state && m_curStage == i && m_curStep == j && ((m_showMode == Selected) || ((m_showMode == Run) && m_curKeep && (m_flag & 0x01)))){
                pen.setColor(0xf2f8ff);
            }else{
                pen.setColor(0x383e83);
            }

            painter->setPen(pen);

            font.setPointSize(m_fontPointSize);
            painter->setFont(font);
            metricsF = QFontMetricsF(font);

            QString stepTitle = metricsF.elidedText(tr("Step %1").arg(j+1), Qt::ElideRight, pStepTitleRectF.width()-4);
            painter->drawText(pStepTitleRectF, Qt::AlignCenter, stepTitle);

            //绘制温度,时间及速率
            font.setPointSize(FONTSIZE);
            painter->setFont(font);
            metricsF = QFontMetricsF(font);
            switch(stageInfo->SubProperty[j]){
            case 1:{
                //绘制温度
                double targetTemp = stageInfo->TarValue[j];
                QRectF pStepTempRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, -pStepTopResv);
                double step_temp_val = step_temp;
                if(m_showMode == Run){
                    double temp_step = (stageInfo->Delta[j]) * m_curCycle;
                    if(step_temp < targetTemp){
                        step_temp_val = (step_temp+temp_step < targetTemp) ? (step_temp+temp_step) : targetTemp;
                    }else if(step_temp > targetTemp){
                        step_temp_val = (step_temp-temp_step > targetTemp) ? (step_temp-temp_step) : targetTemp;
                    }
                }
                QString stepTemp = metricsF.elidedText(QString::number(step_temp_val,'f',1)+trUtf8("»")+QString::number(targetTemp,'f',1)+trUtf8("℃"), Qt::ElideRight, pStepTempRectF.width()-4);
                painter->drawText(pStepTempRectF, Qt::AlignCenter, stepTemp);
                qDebug()<<" -----------1------aaaaaaaaa----"<<stepTemp;

                //绘制时间
                quint16 vHoldTime = stageInfo->Time[j];
                if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run)/* && m_curKeep*/){
                     vHoldTime = m_leftTime;
                }
                int hour = vHoldTime / 60;
                int minute = vHoldTime % 60;
                QRectF pStepTimeRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, pStepBtmResv/2.0);
                QString stepTime = metricsF.elidedText(tr("%1:%2").arg(QString::number(hour).rightJustified(2,'0',true)).arg(QString::number(minute).rightJustified(2,'0',true)), Qt::ElideRight, pStepTimeRectF.width()-4);
                painter->drawText(pStepTimeRectF, Qt::AlignCenter, stepTime);

#ifndef DEVICE_TYPE_TL13
                if(stageInfo->ReadFluor[j]){
                    QPointF pReadFluorF((pTempMidX+pTempLastX*2.0)/3.0, pTempMidY1+pStepBtmResv/2.0);
                    QPixmap pixmap(":/png/fsingle");
                    painter->drawPixmap(pReadFluorF, pixmap);
                }
#endif
                break;
            }
            case 2:{
                QRectF pStepTempRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, -pStepTopResv);
                QString stepTemp = metricsF.elidedText(QString::number(step_temp,'f',1)+trUtf8("℃"), Qt::ElideRight, pStepTempRectF.width()-4);
                       qDebug()<<" -----------2------aaaaaaaaa----"<<stepTemp;
                painter->drawText(pStepTempRectF, Qt::AlignCenter, stepTemp);
                qDebug()<<" -----------2------000000000------"<<stepTemp;

                quint16 vHoldTime = stageInfo->Time[j];
                if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run)/* && m_curKeep*/){
                    vHoldTime = m_leftTime;
                }

                int hour1 = vHoldTime / 60;
                int minute1 = vHoldTime % 60;
                int hour2 = stageInfo->TarValue[j] / 60;
                int minute2 = (int)stageInfo->TarValue[j] % 60;
                QRectF pStepTimeRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, pStepBtmResv/2.0);
                QString stepTime = metricsF.elidedText(tr("%1:%2").arg(QString::number(hour1).rightJustified(2,'0',true)).arg(QString::number(minute1).rightJustified(2,'0',true))+trUtf8("»")+tr("%1:%2").arg(QString::number(hour2).rightJustified(2,'0',true)).arg(QString::number(minute2).rightJustified(2,'0',true)), Qt::ElideRight, pStepTimeRectF.width()-4);
               qDebug()<<" -----------2-bbbb-----"<<stepTime;
                painter->drawText(pStepTimeRectF, Qt::AlignCenter, stepTime);
                qDebug()<<" -----------2-----111111111111-------"<<minute2;

#ifndef DEVICE_TYPE_TL13
                //绘制是否采样
                if(stageInfo->ReadFluor[j]){
                    QPointF pReadFluorF((pTempMidX+pTempLastX*2.0)/3.0, pTempMidY1+pStepBtmResv/2.0);
                    QPixmap pixmap(":/png/fsingle");
                    painter->drawPixmap(pReadFluorF, pixmap);
                }
#endif
                break;
            }
            case 3:{
                QRectF pStepTempRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, -pStepTopResv);
                QString stepTemp = metricsF.elidedText(QString::number(step_temp,'f',1)+trUtf8("±")+QString::number(qAbs(step_temp-step_temp),'f',1)+trUtf8("℃"), Qt::ElideRight, pStepTempRectF.width()-4);
                painter->drawText(pStepTempRectF, Qt::AlignCenter, stepTemp);
                quint16 vHoldTime = stageInfo->Time[j];
                if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run)/* && m_curKeep*/){
                    vHoldTime = m_leftTime;
                }
                int hour = vHoldTime / 60;
                int minute = vHoldTime % 60;
                QRectF pStepTimeRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, pStepBtmResv/2.0);
                QString stepTime = metricsF.elidedText(tr("%1:%2").arg(QString::number(hour).rightJustified(2,'0',true)).arg(QString::number(minute).rightJustified(2,'0',true)), Qt::ElideRight, pStepTimeRectF.width()-4);
                painter->drawText(pStepTimeRectF, Qt::AlignCenter, stepTime);


#ifndef DEVICE_TYPE_TL13
                //绘制是否采样
                if(stageInfo->ReadFluor[j]){
                    QPointF pReadFluorF((pTempMidX+pTempLastX*2.0)/3.0, pTempMidY1+pStepBtmResv/2.0);
                    QPixmap pixmap(":/png/fsingle");
                    painter->drawPixmap(pReadFluorF, pixmap);
                }
#endif
                break;
            }
            case 4:{
                //绘制温度
                QRectF pStepTempRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, -pStepTopResv);
                QString stepTemp = metricsF.elidedText(QString::number(step_temp,'f',1)+trUtf8("℃"), Qt::ElideRight, pStepTempRectF.width()-4);
                painter->drawText(pStepTempRectF, Qt::AlignCenter, stepTemp);

                //是否连续性熔解
                bool isContinue = stageInfo->ReadMode[j] == 2;
                if(!isContinue){    //阶跃型熔解没有保温时间
                    //绘制时间
                    quint16 vHoldTime = stageInfo->Time[j];
                    if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run)/* && m_curKeep*/){
                        vHoldTime = m_leftTime;
                    }
                    int hour = vHoldTime / 60;
                    int minute = vHoldTime % 60;
                    QRectF pStepTimeRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, pStepBtmResv/2.0);
                    QString stepTime = metricsF.elidedText(tr("%1:%2").arg(QString::number(hour).rightJustified(2,'0',true)).arg(QString::number(minute).rightJustified(2,'0',true)), Qt::ElideRight, pStepTimeRectF.width()-4);
                    painter->drawText(pStepTimeRectF, Qt::AlignCenter, stepTime);
                }

                //绘制台阶
                double pRateStartY = (pTempFirstY + pTempMidY1+ pStepTopResv) / 2.0;

                if(isContinue){
                    double deta = pStepTopResv;
                    if(pTempFirstY >= pTempMidY1){
                        if(pRateStartY>=pTempMidY1 && pRateStartY<pTempMidY1+deta){
                            pRateStartY = pTempMidY1 + deta;
                        }
                    }else{
                        pRateStartY = pTempMidY1 + deta;
                    }
                }else{
                    double deta = pStepTopResv + pStepBtmResv/2.0;
                    if(pTempFirstY >= pTempMidY1){
                        if(pRateStartY>=pTempMidY1 && pRateStartY<pTempMidY1+deta){
                            pRateStartY = pTempMidY1 + deta;
                        }
                    }else{
                        pRateStartY = pTempMidY1 + deta;
                    }
                }

                QRectF pStepRateRectF(pTempFirstX, pRateStartY, pTempLastX-pTempFirstX, -pStepTopResv);
                QString tmp = isContinue ? (QString::number(stageInfo->ReadInterval[j])+tr("Readings")+trUtf8("/℃")) : (QString::number(stageInfo->ReadInterval[j])+trUtf8("℃"));

                QString stepRate = metricsF.elidedText(tmp, Qt::ElideRight, pStepRateRectF.width()-4);
                painter->drawText(pStepRateRectF, Qt::AlignVCenter|Qt::AlignLeft, stepRate);

                //绘制采样标志
#ifndef DEVICE_TYPE_TL13
                QPixmap pixmap(isContinue?":/png/fserial":":/png/fsingle");

                QPointF pReadFluorF((pTempFirstX+pTempMidX-pixmap.width())/2.0, pStepRateRectF.bottom()+pStepTopResv);

                painter->drawPixmap(pReadFluorF, pixmap);
#endif
                break;
            }
            default:{
                //绘制温度
                QRectF pStepTempRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, -pStepTopResv);
                QString stepTemp = metricsF.elidedText(QString::number(step_temp,'f',1)+trUtf8("℃"), Qt::ElideRight, pStepTempRectF.width()-4);
                painter->drawText(pStepTempRectF, Qt::AlignCenter, stepTemp);

                //绘制时间
                quint16 vHoldTime = stageInfo->Time[j];
                if(m_state && m_curStage == i && m_curStep == j && (m_showMode == Run) && m_curKeep){
                    vHoldTime = m_leftTime;
                }
                int hour = vHoldTime / 60;
                int minute = vHoldTime % 60;
                QRectF pStepTimeRectF(pTempMidX, pTempMidY1, pTempLastX-pTempMidX, pStepBtmResv/2.0);
                QString txt = (stageInfo->Time==0) ? "∞" : tr("%1:%2").arg(QString::number(hour).rightJustified(2,'0',true)).arg(QString::number(minute).rightJustified(2,'0',true));
                QString stepTime = metricsF.elidedText(txt, Qt::ElideRight, pStepTimeRectF.width()-4);
                painter->drawText(pStepTimeRectF, Qt::AlignCenter, stepTime);

//                //绘制速率
//                double pRateStartY = (pTempFirstY + pTempMidY1+ pStepTopResv) / 2.0;
//                QRectF pStepRateRectF(pTempFirstX, pRateStartY, pTempLastX-pTempFirstX, -pStepTopResv);

//                int z = 0;
//                double ramp = stageInfo->Ramp/100.0;
//                if(ramp < MIN_RAMP){
//                    ramp = MIN_RAMP;
//                }else if(ramp > m_pool->maxSpeed[z]){
//                    ramp = m_pool->maxSpeed[z];
//                }
//                QString stepRate = metricsF.elidedText(tr("%1%2").arg(QString::number(ramp,'f',1)).arg(trUtf8("℃/s")), Qt::ElideRight, pStepRateRectF.width()-4);
//                painter->drawText(pStepRateRectF, Qt::AlignVCenter|Qt::AlignLeft, stepRate);

#ifndef DEVICE_TYPE_TL13
                //绘制是否采样
                if(stageInfo->ReadFluor[j]){
                    QPointF pReadFluorF((pTempMidX+pTempLastX*2.0)/3.0, pTempMidY1+pStepBtmResv/2.0);
                    QPixmap pixmap(":/png/fsingle");

                    painter->drawPixmap(pReadFluorF, pixmap);
                }
#endif
                break;
            }
            }

            //保存这次Step坐标
            prevTempY = pTempMidY1;
        }

        //重新定位Stage的X像素坐标
        pStageStartX = pStageStartX + pStageWidth;
    }

    if(g_dataInfo->count() > 0){
        //画边框
        pen.setWidth(1);
        pen.setColor(QColor(0x2f75d0));
        painter->setPen(pen);
        QRectF pStageAllRectF(pOriginX, pOriginY, pStageStartX, -canvasRect.height());
        painter->drawLine(pStageAllRectF.topLeft(), pStageAllRectF.topRight());
        painter->drawLine(pStageAllRectF.bottomLeft().x(), pStageAllRectF.bottomLeft().y()+2, pStageAllRectF.bottomRight().x(), pStageAllRectF.bottomRight().y()+2);
        painter->drawLine(pStageAllRectF.topLeft().x()+1,pStageAllRectF.topLeft().y(), pStageAllRectF.bottomLeft().x()+1,pStageAllRectF.bottomLeft().y());
    //    painter->drawRect(canvasRect);
    }

    //保留绘图最大值
    m_sXWidth = xMap.invTransform(pXMaxPoints) - sXMin;

    //计算翻页中使用的各种变量的值
    calcNext(pXWidth);
    calcPrev(pXWidth);    

    painter->restore();

    qDebug() << Q_FUNC_INFO << "end" << m_prevStageIndex << m_curStageIndex << m_nextStageIndex << m_prevStepIndex << m_curStepIndex << m_nextStepIndex;
}

void GRunMethodItem::initVariable()
{
    m_showMode = Stop;
    m_curCycle = 0;
    m_curStage = 0;
    m_curStep = 0;
    m_curKeep = false;
    m_leftTime = 0;

    m_flag = 0;

    m_sXWidth = 0.0;

    m_clickdown = false;
    m_btnClick = NoMove;

    m_stageAlign = true;
    m_leftAlign = true;

    m_prevStepIndex = 0;
    m_nextStepIndex = 0;
    m_curStepIndex = 0;    

    m_prevStageIndex = 0;
    m_nextStageIndex = 0;
    m_curStageIndex = 0;

    m_stageRectList.clear();
    m_stepRectList.clear();

    m_stageWidths.clear();
    m_needLocate = false;

    m_sXMin = 0.0;
    m_sXMax = 100;

    if(m_simplify){
        pStepTitleHeight = 20.0;
        pStageHeight = 20.0;
        m_fontPointSize = 9;
    }else{
        pStepTitleHeight = 30.0;
        pStageHeight = 50.0;
        m_fontPointSize = 11;
    }
}
