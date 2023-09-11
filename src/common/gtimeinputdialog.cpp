#include "gtimeinputdialog.h"
#include "ui_gtimeinputdialog.h"

#include "gglobal.h"

#include <QKeyEvent>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>
#include <QDebug>

GTimeInputDialog::GTimeInputDialog(const QString &title, int minute, int second, bool hasInfinite, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GTimeInputDialog),
    m_focusIndex(0),
    m_hasInfinite(hasInfinite),
    m_savedMin(0),
    m_savedSec(0)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint);

    ui->panelButton_Infinite->setVisible(m_hasInfinite);

    bool isInfinite = m_hasInfinite && minute==0 && second==0;
    ui->labelTitle->setText(title);    
    if(isInfinite){
        ui->panelButton_Infinite->setChecked(true);
        ui->panelButton_Infinite->setDown(true);

        ui->lineEditMinute->setVisible(false);
        ui->labelColon->setVisible(false);
        ui->buttonBackspace->setEnabled(false);

        QFont font = ui->lineEditSecond->font();
        font.setPointSize(20);
        ui->lineEditSecond->setFont(font);

        m_savedMin = 0;
        m_savedSec = 30;
        ui->lineEditSecond->setText("∞");

        ui->lineEditSecond->setReadOnly(true);
    }else{
        ui->lineEditMinute->setText(QString::number(minute).rightJustified(2,'0',true));
        ui->lineEditSecond->setText(QString::number(second).rightJustified(2,'0',true));
    }

    signalMapper.setMapping(ui->panelButton_1, ui->panelButton_1->text());
    signalMapper.setMapping(ui->panelButton_2, ui->panelButton_2->text());
    signalMapper.setMapping(ui->panelButton_3, ui->panelButton_3->text());
    signalMapper.setMapping(ui->panelButton_4, ui->panelButton_4->text());
    signalMapper.setMapping(ui->panelButton_5, ui->panelButton_5->text());
    signalMapper.setMapping(ui->panelButton_6, ui->panelButton_6->text());
    signalMapper.setMapping(ui->panelButton_7, ui->panelButton_7->text());
    signalMapper.setMapping(ui->panelButton_8, ui->panelButton_8->text());
    signalMapper.setMapping(ui->panelButton_9, ui->panelButton_9->text());    
    signalMapper.setMapping(ui->panelButton_0, ui->panelButton_0->text());

    connect(ui->panelButton_1, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_2, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_3, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_4, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_5, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_6, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_7, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_8, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_9, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_0, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));

    connect(&signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(buttonClicked(QString)));

    ui->lineEditMinute->installEventFilter(this);
    ui->lineEditSecond->installEventFilter(this);

    if(!isInfinite){
        ui->lineEditSecond->setFocus();
        ui->lineEditSecond->selectAll();
    }
}

GTimeInputDialog::~GTimeInputDialog()
{
    delete ui;
}

void GTimeInputDialog::setIntRange(int bottom, int top)
{
    int btmMinute = bottom / 60;
    int btmSecond = bottom % 60;
    int topMinute = top / 60;
    int topSecond = top % 60;
    QString txt = QLatin1String("{") + \
            QString::number(btmMinute).rightJustified(2,'0',true) + QLatin1String(":") + QString::number(btmSecond).rightJustified(2,'0',true) + \
            QLatin1String("-") + \
            QString::number(topMinute).rightJustified(2,'0',true) + QLatin1String(":") + QString::number(topSecond).rightJustified(2,'0',true) + \
            QLatin1String("}");
    ui->labelRange->setText(txt);

    QIntValidator *validater = new QIntValidator(0, topMinute, this);
    ui->lineEditMinute->setValidator(validater);

    validater = new QIntValidator(0, (top<60?top:59), this);
    ui->lineEditSecond->setValidator(validater);
}

QVariant GTimeInputDialog::value() const
{
    int val = -1;
    if(!ui->panelButton_Infinite->isChecked()){
        val = ui->lineEditMinute->text().toInt()*60+ui->lineEditSecond->text().toInt();
    }
    return QVariant(val);
}

void GTimeInputDialog::paintEvent(QPaintEvent */*event*/)
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

bool GTimeInputDialog::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == ui->lineEditMinute){
        switch(ev->type()){
        case QEvent::MouseButtonPress:
            m_focusIndex = 1;
            break;
        case QEvent::MouseButtonRelease:
            ui->lineEditMinute->selectAll();
            break;
        default:break;
        }
    }else if(obj == ui->lineEditSecond){
        switch(ev->type()){
        case QEvent::MouseButtonPress:
            m_focusIndex = 0;
            break;
        case QEvent::MouseButtonRelease:
            ui->lineEditSecond->selectAll();
            break;
        default:break;
        }
    }

    return QDialog::eventFilter(obj,ev);
}

void GTimeInputDialog::on_buttonBackspace_clicked()
{
    emit keyPressed();

    if(m_focusIndex & 0x01){
        ui->lineEditMinute->backspace();
    }else{
        ui->lineEditSecond->backspace();
    }
}

void GTimeInputDialog::on_panelButton_Infinite_clicked()
{
    emit keyPressed();

    bool showInfinite = ui->panelButton_Infinite->isChecked();

    ui->panelButton_Infinite->setDown(showInfinite);

    ui->lineEditMinute->setVisible(!showInfinite);
    ui->labelColon->setVisible(!showInfinite);
    ui->buttonBackspace->setEnabled(!showInfinite);

    QFont font = ui->lineEditSecond->font();
    font.setPointSize(showInfinite?20:11);
    ui->lineEditSecond->setFont(font);

    if(showInfinite){
        m_savedMin = ui->lineEditMinute->text().trimmed().toInt();
        m_savedSec = ui->lineEditSecond->text().trimmed().toInt();
        ui->lineEditSecond->setText("∞");
    }else{
        ui->lineEditMinute->setText(QString::number(m_savedMin).rightJustified(2,'0',true));
        ui->lineEditSecond->setText(QString::number(m_savedSec).rightJustified(2,'0',true));
    }
    ui->lineEditSecond->setReadOnly(showInfinite);
    ui->lineEditSecond->setFocus();
}

void GTimeInputDialog::on_buttonOk_clicked()
{
    emit keyPressed();
    accept();
}

void GTimeInputDialog::on_buttonCancel_clicked()
{
    emit keyPressed();
    reject();
}

void GTimeInputDialog::buttonClicked(const QString &text)
{
    QLineEdit *lineEdit = (m_focusIndex & 0x01) ? ui->lineEditMinute : ui->lineEditSecond;

    QKeyEvent keyPress(QEvent::KeyPress, text.at(0).unicode(), Qt::NoModifier, QString(text.at(0).unicode()));
    QApplication::sendEvent(lineEdit, &keyPress);

    emit keyPressed();

    QKeyEvent keyRelease(QEvent::KeyPress, text.at(0).unicode(), Qt::NoModifier, QString());
    QApplication::sendEvent(lineEdit, &keyRelease);
}
