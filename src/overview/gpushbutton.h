#ifndef GPUSHBUTTON_H
#define GPUSHBUTTON_H

#include <QPushButton>
#include <QMenu>
#include <QDebug>

class GPushButton : public QPushButton
{
    Q_OBJECT
public:
    enum ShowMode{
        Auto = 0,
        UpLeft,
        UpRight
    };

    explicit GPushButton(QWidget * parent = 0) \
        : QPushButton(parent)
    {
        m_showMode = Auto;
        m_topPadding = 9;
        m_leftPadding = 13;
        m_bottomPadding = 9;
        m_rightPadding = 13;
    }

    void setMenu(QMenu *menu){
        QPushButton::setMenu(menu);

        if(menu){
            menu->setStyleSheet(QString("QMenu{border: 1px solid #2f74d0;}\nQMenu::item{padding: %1px %2px %3px %4px;}\nQMenu::item:disabled{background: #c9caca;}").arg(m_topPadding).arg(m_leftPadding).arg(m_bottomPadding).arg(m_rightPadding));

            if(m_showMode != Auto){
                qDebug() << "GPushButton setMenu" << m_showMode;
                disconnect(this, SIGNAL(pressed()), 0, 0);
                connect(this, SIGNAL(pressed()), this, SLOT(g_showMenu()));
            }
        }
    }

    void setShowMode(ShowMode mode = Auto){
        m_showMode = mode;
    }

    void setMargin(int top=9, int left=20, int bottom=9, int right=20){
        m_topPadding = top;
        m_leftPadding = left;
        m_bottomPadding = bottom;
        m_rightPadding = right;
    }

private slots:
    void g_showMenu(){
        if(menu() == NULL || m_showMode == Auto)  return;

        QRect rect = this->geometry();
        rect.setRect(rect.x()-this->x(), rect.y()-this->y(), rect.width(), rect.height());

        QPoint point;
        if(m_showMode == UpRight){
            point = this->mapToGlobal(rect.topRight());
        }else{
            point = this->mapToGlobal(rect.topLeft());
        }

        QSize menuSize = menu()->sizeHint();

        if(m_showMode == UpRight){
            point.rx() -= menuSize.width();
        }
        point.ry() -= menuSize.height();

        qDebug() << "GPushButton g_showMenu" << m_showMode << point << this->isDown();

        menu()->exec(point);

        this->setDown(false);
    }

private:
    ShowMode m_showMode;
    int m_topPadding,m_leftPadding,m_bottomPadding,m_rightPadding;
};

#endif // GPUSHBUTTON_H
