#include "gcolordelegate.h"

#include "gdatapool.h"

#include <QPainter>
#include <QDebug>

GColorDelegate::GColorDelegate(GDataPool *pool, int debug_font_pixel_size, QObject *parent) :
    QStyledItemDelegate(parent),
    m_pool(pool),
    m_pixel_size(debug_font_pixel_size),
    m_maxColorVal(MAX_COLOR_VAL),
    m_midColorVal(MID_COLOR_VAL)
{
}

void GColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int rr = option.rect.width() < option.rect.height() ? option.rect.width() : option.rect.height();
    rr = rr / 2 - 6;

    int r = 0, g = 0, b = 0;
    double val = index.data(Qt::EditRole).toDouble();

    if(val < 0){    //Qt::darkGray
        r = 0x80;
        g = 0x80;
        b = 0x80;
    }else if(val <= m_midColorVal){
        r += (int)(((double)val / m_midColorVal) * 255.0);
        g += (int)(((double)val / m_midColorVal) * 125.0)+130;
        b = 240 - (int)(((double)val / m_midColorVal) * 240.0);
    }else{
        r = 255;
        g = 255 - (int)(((double)(val-m_midColorVal) / m_midColorVal) * 255.0);
        b = 0;
    }

    painter->save();
    QColor cc(r,g,b);
    painter->setBrush(cc);
    painter->fillRect(option.rect, cc);

    painter->restore();
}
