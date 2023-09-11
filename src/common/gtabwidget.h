#ifndef GTABWIDGET_H
#define GTABWIDGET_H

#include <QTabBar>
#include <QTabWidget>

class GTabBar : public QTabBar
{
public:
    GTabBar(QWidget * parent = 0) : \
        QTabBar(parent),
        m_width(0),
        m_height(0)
    {}

    void setCustomTabSize(int width, int height){
        m_width = width;
        m_height = height;
    }

protected:
    QSize tabSizeHint(int index) const{
        QSize size;
        size.setWidth(m_width<=0 ? QTabBar::tabSizeHint(index).width() : m_width);
        size.setHeight(m_height<=0 ? QTabBar::tabSizeHint(index).height() : m_height);
        return size;
    }

private:
    int m_width, m_height;
};

class GTabWidget : public QTabWidget
{
public:
    GTabWidget(QWidget *parent = 0) : \
        QTabWidget(parent)
    {
        setTabBar(new GTabBar(this));
    }

    void setCustomTabSize(int width, int height){
        ((GTabBar *)tabBar())->setCustomTabSize(width, height);
    }
};

#endif // GTABWIDGET_H

