#ifndef GCOLORDELEGATE_H
#define GCOLORDELEGATE_H

#include <QStyledItemDelegate>

#define MAX_COLOR_VAL   400.0
#define MID_COLOR_VAL   (MAX_COLOR_VAL / 2)

class GDataPool;
class GColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GColorDelegate(GDataPool *pool, int debug_font_pixel_size = 9, QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setMaxColorValue(double val){
        if(val > MAX_COLOR_VAL){
            m_maxColorVal = val;
            m_midColorVal = val/2.0;
        }
    }

private:
    GDataPool *m_pool; ///< 私有自检报告文件指针
    int m_pixel_size;  ///< debug模式下显示荧光值数据字体的像素大小

    double m_maxColorVal;
    double m_midColorVal;    
};

#endif // GCOLORDELEGATE_H
