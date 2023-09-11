#include "gversiondetail.h"

#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QDebug>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

#include "gglobal.h"

GVersionDetail::GVersionDetail(const QStringList &versions, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Build No."));
    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet("QDialog {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n\nQLabel{\nbackground: transparent;\n}\n\n QPushButton {\nborder: 1px solid #2f74d0;\nfont: 11pt \"文泉驿等宽微米黑\";\nborder-radius: 2px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n\nQPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}\n\nQPushButton:disabled { \nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #F1F0EF);\n}");

    QStringList versionTitles;

#if defined (DEVICE_TYPE_TL13)
    versionTitles  << tr("System:") \
                       << tr("Application:") \
                  << tr("Thermal Cycle Module:");
#elif defined (DEVICE_TYPE_TL23)
        versionTitles << tr("System:") \
                           << tr("Application:") \
                  << tr("Optical Module:") \
                  << tr("Thermal Cycle Module:");
        if ( 6 == GHelper::total_instrument_id )
        {
            versionTitles << tr("Algorithm:");
        }
#elif (defined DEVICE_TYPE_TL22) || (defined DEVICE_TYPE_TL12)
    versionTitles<< tr("System:") \
                      << tr("Application:") \
                  << tr("Optical Module:") \
                  << tr("Thermal Cycle Module:") \
                  << tr("Driver Module:");

#endif

    QGridLayout *gridLayout = new QGridLayout;

    int row = 0;
    for(int i=0; i<versionTitles.count()+1; i++){
        if(i >= versions.count()) break;

        gridLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Preferred), row, 0, 1, 2);
        row ++;

        QLabel *label = new QLabel;
        label->setText(versionTitles.at(i));
        m_labelTitles << label;
        gridLayout->addWidget(label, row, 0, 1, 1);

        label = new QLabel(versions.at(i));
        m_labelUnits << label;
        gridLayout->addWidget(label, row, 1, 1, 1);

        row ++;
    }

    gridLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Preferred), row, 0, 1, 2);
    row ++;    

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

    gridLayout->addLayout(hLayout, row, 0, 1, 2);

    gridLayout->setContentsMargins(20,10,6,6);
    gridLayout->setSpacing(20);
    setLayout(gridLayout);
    resize(400,100);

    m_labelTitles.at(0)->setFocus();
}

GVersionDetail::~GVersionDetail()
{
    qDebug() << "delete GVersionDetail";
    m_buttonClose = NULL;
    m_labelTitles.clear();
    m_labelUnits.clear();
}

void GVersionDetail::paintEvent(QPaintEvent */*event*/)
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

