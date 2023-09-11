#include "gfilecopydelegate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QFileInfo>

#ifdef DEVICE_TYPE_TL22
#define FONTSIZE    11
#else
#define FONTSIZE    12
#endif

GFileCopyDelegate::GFileCopyDelegate(bool enableSelect, QObject *parent) \
    : QStyledItemDelegate(parent)
    , m_enableSelect(enableSelect)
{

}

void GFileCopyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }

    QFont font = painter->font();
    font.setPointSize(FONTSIZE);
    painter->setFont(font);

    int isSelect = index.data(Qt::UserRole+1).toInt();
    int isDir = index.data(Qt::UserRole+2).toInt();
    QFileInfo fileInfo(index.data(Qt::DisplayRole).toString());

    int width1 = 37;
    //添加选择图标
    if(m_enableSelect){
        QRect urect = option.rect;
        urect.setWidth(width1);

        QPixmap pixmap1;
        pixmap1.load(isSelect!=0?":/png/select":":/png/noselect");
        QPoint point1 = urect.center() - pixmap1.rect().center();
        painter->drawPixmap(point1,pixmap1);
    }

    //添加文件图标
    QString filePng;
    if(fileInfo.suffix().toLower()==QString("tlpp")){
        filePng = ":/png/tlppfile";
    }else if(fileInfo.suffix().toLower()==QString("tlpd")){
        filePng = ":/png/tlpdfile";
    }else{
        filePng = ":/png/unknowfile";
    }
    QPixmap pixmap2;
    pixmap2.load(isDir!=0?":/png/folder":filePng);
    int left = 4;
    QRect urect = option.rect;
    urect.setLeft(option.rect.left()+(m_enableSelect?width1:left));
    int height = option.rect.height()-6;
    int width2 = height * pixmap2.width() / pixmap2.height();
    urect.setWidth(width2+2);
    QRect pixRect = urect.adjusted(1,3,-1,-3);
    painter->drawPixmap(pixRect,pixmap2.scaled(pixRect.size(), Qt::KeepAspectRatio));

    //显示文件夹或文件名
    urect = option.rect;
    urect.setLeft(option.rect.left()+(m_enableSelect?width1:left)+width2+4);
    urect.setWidth(option.rect.width()-(m_enableSelect?width1:left)-width2-4);
    painter->drawText(urect, Qt::AlignLeft|Qt::AlignVCenter, fileInfo.completeBaseName());

    painter->restore();
}

bool GFileCopyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.column() == 0 && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton && index.column() == 0)
        {            
            int flag = model->data(index, Qt::UserRole+1).toInt();
            flag = flag==0 ? 1 : 0;
            model->setData(index, flag, Qt::UserRole+1);
            emit itemClicked(flag != 0);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
