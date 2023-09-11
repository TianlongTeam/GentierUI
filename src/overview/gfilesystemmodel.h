#ifndef GFILESYSTEMMODEL_H
#define GFILESYSTEMMODEL_H

#include <QFileSystemModel>

class GDataPool;
class GFileSystemModel : public QFileSystemModel{
    Q_OBJECT
public:
    explicit GFileSystemModel(GDataPool *dataPool, QObject *parent = 0);
    ~GFileSystemModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
private:
    GDataPool *m_pool;
};

#endif // GFILESYSTEMMODEL_H
