#include "gusbselectdialog.h"

#include <QPushButton>
#include <QBoxLayout>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

#include <QDebug>

GUsbSelectDialog::GUsbSelectDialog(const QMap<QString, QString> &usbDevices, QWidget *parent) : \
    QDialog(parent)
{
    m_currentDev.clear();

    QVBoxLayout *vLayout = new QVBoxLayout;

    foreach(const QString &key, usbDevices.keys()){
        QPushButton *button  = new QPushButton(usbDevices.value(key), this);
        button->setFlat(true);
        connect(button, SIGNAL(clicked()), this, SLOT(usbButtonClicked()));

        vLayout->addWidget(button);

        m_buttons.insert(button, key);
    }

    QPushButton *button  = new QPushButton(tr("Cancel"), this);
    button->setFlat(true);
    connect(button, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));

    vLayout->addWidget(button);

    this->setLayout(vLayout);

    this->resize(200, (usbDevices.count()+1)*30);
    this->setFocus();
}

GUsbSelectDialog::~GUsbSelectDialog()
{
    m_buttons.clear();
}

void GUsbSelectDialog::usbButtonClicked()
{
    this->setFocus();
    QPushButton *button = static_cast<QPushButton *>(sender());
    if(m_buttons.contains(button))
        m_currentDev = m_buttons.value(button);
    accept();
}

void GUsbSelectDialog::cancelButtonClicked()
{
    this->setFocus();
    reject();
}

void GUsbSelectDialog::paintEvent(QPaintEvent */*event*/)
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

//    QColor color(0, 0, 0, 50);
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
