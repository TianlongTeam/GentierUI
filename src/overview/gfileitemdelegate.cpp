/*!
* \file gfileitemdelegate.cpp
* \brief ARM板软件中实验管理界面文件显示代理文件
*
*实现了ARM板软件实验文件两种方式的显示
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-02 10:38
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gfileitemdelegate.h"

#include <QPainter>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------

/*!
* \class GFileIconDelegate
* \brief ARM板实验管理界面图标显示方式代理类
*
* 实验文件的图标显示
*/

/*!
* \brief 类GFileIconDelegate的构造函数
* \param parent = NULL
* \return 无
*/
GFileIconDelegate::GFileIconDelegate(int width, int height, QObject *parent) :
    QStyledItemDelegate(parent),
    IWIDTH(width),
    IHEIGHT(height)
{
}

/*!
* \brief 类GFileIconDelegate的构造函数
* \param painter 画刷指针
* \param option  控件子部件
* \param index   控件子部件的序号
* \return 无
*/
void GFileIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }

    QFont font = painter->font();
    font.setPointSize(ICON_FONT_POINTSIZE);
    painter->setFont(font);

    QFontMetrics fontMetrics = painter->fontMetrics();

    int type = 0;

    QFileInfo fileInfo(index.data().toString());
    QString fileName = fileInfo.fileName();
    if(fileInfo.suffix().toLower() == QString("tlpp"))
        type = 1;
    else if(fileInfo.suffix().toLower() == QString("tlpd"))
        type = 2;
#ifdef DEVICE_TYPE_TL13
else if(fileName != QString("..")){
        type = 3;
    }else{
        type =4;
    }
#endif
    QString txt = fontMetrics.elidedText(type>2?fileName:fileInfo.completeBaseName(), Qt::ElideRight, option.rect.width()-8);

    int dw = (option.rect.width() - PIXMAP_WIDTH) / 2;

//    option.rect.height()
//    int dh = (option.rect.height() - PIXMAP_WIDTH - fontMetrics.height()) / 3;
    int dh = 5;


    QRect rect(option.rect.left()+dw, option.rect.top()+dh, PIXMAP_WIDTH, PIXMAP_HEIGHT);
    switch(type){
    case 1:painter->drawPixmap(rect, QPixmap(":/png/tlppfile"));break;
    case 2:painter->drawPixmap(rect, QPixmap(":/png/tlpdfile"));break;
#ifdef DEVICE_TYPE_TL13
    case 3:painter->drawPixmap(rect, QPixmap(":/png/folder"));break;
    case 4:{
            int hg=75;
         option.rect.height()== hg;
         qDebug()<<"----png/back-------hg---"<<  option.rect.height();
        painter->drawPixmap(rect, QPixmap(":/png/back"));
        qDebug()<<"----png/back------------------ rect ------------------"<<rect<<dh<<fontMetrics.height()<<option.rect.height() ;
        break;}
#endif
    default:painter->drawPixmap(rect, QPixmap(":/png/unknowfile"));break;
    }

    QRect txtRect(option.rect.left()+4, rect.bottom()+2, option.rect.width()-8, option.rect.height()-2*dh-PIXMAP_HEIGHT);
    painter->drawText(txtRect, Qt::AlignHCenter | Qt::AlignVCenter, txt);

    painter->restore();
}


/////////////////////////////////////////////////////////////////////////////////////

/*!
* \class GFileDetailDelegate
* \brief ARM板实验管理界面列表显示方式代理类
*
* 实验文件的列表显示
*/

/*!
* \brief 类GFileDetailDelegate的构造函数
* \param parent = NULL
* \return 无
*/
GFileDetailDelegate::GFileDetailDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

/*!
* \brief 类GFileDetailDelegate的构造函数
* \param painter 画刷指针
* \param option  控件子部件
* \param index   控件子部件的序号
* \return 无
*/
void GFileDetailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if(option.state & QStyle::State_Selected){      
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.windowText().color());
    }

    QStyleOptionViewItem  view_option(option);
    if (view_option.state & QStyle::State_HasFocus) {
        view_option.state = view_option.state ^ QStyle::State_HasFocus;
    }

    int column = index.column();
    if(column == 0){    //如果是文件名，去掉扩展名tlpp、tlpd
        QFontMetrics fontMetrics = painter->fontMetrics();

        int type = 0;
        QFileInfo fileInfo(index.data().toString());
        QString fileName = fileInfo.fileName();
        if(fileInfo.suffix().toLower() == QString("tlpp"))
            type = 1;
        else if(fileInfo.suffix().toLower() == QString("tlpd"))
            type = 2;
#ifdef DEVICE_TYPE_TL13
        else if(fileName != QString("..")){
            type = 3;
        }else{
      qDebug()<<" --------------------------- else if(fileName != QString())----------------------------"<<fileName;
            type = 4;
        }
#endif
        QString txt = fontMetrics.elidedText(type>2?fileName:fileInfo.completeBaseName(), Qt::ElideRight, option.rect.width()-8);

        int dh = option.rect.height() - 4;
        int dw = dh * PIXMAP_WIDTH / PIXMAP_HEIGHT;
        QRect rect(option.rect.left()+2, option.rect.top()+2, dw, dh);

        switch(type){
        case 1:painter->drawPixmap(rect, QPixmap(":/png/tlppfile"));break;
        case 2:painter->drawPixmap(rect, QPixmap(":/png/tlpdfile"));break;
#ifdef DEVICE_TYPE_TL13
        case 3:painter->drawPixmap(rect, QPixmap(":/png/folder"));break;
        case 4:painter->drawPixmap(rect, QPixmap(":/png/back"));break;
#endif
        default:painter->drawPixmap(rect, QPixmap(":/png/unknowfile"));break;
        }

        QRect txtRect(option.rect.left()+4+dw, option.rect.top(), option.rect.width()-4-dh, option.rect.height());
        painter->drawText(txtRect, Qt::AlignLeft | Qt::AlignVCenter, txt);
    }else{
        QString txt = index.data(Qt::DisplayRole).toString();

        //如果是文件类型,水平居中
        int flag = (column == 2)?Qt::AlignCenter:(Qt::AlignLeft|Qt::AlignVCenter);
        painter->drawText(option.rect, flag, txt);
    }
    painter->restore();
}
