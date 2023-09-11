#ifndef GFILECOPYDELEGATE_H
#define GFILECOPYDELEGATE_H

#include <QStyledItemDelegate>

class GFileCopyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GFileCopyDelegate(bool enableSelect = false, QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
    void itemClicked(bool checked);
protected:
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    bool m_enableSelect;
};

#endif // GFILECOPYDELEGATE_H
