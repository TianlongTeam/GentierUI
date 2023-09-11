#include "gdatainputdialog.h"
#include "ui_gdatainputdialog.h"

#include <QKeyEvent>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

GDataInputDialog::GDataInputDialog(const QString &title, const QString &value, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GDataInputDialog),
    m_type(0)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint);

    ui->labelTitle->setText(title);
    ui->lineEdit->setText(value);
    ui->lineEdit->selectAll();

    signalMapper.setMapping(ui->panelButton_1, ui->panelButton_1->text());
    signalMapper.setMapping(ui->panelButton_2, ui->panelButton_2->text());
    signalMapper.setMapping(ui->panelButton_3, ui->panelButton_3->text());
    signalMapper.setMapping(ui->panelButton_4, ui->panelButton_4->text());
    signalMapper.setMapping(ui->panelButton_5, ui->panelButton_5->text());
    signalMapper.setMapping(ui->panelButton_6, ui->panelButton_6->text());
    signalMapper.setMapping(ui->panelButton_7, ui->panelButton_7->text());
    signalMapper.setMapping(ui->panelButton_8, ui->panelButton_8->text());
    signalMapper.setMapping(ui->panelButton_9, ui->panelButton_9->text());
    signalMapper.setMapping(ui->panelButton_dot, ui->panelButton_dot->text());
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
    connect(ui->panelButton_dot, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(ui->panelButton_0, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));

    connect(&signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(buttonClicked(QString)));
}

GDataInputDialog::~GDataInputDialog()
{
    delete ui;
}

void GDataInputDialog::setIntRange(int bottom, int top)
{
    QString txt = tr("{%1-%2}").arg(bottom).arg(top);
    ui->labelRange->setText(txt);

    QIntValidator *validater = new QIntValidator(bottom, top, this);
    ui->lineEdit->setValidator(validater);

    ui->panelButton_line->setVisible(bottom<0);
    ui->panelButton_dot->setVisible(false);
    m_type = 0;
}

void GDataInputDialog::setDoubleRange(double bottom, double top, int decimals)
{
    QString txt = tr("{%1-%2}").arg(QString::number(bottom,'f',1)).arg(QString::number(top,'f',1));
    ui->labelRange->setText(txt);

    QDoubleValidator *validater = new QDoubleValidator(bottom, top, decimals, this);
    validater->setNotation(QDoubleValidator::StandardNotation);
    ui->lineEdit->setValidator(validater);

    ui->panelButton_line->setVisible(bottom<0.0);
    ui->panelButton_dot->setVisible(true);
    m_type = 1;
}

void GDataInputDialog::setPwdMode()
{
    ui->panelButton_dot->setEnabled(false);
    ui->lineEdit->setEchoMode(QLineEdit::Password);
}

QVariant GDataInputDialog::value() const
{
    if(m_type){
        double val = ui->lineEdit->text().toDouble();
        return QVariant(val);
    }else{
        int val = ui->lineEdit->text().toInt();
        return QVariant(val);
    }
//    return QVariant(ui->lineEdit->text());
}

void GDataInputDialog::paintEvent(QPaintEvent */*event*/)
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

void GDataInputDialog::on_buttonBackspace_clicked()
{
    emit keyPressed();

    ui->lineEdit->backspace();
}

void GDataInputDialog::on_panelButton_line_clicked()
{
    emit keyPressed();

    QString txt = ui->lineEdit->text().trimmed();

    if(txt.isEmpty())
        ui->lineEdit->setText("-");
    else{
        if(txt.left(1) == tr("-")){
            txt.remove(0,1);
        }else{
            txt.insert(0,"-");
        }
        ui->lineEdit->setText(txt);
    }
}

void GDataInputDialog::on_buttonOk_clicked()
{
    emit keyPressed();
    accept();
}

void GDataInputDialog::on_buttonCancel_clicked()
{
    emit keyPressed();
    reject();
}

void GDataInputDialog::buttonClicked(const QString &text)
{    
    QKeyEvent keyPress(QEvent::KeyPress, text.at(0).unicode(), Qt::NoModifier, QString(text.at(0).unicode()));
    QApplication::sendEvent(ui->lineEdit, &keyPress);

    emit keyPressed();

    QKeyEvent keyRelease(QEvent::KeyPress, text.at(0).unicode(), Qt::NoModifier, QString());
    QApplication::sendEvent(ui->lineEdit, &keyRelease);
}
