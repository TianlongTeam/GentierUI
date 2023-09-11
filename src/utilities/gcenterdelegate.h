#ifndef GCENTERDELEGATE_H
#define GCENTERDELEGATE_H

#include <QStyledItemDelegate>

class GCenterDelegate : public QStyledItemDelegate
{
public:
    explicit GCenterDelegate(QObject *parent = 0);
    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // GCENTERDELEGATE_H
