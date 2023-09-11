#include "ggradientdetail.h"

#include "gglobal.h"

#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QtMath>
#include <QDebug>
#include <QString>
#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

GGradientDetail::GGradientDetail(double center, double offset, QWidget *parent) :
    QDialog(parent),
    m_buttonClose(NULL)
{
    setAttribute(Qt::WA_TranslucentBackground);

    setWindowTitle(tr("Gradient Detail"));
    setStyleSheet("QDialog {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n\nQLabel{\nbackground: transparent;\n}\n\n QPushButton {\nborder: 1px solid #2f74d0;\nfont: 11pt \"文泉驿等宽微米黑\";\nborder-radius: 2px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n\nQPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}\n\nQPushButton:disabled { \nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #F1F0EF);\n}");

    //假设中心温度为T,梯度温度为<T, 则12列梯度温度计算公式为：
    //T(1) = T - <T;
    //T(n) = T(n-1) + An * <T, n>1; 其中An为列因子, n为列数


#ifndef DEVICE_TYPE_TL23
    const double An[] = {0, 0.1, 0.1, 0.2, 0.25, 0.25, 0.2, 0.25, 0.25, 0.2, 0.1, 0.1};
    double val = center - offset;
    QVBoxLayout *vLayout = new QVBoxLayout;
    for(int i=0; i<GHelper::column_count; i++){

        QHBoxLayout *hLayout = new QHBoxLayout;

        hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));

        QLabel *label = new QLabel(this);

        if(i<9)
            label->setText(tr("Column %1 ").arg(i+1));
        else
            label->setText(tr("Column%1").arg(i+1));

#else
        const double An[] = {0, 0.1, 0.2, 0.45, 0.5,  0.45, 0.2, 0.1};

    double val = center - offset;
    QVBoxLayout *vLayout = new QVBoxLayout;
    for(int i=0; i<GHelper::row_count; i++){

        QHBoxLayout *hLayout = new QHBoxLayout;

        hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));

        QLabel *label = new QLabel(this);

        label->setText(tr("Row ")+QString('A'+i));


#endif

        m_labelTitles << label;
        hLayout->addWidget(label);

        val = val + An[i] * offset;
        double showVal = qRound(val * 10) / 10.0;
        label = new QLabel(QString::number(showVal,'f',1)+trUtf8("℃"), this);
        m_labelUnits << label;
        hLayout->addWidget(label);

        hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));

        hLayout->setSpacing(20);

        vLayout->addLayout(hLayout);
    }

    m_buttonClose = new QPushButton(tr("Close"), this);
    m_buttonClose->setFocusPolicy(Qt::NoFocus);
    m_buttonClose->setMinimumSize(120,40);
    m_buttonClose->setMaximumSize(120,40);
    connect(m_buttonClose, SIGNAL(clicked()), this, SIGNAL(keyPressed()));
    connect(m_buttonClose, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hLayout->addWidget(m_buttonClose);
    hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    vLayout->addLayout(hLayout);

    setLayout(vLayout);
    resize(200,300);

    m_labelTitles.at(0)->setFocus();
}

GGradientDetail::~GGradientDetail()
{
    qDebug() << "delete GGradientDetail";
    m_buttonClose = NULL;
    m_labelTitles.clear();
    m_labelUnits.clear();
}

void GGradientDetail::paintEvent(QPaintEvent */*event*/)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(2, 2, this->width()-4, this->height()-4);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient gradient(0.3,-0.4,1.35,0.3,-0.4);
    QVector<QGradientStop> gradientStops;
    gradientStops.append(qMakePair(0,0x000fff));
    gradientStops.append(qMakePair(1,0xc9e5fe));
    gradient.setStops(gradientStops);
    painter.fillPath(path, QBrush(gradient));

    QColor color(47, 116, 208, 50);
    for(int i=0; i<1; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(2-i, 2-i, this->width()-(2-i)*2, this->height()-(2-i)*2);
//        color.setAlpha(150 - qSqrt(i)*50);
        color.setAlpha(255);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

