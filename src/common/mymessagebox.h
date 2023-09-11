/*****************************************************************************
**  关于返回值   :warning和information 按钮只有一个,返回值为0
**             :question按钮有2个, 点击前一个按钮(yes)返回0, 点击第二个按钮(no)返回1
******************************************************************************/
#ifndef MY_MESSAGEBOX_H
#define MY_MESSAGEBOX_H
#include <QMessageBox>
#include <QPixmap>
#include <QIcon>
#include <QPushButton>
#include <QDesktopWidget>
#include <QToolButton>
#include <QDebug>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

#include "gdatapool.h"

class My_MessageBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit My_MessageBox(QWidget *parent = 0) : QMessageBox(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }

    static int showMessageBox(QWidget *parent, const QString &title, const QString &text, int flag, QString button1="", QString button2="", QString button3 ="")
    {
        QMessageBox box(parent);
        box.setWindowTitle(title);
        QString IconPath;
        //! 2011-6-10
        QToolButton *toolButton1 = NULL;
        QToolButton *toolButton2 = NULL;
        QToolButton *toolButton3 = NULL;
        QToolButton *toolButton4 = NULL;
//*******************************1***********************************/
        QPushButton MyPushButton1(button1);                         //
        QPushButton MyPushButton2(button2);                         //
        QPushButton MyPushButton3(button3);

        MyPushButton1.setFixedHeight(60);                           //
        MyPushButton2.setFixedHeight(60);                           //
        MyPushButton3.setFixedHeight(60);

        MyPushButton1.setFocusPolicy(Qt::NoFocus);
        MyPushButton2.setFocusPolicy(Qt::NoFocus);
        MyPushButton3.setFocusPolicy(Qt::NoFocus);

        if(button1.size() > button2.size())                         //
        {                                                           //
            MyPushButton1.setFixedWidth(button1.size() * 17);        //
            if(MyPushButton1.width()<100)                           //
            {                                                       //
                MyPushButton1.setFixedWidth(100);                   //
                MyPushButton3.setFixedWidth(100);
            }                                                       //
            MyPushButton2.setFixedWidth(MyPushButton1.width());     //
            MyPushButton3.setFixedWidth(MyPushButton1.width());
        }                                                           //
        else                                                        //
        {                                                           //
            MyPushButton2.setFixedWidth(button2.size() * 17);        //
            if(MyPushButton2.width()<100)                           //
            {                                                       //
                MyPushButton2.setFixedWidth(100);                   //
                MyPushButton3.setFixedWidth(100);
            }                                                       //
            MyPushButton1.setFixedWidth(MyPushButton2.width());     //
            MyPushButton3.setFixedWidth(MyPushButton2.width());
        }                                                           //
//*******************************************************************/
        switch(flag)
        {
        case 1:
            {
                IconPath = ":/png/information";
                box.addButton(&MyPushButton1, QMessageBox::YesRole);
            }
            break;
        case 2:
            {
                IconPath = ":/png/warning";
                box.addButton(&MyPushButton1, QMessageBox::YesRole);
            }
            break;
        case 3:
            {
                box.addButton(&MyPushButton1, QMessageBox::YesRole);
                if(!MyPushButton3.text().isEmpty())
                {
                    box.addButton(&MyPushButton2, QMessageBox::YesRole);
                    box.addButton(&MyPushButton3, QMessageBox::NoRole);
                }
                else
                    box.addButton(&MyPushButton2, QMessageBox::NoRole);
                IconPath = ":/png/question";
            }
            break;
            case 4:
            {
                toolButton1 = new QToolButton;
                toolButton2 = new QToolButton;
                toolButton3 = new QToolButton;
                toolButton4 = new QToolButton;

                toolButton1->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton2->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton3->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton4->setStyleSheet("QToolButton::hover {background-color: transparent;}");

                toolButton1->setText(button1);
                toolButton2->setText(button2);
                toolButton3->setText(button3);
                toolButton4->setText(tr("Cancel"));

                toolButton1->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton3->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton4->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

                toolButton1->setIcon(QIcon(":/images/restart.png"));
                toolButton2->setIcon(QIcon(":/images/shutdown_1.png"));
                toolButton3->setIcon(QIcon(":/images/logout.png"));
                toolButton4->setIcon(QIcon(":/images/cancel.png"));

                toolButton1->setFocusPolicy(Qt::NoFocus);
                toolButton2->setFocusPolicy(Qt::NoFocus);
                toolButton3->setFocusPolicy(Qt::NoFocus);
                toolButton4->setFocusPolicy(Qt::NoFocus);

                toolButton1->setFixedSize(100, 100);
                toolButton2->setFixedSize(100, 100);
                toolButton3->setFixedSize(100, 100);
                toolButton4->setFixedSize(100, 100);

                toolButton1->setIconSize(QSize(60, 60));
                toolButton2->setIconSize(QSize(60, 60));
                toolButton3->setIconSize(QSize(60, 60));
                toolButton4->setIconSize(QSize(60, 60));

                toolButton1->setAutoRaise(true);
                toolButton2->setAutoRaise(true);
                toolButton3->setAutoRaise(true);
                toolButton4->setAutoRaise(true);


                box.addButton(toolButton1, QMessageBox::YesRole);
                box.addButton(toolButton2, QMessageBox::YesRole);
//                box.addButton(toolButton3, QMessageBox::YesRole);
                box.addButton(toolButton4, QMessageBox::NoRole);
            }
            break;
        }
        box.setFocus(Qt::BacktabFocusReason);
//        if(!IconPath.isEmpty())
//        {
        QPixmap pixmap(IconPath);
        if(!IconPath.isEmpty())
        {
            box.setIconPixmap(pixmap);
        }
//        }
//        box.setWindowIcon(QIcon(":/images/logo_2.png"));
        if(!text.isEmpty())
            box.setText(text);
//****************************************2*****************************************//
//        box.setStyleSheet("QPushButton {min-height :40px; min-width : 100px}");   //
//        box.setStyleSheet("QMessageBox{background-color : rgb(244,244,250)}");
        box.setStyleSheet("QLabel {\ncolor:#383e83;\n}\nQMessageBox {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n} \nQPushButton {\ncolor:#383e83;\nborder: 2px solid #2f74d0;\nfont: 9pt \"文泉驿等宽微米黑\";\nborder-radius: 11px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n\nQPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}");
//**********************************************************************************//
        int ret = box.exec();
        if(flag == 4 && toolButton1 != NULL)
        {
            delete toolButton1;
            delete toolButton2;
            delete toolButton3;
            delete toolButton4;

            toolButton1 = NULL;
            toolButton2 = NULL;
            toolButton3 = NULL;
            toolButton4 = NULL;
        }

        return ret;
    }

    static int information(QWidget *parent, const QString &title, const QString &text, QString button1=tr("OK"))
    {
        int ret = My_MessageBox::showMessageBox(parent, title, text, 1, button1);
        return ret;
    }


    static int warning(QWidget *parent, const QString &title, const QString &text, QString button1=tr("OK"))
    {
        int ret = My_MessageBox::showMessageBox(parent, title, text, 2, button1);
        return ret;
    }

    static int question(QWidget *parent, const QString &title, const QString &text, QString button1=tr("Yes"), QString button2 = tr("No"), QString button3=tr(""))
    {
        int ret = My_MessageBox::showMessageBox(parent, title, text, 3, button1, button2, button3);
        return ret;
    }

    static int shutdown(QWidget *parent, const QString &title, const QString &text)
    {
        int ret = My_MessageBox::showMessageBox(parent, title, text, 4, tr("Restart"), tr("Shutdown")/*,tr("Cancel")*/);
        return ret;
    }

    int gshowMessageBox(QWidget *parent, const QString &title, const QString &text, int flag, QString button1="", QString button2="", QString button3 ="")
    {
        this->setWindowFlags(Qt::Dialog);
        this->setParent(parent);
        this->setWindowTitle(title);

//        this->setStyleSheet("QMessageBox {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 15px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n QPushButton {\ncolor: #0000ff;\nborder: 2px solid #2f74d0;\nfont: 9pt \"文泉驿等宽微米黑\";\nborder-radius: 11px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n\nQPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}");

        QString IconPath;
        //! 2011-6-10
        QToolButton *toolButton1 = NULL;
        QToolButton *toolButton2 = NULL;
        QToolButton *toolButton3 = NULL;
        QToolButton *toolButton4 = NULL;
//*******************************1***********************************/
        QPushButton MyPushButton1(button1);                         //
        QPushButton MyPushButton2(button2);                         //
        QPushButton MyPushButton3(button3);

        MyPushButton1.setFixedHeight(60);                           //
        MyPushButton2.setFixedHeight(60);                           //
        MyPushButton3.setFixedHeight(60);

        MyPushButton1.setFocusPolicy(Qt::NoFocus);
        MyPushButton2.setFocusPolicy(Qt::NoFocus);
        MyPushButton3.setFocusPolicy(Qt::NoFocus);

        if(button1.size() > button2.size())                         //
        {                                                           //
            MyPushButton1.setFixedWidth(button1.size() * 17);        //
            if(MyPushButton1.width()<100)                           //
            {                                                       //
                MyPushButton1.setFixedWidth(100);                   //
                MyPushButton3.setFixedWidth(100);
            }                                                       //
            MyPushButton2.setFixedWidth(MyPushButton1.width());     //
            MyPushButton3.setFixedWidth(MyPushButton1.width());
        }                                                           //
        else                                                        //
        {                                                           //
            MyPushButton2.setFixedWidth(button2.size() * 17);        //
            if(MyPushButton2.width()<100)                           //
            {                                                       //
                MyPushButton2.setFixedWidth(100);                   //
                MyPushButton3.setFixedWidth(100);
            }                                                       //
            MyPushButton1.setFixedWidth(MyPushButton2.width());     //
            MyPushButton3.setFixedWidth(MyPushButton2.width());
        }                                                           //
//*******************************************************************/
        switch(flag)
        {
        case 1:
            {
                IconPath = ":/png/information";
                this->addButton(&MyPushButton1, QMessageBox::YesRole);
            }
            break;
        case 2:
            {
                IconPath = ":/png/warning";
                this->addButton(&MyPushButton1, QMessageBox::YesRole);
            }
            break;
        case 3:
            {
                this->addButton(&MyPushButton1, QMessageBox::YesRole);
                if(!MyPushButton3.text().isEmpty())
                {
                    this->addButton(&MyPushButton2, QMessageBox::YesRole);
                    this->addButton(&MyPushButton3, QMessageBox::NoRole);
                }
                else
                    this->addButton(&MyPushButton2, QMessageBox::NoRole);
                IconPath = ":/png/question";
            }
            break;
            case 4:
            {
                toolButton1 = new QToolButton;
                toolButton2 = new QToolButton;
                toolButton3 = new QToolButton;
                toolButton4 = new QToolButton;

                toolButton1->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton2->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton3->setStyleSheet("QToolButton::hover {background-color: transparent;}");
                toolButton4->setStyleSheet("QToolButton::hover {background-color: transparent;}");

                toolButton1->setText(button1);
                toolButton2->setText(button2);
                toolButton3->setText(button3);
                toolButton4->setText(tr("Cancel"));

                toolButton1->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton3->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                toolButton4->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

                toolButton1->setIcon(QIcon(":/images/restart.png"));
                toolButton2->setIcon(QIcon(":/images/shutdown_1.png"));
                toolButton3->setIcon(QIcon(":/images/logout.png"));
                toolButton4->setIcon(QIcon(":/images/cancel.png"));

                toolButton1->setFocusPolicy(Qt::NoFocus);
                toolButton2->setFocusPolicy(Qt::NoFocus);
                toolButton3->setFocusPolicy(Qt::NoFocus);
                toolButton4->setFocusPolicy(Qt::NoFocus);

                toolButton1->setFixedSize(100, 100);
                toolButton2->setFixedSize(100, 100);
                toolButton3->setFixedSize(100, 100);
                toolButton4->setFixedSize(100, 100);

                toolButton1->setIconSize(QSize(60, 60));
                toolButton2->setIconSize(QSize(60, 60));
                toolButton3->setIconSize(QSize(60, 60));
                toolButton4->setIconSize(QSize(60, 60));

                toolButton1->setAutoRaise(true);
                toolButton2->setAutoRaise(true);
                toolButton3->setAutoRaise(true);
                toolButton4->setAutoRaise(true);


                this->addButton(toolButton1, QMessageBox::YesRole);
                this->addButton(toolButton2, QMessageBox::YesRole);
//                box.addButton(toolButton3, QMessageBox::YesRole);
                this->addButton(toolButton4, QMessageBox::NoRole);
            }
            break;
        }
        this->setFocus(Qt::BacktabFocusReason);
//        if(!IconPath.isEmpty())
//        {
        QPixmap pixmap(IconPath);
        if(!IconPath.isEmpty())
        {
            this->setIconPixmap(pixmap);
        }
//        }
//        box.setWindowIcon(QIcon(":/images/logo_2.png"));
        if(!text.isEmpty())
            this->setText(text);
//****************************************2*****************************************//
//        box.setStyleSheet("QPushButton {min-height :40px; min-width : 100px}");   //
//        this->setStyleSheet("QMessageBox{background-color : rgb(244,244,250)}");
        this->setStyleSheet("QLabel {\ncolor:#383e83;\n}\nQMessageBox {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n} \nQPushButton {\ncolor:#383e83;\nborder: 1px solid #2f74d0;\nfont: 11pt \"文泉驿等宽微米黑\";\nborder-radius: 2px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\nmin-width: 80px;\n}\n\nQPushButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QPushButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}");
//**********************************************************************************//
        int ret = this->exec();
        if(flag == 4 && toolButton1 != NULL)
        {
            delete toolButton1;
            delete toolButton2;
            delete toolButton3;
            delete toolButton4;

            toolButton1 = NULL;
            toolButton2 = NULL;
            toolButton3 = NULL;
            toolButton4 = NULL;
        }

        return ret;
    }

    int ginformation(GDataPool *dat_ptr, QWidget *parent, const QString &title, const QString &text, QString button1=tr("OK"))
    {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

        if(dat_ptr){
            connect(this, SIGNAL(finished(int)), dat_ptr, SLOT(screen_sound()));
            dat_ptr->set_ui_busy_state(1);
        }

        int ret = gshowMessageBox(parent, title, text, 1, button1);

        if(dat_ptr){
            this->disconnect(dat_ptr);
        }

        return ret;
    }


    int gwarning(GDataPool *dat_ptr, QWidget *parent, const QString &title, const QString &text, QString button1=tr("OK"))
    {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

        if(dat_ptr){
            connect(this, SIGNAL(finished(int)), dat_ptr, SLOT(screen_sound()));
            dat_ptr->set_ui_busy_state(1);
        }

        int ret = gshowMessageBox(parent, title, text, 2, button1);

        if(dat_ptr){
            this->disconnect(dat_ptr);
        }

        return ret;
    }

    int gquestion(GDataPool *dat_ptr, QWidget *parent, const QString &title, const QString &text, QString button1=tr("Yes"), QString button2 = tr("No"), QString button3=tr(""))
    {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

        if(dat_ptr){
            connect(this, SIGNAL(finished(int)), dat_ptr, SLOT(screen_sound()));            
            dat_ptr->set_ui_busy_state(1);
        }

        int ret = gshowMessageBox(parent, title, text, 3, button1, button2, button3);

        if(dat_ptr){
            this->disconnect(dat_ptr);
        }

        return ret;
    }

    int gshutdown(QWidget *parent, const QString &title, const QString &text)
    {
        int ret = gshowMessageBox(parent, title, text, 4, tr("Restart"), tr("Shutdown")/*,tr("Cancel")*/);
        return ret;
    }

protected:
    void paintEvent(QPaintEvent */*event*/){
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
//            color.setAlpha(150 - qSqrt(i)*50);
            color.setAlpha(255);
            painter.setPen(color);
            painter.drawPath(path);
        }
    }
};

#endif // MY_MESSAGEBOX_H
