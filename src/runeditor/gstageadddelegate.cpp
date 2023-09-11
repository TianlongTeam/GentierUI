#include "gstageadddelegate.h"

#include <QPainter>

//#ifdef DEVICE_TYPE_TL22
//#define STAGEADD_COLUMN_COUNT   3
//#else
#define STAGEADD_COLUMN_COUNT   2
//#endif

#define PIXMAP_WIDTH            100

GStageAddDelegate::GStageAddDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void GStageAddDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }    

    int indx = index.row() * STAGEADD_COLUMN_COUNT + index.column();
    QPixmap pixmap;
    switch(indx){
#ifdef DEVICE_TYPE_TL13
    case 0:pixmap.load(":/png/stageadd1");break;
    case 1:pixmap.load(":/png/stageadd2");break;
    case 2:pixmap.load(":/png/22");break;
    case 3:pixmap.load(":/png/11");break;
    case 4:pixmap.load(":/png/stageadd7");break;
    case 5:pixmap.load(":/png/stageadd8");break;
    case 6:pixmap.load(":/png/stageadd8");break;
    case 7:pixmap.load(":/png/stageadd8");break;
    case 8:pixmap.load(":/png/stageadd9");break;
#else
    case 0:pixmap.load(":/png/stageadd1");break;
    case 1:pixmap.load(":/png/stageadd2");break;
    case 2:pixmap.load(":/png/stageadd3");break;
    case 3:pixmap.load(":/png/stageadd4");break;
    case 4:pixmap.load(":/png/stageadd5");break;
    case 5:pixmap.load(":/png/stageadd6");break;
    case 6:pixmap.load(":/png/stageadd7");break;
    case 7:pixmap.load(":/png/stageadd8");break;
        //    case 8:pixmap.load(":/png/stageadd9");break;
#endif
    default:break;
    }
    QRect rect = option.rect;
//    rect.setLeft(rect.left() + 2);
    rect.setWidth(PIXMAP_WIDTH);
    painter->drawPixmap(rect, pixmap);

    QString txt = index.data(Qt::DisplayRole).toString().trimmed();
    rect.setLeft(rect.left() + PIXMAP_WIDTH + 1);
    rect.setWidth(option.rect.width()-1-PIXMAP_WIDTH);
    painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, txt);

    painter->restore();
}
