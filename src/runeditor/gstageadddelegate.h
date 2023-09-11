#ifndef GSTAGEADDDELEGATE_H
#define GSTAGEADDDELEGATE_H

#include <QStyledItemDelegate>

class GStageAddDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GStageAddDelegate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:

public slots:

};

#endif // GSTAGEADDDELEGATE_H
