#ifndef GCHANNELSETDELEGATE_H
#define GCHANNELSETDELEGATE_H

#include <QStyledItemDelegate>

class GFluorEditor;
class GChannelSetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GChannelSetDelegate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void setEnable(bool enable);
signals:
    void editCtrl(bool start) const;
    void itemEdited(int row, const QByteArray &probe) const;
    void itemUnableClicked() const;
    void keyPressed() const;
protected:
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    bool m_enable;
    mutable bool m_editing;
};

#endif // GCHANNELSETDELEGATE_H
