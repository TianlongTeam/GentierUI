#include "gcircularwidget.h"

#include <QLabel>
#include <QBoxLayout>
#include <QTimerEvent>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

GCircularWidget::GCircularWidget(QSize desktopSize, QWidget *parent) :
    QWidget(parent),
    m_desktopWidth(desktopSize.width()),
    m_desktopHeight(desktopSize.height())
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QWidget {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n\nQLabel{\nbackground: transparent;\n}\n");

    m_processIndicator.setAnimationDelay(80);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(&m_processIndicator);
    hLayout->addWidget(&m_label);

    setLayout(hLayout);
}

void GCircularWidget::showProcess(const QString &text)
{
    m_processIndicator.startAnimation();

    m_label.setText(text);
    show();
    int xx = (m_desktopWidth - width())/2;
    int yy = (m_desktopHeight - height())/2;
    move(xx,yy);
}

void GCircularWidget::hideProcess()
{
    m_processIndicator.stopAnimation();
    hide();
}

void GCircularWidget::paintEvent(QPaintEvent */*event*/)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(3, 3, this->width()-6, this->height()-6);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient gradient(0.3,-0.4,1.35,0.3,-0.4);
    QVector<QGradientStop> gradientStops;
    gradientStops.append(qMakePair(0,0x000fff));
    gradientStops.append(qMakePair(1,0xc9e5fe));
    gradient.setStops(gradientStops);
    painter.fillPath(path, QBrush(gradient));

    QColor color(47, 116, 208, 50);
    for(int i=0; i<3; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(3-i, 3-i, this->width()-(3-i)*2, this->height()-(3-i)*2);
//        color.setAlpha(150 - qSqrt(i)*50);
        color.setAlpha(255);
        painter.setPen(color);
        painter.drawPath(path);
    }
}
