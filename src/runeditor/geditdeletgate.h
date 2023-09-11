#ifndef GEDITDELETGATE_H
#define GEDITDELETGATE_H

#include <QStyledItemDelegate>

class GRightAlignDeletgate : public QStyledItemDelegate
{
public:
    explicit GRightAlignDeletgate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class GEditIndicatorDeletgate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GEditIndicatorDeletgate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEnable(bool enable);
signals:
    void beginEditing();
protected:
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    bool m_enable;
};

class GCheckIndicatorDeletgate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GCheckIndicatorDeletgate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEnable(bool enable);
signals:
    void checkChanged(int row, bool checked);

protected:
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    bool m_enable;
};

#endif // GEDITDELETGATE_H
