#include "geditdeletgate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

GRightAlignDeletgate::GRightAlignDeletgate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void GRightAlignDeletgate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }    

    QString txt = index.data(Qt::DisplayRole).toString().trimmed();
    QRect rect(option.rect);
    rect.setWidth(option.rect.width()-2);
    painter->drawText(rect, Qt::AlignRight|Qt::AlignVCenter, txt);

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GEditIndicatorDeletgate::GEditIndicatorDeletgate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_enable(true)
{

}

void GEditIndicatorDeletgate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }
    painter->restore();

    QPixmap pixmap;
    pixmap.load(m_enable?":/png/edit":":/png/noedit");
    QPoint point = option.rect.center() - pixmap.rect().center();
    painter->drawPixmap(point, pixmap);
}

void GEditIndicatorDeletgate::setEnable(bool enable)
{
    m_enable = enable;
}

bool GEditIndicatorDeletgate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(m_enable && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton)
        {
            emit beginEditing();
        }
        return true;
    }else
        return QStyledItemDelegate::editorEvent(event, model, option, index);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GCheckIndicatorDeletgate::GCheckIndicatorDeletgate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_enable(true)
{

}

void GCheckIndicatorDeletgate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }
    painter->restore();

    int flag = index.data(Qt::EditRole).toInt();

//    QStyleOptionButton optionButton;
//    QPoint pt = option.rect.center();
//    optionButton.rect = QRect(pt.x()-10, pt.y()-10, 20, 20);
//    optionButton.state = QStyle::State_None;
//    optionButton.state |= (flag & 0x02) ? (QStyle::State_Enabled|QStyle::State_Active) : QStyle::State_None;
//    optionButton.state |= (flag & 0x01) ? QStyle::State_On : QStyle::State_Off;
//    qApp->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &optionButton, painter);

    QPixmap pixmap;
//    pixmap.load(flag!=0?":/png/fluor":":/png/nofluor");
    if(flag!=0){
//        pixmap.load(m_enable?":/png/fluor":":/png/nofluor");
        pixmap.load(":/png/fluor");
        QPoint point = option.rect.center() - pixmap.rect().center();
        painter->drawPixmap(point, pixmap);
    }
}

void GCheckIndicatorDeletgate::setEnable(bool enable)
{
    m_enable = enable;
}

bool GCheckIndicatorDeletgate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(m_enable && event->type() == QEvent::MouseButtonRelease)
    {        
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton)
        {         
            int flag = model->data(index, Qt::EditRole).toInt() & 0x01 ? 0 : 1;
            model->setData(index, flag, Qt::EditRole);
            emit checkChanged(index.row(), flag);
        }
        return true;
    }else
        return QStyledItemDelegate::editorEvent(event, model, option, index);
}
