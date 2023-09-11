/*!
* \file gfileitemdelegate.h
* \brief ARM板软件中实验管理界面文件显示代理头文件
*
*实现了ARM板软件实验文件两种方式的显示
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-02 10:38
*
*/

#ifndef GFILEITEMDELEGATE_H
#define GFILEITEMDELEGATE_H

#include <QStyledItemDelegate>

#ifdef DEVICE_TYPE_TL22
#define ICON_FONT_POINTSIZE 11
#else
#define ICON_FONT_POINTSIZE 12
#endif

#define PIXMAP_WIDTH    35
#define PIXMAP_HEIGHT   45

//图标显示方式
class GFileIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GFileIconDelegate(int width, int height, QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:

public slots:

private:
    const int IWIDTH;
    const int IHEIGHT;
};

//列表显示方式
class GFileDetailDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GFileDetailDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:

public slots:

};

#endif // GFILEITEMDELEGATE_H
