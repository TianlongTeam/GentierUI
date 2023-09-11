#include "gfilesystemmodel.h"

#include "gdatapool.h"
#include <QDateTime>
#include <QtMath>
#include <QDebug>

GFileSystemModel::GFileSystemModel(GDataPool *dataPool, QObject *parent) \
    : QFileSystemModel(parent)
    , m_pool(dataPool)
{
}

GFileSystemModel::~GFileSystemModel()
{
    m_pool = NULL;
}

QVariant GFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (section == 0) {
            // ### TODO oh man this is ugly and doesn't even work all the way!
            // it is still 2 pixels off
            QImage pixmap(16, 1, QImage::Format_Mono);
            pixmap.fill(0);
            pixmap.setAlphaChannel(pixmap.createAlphaMask());
            return pixmap;
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case 0: returnValue = tr("Name");
            break;
    case 1: returnValue = tr("Size");
            break;
    case 2: returnValue =
#ifdef Q_OS_MAC
                   tr("Kind", "Match OS X Finder");
#else
                   tr("Type", "All other platforms");
#endif
           break;
    // Windows   - Type
    // OS X      - Kind
    // Konqueror - File Type
    // Nautilus  - Type
    case 3: returnValue = tr("Date Created");
            break;
    default: return QVariant();
    }
    return returnValue;
}

QVariant GFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();    

    if(index.column() == 1 && role == Qt::DisplayRole){       
        if(!this->isDir(index)){
            //如果是文件时 文件大小+1
            QString fn = m_pool->filePath + this->data(this->index(index.row(),0,index.parent()),role).toString();
            qint64 bytes = QFileInfo(fn).size();

            const qint64 kb = 1024;
            const qint64 mb = 1024 * kb;
            const qint64 gb = 1024 * mb;
            const qint64 tb = 1024 * gb;
            if (bytes >= tb)
                return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
            if (bytes >= gb)
                return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
            if (bytes >= mb)
                return QString("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
            if (bytes >= kb)
                return QString("%1 KB").arg(qCeil(qreal(bytes) / kb));
            return QString("%1 bytes").arg(bytes);
        }
    }else if(index.column() == 3 && role == Qt::DisplayRole){
        QString formatTxt = m_pool->dateFormat + " " + (m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP");
        return QFileInfo(filePath(index)).created().toString(formatTxt);
    }
    return QFileSystemModel::data(index,role);
}
