#include "gcenterdelegate.h"

#include <QPainter>

GCenterDelegate::GCenterDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void GCenterDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }

    painter->drawText(option.rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());

    painter->restore();
}
