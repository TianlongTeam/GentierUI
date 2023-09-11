/*!
* \file gproxymodel.cpp
* \brief ARM板软件中实验管理界面文件模板的代理模板文件
*
*实现了文件模板的代理，用于文件的图标显示
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-02 ‏‎16:55
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gproxymodel.h"

#include <QDebug>
#include <QItemSelection>


//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------

/*!
* \class GProxyModel
* \brief 文件显示的代理模板
*
* 实验文件的图标方式显示
*/

/*!
* \brief 类GProxyModel的构造函数
* \param columnMax 图标显示时的列数
* \param rootIndex 源模板中根目录的序号
* \param parent = NULL
* \return 无
*/
GProxyModel::GProxyModel(int columnMax, const QModelIndex &rootIndex, int *deep_ptr, QObject *parent) :
    QAbstractProxyModel(parent),
    COLUMN_MAX(columnMax),
    ROOT_INDEX(rootIndex),
    dir_deep_ptr(deep_ptr)
{
}

GProxyModel::~GProxyModel()
{
    indexPool.clear();
}

QModelIndex	GProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(!sourceIndex.isValid())
        return QModelIndex();

    if (sourceIndex.model() != sourceModel()) {
        qWarning() << "GProxyModel: index from wrong model passed to mapFromSource";
        Q_ASSERT(!"GProxyModel: index from wrong model passed to mapFromSource");
        return QModelIndex();
    }

    if(sourceIndex.column() != 0)
        return QModelIndex();

    int count = sourceIndex.row();
    int row = count / COLUMN_MAX;
    int col = count % COLUMN_MAX;

//    qDebug() << "--- mapFromSource" << sourceModel()->index(count,0,ROOT_INDEX);
//    int pos = register_index( sourceIndex );
    int pos = register_index( sourceModel()->index(count,0,ROOT_INDEX) );
    return createIndex(row, col, pos);
}

QModelIndex	GProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{   
    if(!proxyIndex.isValid())
        return QModelIndex();

    qint64 pos = proxyIndex.internalId();
    return indexPool[pos];
}

QItemSelection GProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    if(sourceSelection.indexes().count() > 0){
        int source_row = sourceSelection.first().topLeft().row();

        int row = source_row / COLUMN_MAX;
        int col = source_row % COLUMN_MAX;

        QModelIndex indx = this->index(row,col);
        return QItemSelection(indx, indx);
    }else{
        return QItemSelection();
    }
}

QItemSelection GProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    if(proxySelection.indexes().count() > 0){
        QModelIndex proxy_index = proxySelection.first().topLeft();

        int source_row = proxy_index.row() * COLUMN_MAX + proxy_index.column();
        int source_column = sourceModel()->columnCount(ROOT_INDEX);
        QModelIndex topLeft = sourceModel()->index(source_row, 0, ROOT_INDEX);
        QModelIndex bottomRight = sourceModel()->index(source_row, source_column-1, ROOT_INDEX);
        return QItemSelection(topLeft, bottomRight);
    }else{
        return QItemSelection();
    }
}

int	GProxyModel::columnCount(const QModelIndex &) const
{    
    return sourceModel() == NULL ? 0 : COLUMN_MAX;
}

int	GProxyModel::rowCount(const QModelIndex &) const
{   
    int count = 0;
    QAbstractItemModel *_model = sourceModel();
    if(_model != NULL){

        int rowCount = sourceModel()->rowCount( ROOT_INDEX );

        if(dir_deep_ptr && *dir_deep_ptr!=0){
            rowCount++;
        }

        count = rowCount / COLUMN_MAX;
        int tmp = rowCount % COLUMN_MAX;
        if(tmp != 0) count++;
    }

    return count;
}

QModelIndex GProxyModel::parent(const QModelIndex &) const
{
    return ROOT_INDEX;
}

QModelIndex	GProxyModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || column < 0)
            return QModelIndex();

    QAbstractItemModel *_model = sourceModel();

    if(_model == NULL)
        return QModelIndex();

    int rowCount = _model->rowCount( ROOT_INDEX );

    int source_row = row * COLUMN_MAX + column;
    if(source_row >= rowCount)
        return QModelIndex();

    QModelIndex source_index = _model->index(source_row, 0, ROOT_INDEX);

    int pos = register_index(source_index);

    return createIndex( row, column, pos );
}

void GProxyModel::setRootIndex(const QModelIndex &rootIndex)
{
    ROOT_INDEX = rootIndex;
}

int GProxyModel::register_index(const QModelIndex & source_index) const
{
    int pos = indexPool.indexOf( source_index );
    if ( pos == -1 ){
        indexPool.push_back( source_index );
        pos = indexPool.size() - 1;
    }
    return pos;
}

