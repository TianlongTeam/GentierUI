/*!
* \file ginputstringdialog.cpp
* \brief ARM板软件中字符串输入对话框
*
*实现了ARM板软件中字符串的输入功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-08 11:42
*
*/

#include "ginputstringdialog.h"
#include "ui_ginputstringdialog.h"

#include <QFileInfo>
#include "mymessagebox.h"
#include <QDir>
#include <QDebug>

GInputStringDialog::GInputStringDialog(const QString &suffix, const QString &path, const QString &text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GInputStringDialog),
    m_defaultSuffix(suffix)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint);
    setStyleSheet("background-color: rgb(217,215,211)");

    signalMapper.setMapping(ui->toolButton_0, ui->toolButton_0);
    signalMapper.setMapping(ui->toolButton_1, ui->toolButton_1);
    signalMapper.setMapping(ui->toolButton_2, ui->toolButton_2);
    signalMapper.setMapping(ui->toolButton_3, ui->toolButton_3);
    signalMapper.setMapping(ui->toolButton_4, ui->toolButton_4);
    signalMapper.setMapping(ui->toolButton_5, ui->toolButton_5);
    signalMapper.setMapping(ui->toolButton_6, ui->toolButton_6);
    signalMapper.setMapping(ui->toolButton_7, ui->toolButton_7);
    signalMapper.setMapping(ui->toolButton_8, ui->toolButton_8);
    signalMapper.setMapping(ui->toolButton_9, ui->toolButton_9);

    signalMapper.setMapping(ui->toolButton_a, ui->toolButton_a);
    signalMapper.setMapping(ui->toolButton_b, ui->toolButton_b);
    signalMapper.setMapping(ui->toolButton_c, ui->toolButton_c);
    signalMapper.setMapping(ui->toolButton_d, ui->toolButton_d);
    signalMapper.setMapping(ui->toolButton_e, ui->toolButton_e);
    signalMapper.setMapping(ui->toolButton_f, ui->toolButton_f);
    signalMapper.setMapping(ui->toolButton_g, ui->toolButton_g);
    signalMapper.setMapping(ui->toolButton_h, ui->toolButton_h);
    signalMapper.setMapping(ui->toolButton_i, ui->toolButton_i);
    signalMapper.setMapping(ui->toolButton_j, ui->toolButton_j);
    signalMapper.setMapping(ui->toolButton_k, ui->toolButton_k);
    signalMapper.setMapping(ui->toolButton_l, ui->toolButton_l);
    signalMapper.setMapping(ui->toolButton_m, ui->toolButton_m);
    signalMapper.setMapping(ui->toolButton_n, ui->toolButton_n);
    signalMapper.setMapping(ui->toolButton_o, ui->toolButton_o);
    signalMapper.setMapping(ui->toolButton_p, ui->toolButton_p);
    signalMapper.setMapping(ui->toolButton_q, ui->toolButton_q);
    signalMapper.setMapping(ui->toolButton_r, ui->toolButton_r);
    signalMapper.setMapping(ui->toolButton_s, ui->toolButton_s);
    signalMapper.setMapping(ui->toolButton_t, ui->toolButton_t);
    signalMapper.setMapping(ui->toolButton_u, ui->toolButton_u);
    signalMapper.setMapping(ui->toolButton_v, ui->toolButton_v);
    signalMapper.setMapping(ui->toolButton_w, ui->toolButton_w);
    signalMapper.setMapping(ui->toolButton_x, ui->toolButton_x);
    signalMapper.setMapping(ui->toolButton_y, ui->toolButton_y);
    signalMapper.setMapping(ui->toolButton_z, ui->toolButton_z);

    signalMapper.setMapping(ui->toolButton_leftBracket, ui->toolButton_leftBracket);
    signalMapper.setMapping(ui->toolButton_rightBracket, ui->toolButton_rightBracket);

    signalMapper.setMapping(ui->toolButton_dot, ui->toolButton_dot);
    signalMapper.setMapping(ui->toolButton_rod, ui->toolButton_rod);

    connect(ui->toolButton_0, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_1, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_2, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_3, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_4, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_5, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_6, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_7, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_8, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_9, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    connect(ui->toolButton_a, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_b, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_c, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_d, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_e, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_f, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_g, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_h, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_i, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_j, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_k, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_l, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_m, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_n, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_o, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_p, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_q, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_r, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_s, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_t, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_u, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_v, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_w, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_x, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_y, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_z, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    connect(ui->toolButton_leftBracket, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_rightBracket, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    connect(ui->toolButton_dot, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->toolButton_rod, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    connect(&signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(buttonClicked(QWidget*)));

    ui->label->setText(tr("New Experiment Name:"));

    //如果文件路径为空，设置为当前路径
    if(path.isEmpty()){
        m_path = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator());
    }else{
        m_path = path;
    }

    m_fileName = QString::null;
    m_textSelected = !text.isEmpty();

    ui->lineEdit->setText(text);
    ui->lineEdit->selectAll();
    ui->lineEdit->setFocus();
}

GInputStringDialog::~GInputStringDialog()
{
    delete ui;
}

QString GInputStringDialog::input() const
{
    return m_fileName;
}

void GInputStringDialog::setMode(InputType type)
{
    if(type == IT_Name){
        //如果是输入设备名称时
        ui->label->setText(tr("Device Name:"));
        ui->toolButton_leftBracket->setEnabled(false);
        ui->toolButton_rightBracket->setEnabled(false);
        ui->toolButton_dot->setEnabled(false);
        ui->toolButton_rod->setEnabled(false);
//        ui->pushButton_blank->setEnabled(false);
    }
}

void GInputStringDialog::buttonClicked(QWidget *w)
{
    emit keyPressed();

    QString txt = w->property("buttonValue").toString();
    if(ui->toolButton_shift->isChecked())
        txt = txt.toUpper();

    if(m_textSelected){
        m_textSelected = false;
    }else{
        txt = ui->lineEdit->text() + txt;
    }

    ui->lineEdit->setText(txt);
    ui->lineEdit->setFocus();
}

void GInputStringDialog::on_toolButton_shift_clicked()
{
    emit keyPressed();

    bool isUpper = ui->toolButton_shift->isChecked();

    ui->toolButton_a->setText(isUpper?"A":"a");
    ui->toolButton_b->setText(isUpper?"B":"b");
    ui->toolButton_c->setText(isUpper?"C":"c");
    ui->toolButton_d->setText(isUpper?"D":"d");
    ui->toolButton_e->setText(isUpper?"E":"e");
    ui->toolButton_f->setText(isUpper?"F":"f");
    ui->toolButton_g->setText(isUpper?"G":"g");
    ui->toolButton_h->setText(isUpper?"H":"h");
    ui->toolButton_i->setText(isUpper?"I":"i");
    ui->toolButton_j->setText(isUpper?"J":"j");
    ui->toolButton_k->setText(isUpper?"K":"k");
    ui->toolButton_l->setText(isUpper?"L":"l");
    ui->toolButton_m->setText(isUpper?"M":"m");
    ui->toolButton_n->setText(isUpper?"N":"n");
    ui->toolButton_o->setText(isUpper?"O":"o");
    ui->toolButton_p->setText(isUpper?"P":"p");
    ui->toolButton_q->setText(isUpper?"Q":"q");
    ui->toolButton_r->setText(isUpper?"R":"r");
    ui->toolButton_s->setText(isUpper?"S":"s");
    ui->toolButton_t->setText(isUpper?"T":"t");
    ui->toolButton_u->setText(isUpper?"U":"u");
    ui->toolButton_v->setText(isUpper?"V":"v");
    ui->toolButton_w->setText(isUpper?"W":"w");
    ui->toolButton_x->setText(isUpper?"X":"x");
    ui->toolButton_y->setText(isUpper?"Y":"y");
    ui->toolButton_z->setText(isUpper?"Z":"z");

    ui->toolButton_rod->setText(isUpper?"_":"-");
    ui->toolButton_rod->setProperty("buttonValue", (isUpper?"_":"-"));
}

void GInputStringDialog::on_toolButton_backspace_clicked()
{    
    emit keyPressed();

    int count = ui->lineEdit->text().count();
    if(count > 0){
        QString txt;
        if(!m_textSelected)
            txt = ui->lineEdit->text().left(count-1);        

        ui->lineEdit->setText(txt);
    }

    m_textSelected = false;
    ui->lineEdit->setFocus();
}

void GInputStringDialog::on_toolButton_blank_clicked()
{
    emit keyPressed();

    QString txt = " ";

    if(m_textSelected){
        m_textSelected = false;
    }else{
        txt = ui->lineEdit->text() + txt;
    }

    m_textSelected = false;
    ui->lineEdit->setText(txt);
    ui->lineEdit->setFocus();
}

void GInputStringDialog::on_buttonOk_clicked()
{
    emit keyPressed();

    QString txt = ui->lineEdit->text().trimmed();
    if(txt.isEmpty()) return;

    QFileInfo fileInfo(txt);
    m_fileName = txt + "." + ((fileInfo.suffix() == m_defaultSuffix) ? QString::null : m_defaultSuffix);

    if(QFile::exists(m_path+m_fileName)){

        My_MessageBox mb;
        connect(&mb, SIGNAL(finished(int)), this, SIGNAL(keyPressed()));
        mb.ginformation(NULL, tr("New"), tr("File %1 already exists.").arg(m_fileName));
        m_textSelected = true;
        ui->lineEdit->selectAll();
        ui->lineEdit->setFocus();
        return;
    }

    accept();
}

void GInputStringDialog::on_buttonCancel_clicked()
{
    emit keyPressed();

    reject();
}
