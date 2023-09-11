/*!
* \file gproxymodel.h
* \brief ARM板软件中实验管理界面文件模板的代理模板头文件
*
*实现了文件模板的代理，用于文件的图标显示
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-02 ‏‎16:55
*
*/

#ifndef GPROXYMODEL_H
#define GPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QVector>

class GProxyModel : public QAbstractProxyModel
{
public:
    explicit GProxyModel(int columnMax, const QModelIndex &rootIndex = QModelIndex(), int *deep_ptr = 0, QObject *parent = 0);
    ~GProxyModel();

    virtual QModelIndex	mapFromSource(const QModelIndex &sourceIndex) const;
    virtual QModelIndex	mapToSource(const QModelIndex &proxyIndex) const;
    virtual QItemSelection	mapSelectionFromSource(const QItemSelection &sourceSelection) const;
    virtual QItemSelection	mapSelectionToSource(const QItemSelection &proxySelection) const;

    virtual int	columnCount(const QModelIndex &proxy_parent = QModelIndex()) const;
    virtual int	rowCount(const QModelIndex &proxy_parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &proxy_child) const;
    virtual QModelIndex	index(int row, int column, const QModelIndex &proxy_parent = QModelIndex()) const;

    void setRootIndex(const QModelIndex &rootIndex);
private:
    int register_index(const QModelIndex & source_index) const;

private:
    const int COLUMN_MAX;           ///<图标显示时多少列
    QModelIndex ROOT_INDEX;         ///<文件模板根目录序号
    int* const dir_deep_ptr;        ///<目录深度指针（用于tl13进入子目录时，rowcount+1）

    mutable QVector<QModelIndex> indexPool;     ///<源模板的序号池，用于存储与当前代理模板序号对应的源模板序号
};

#endif // GPROXYMODEL_H
