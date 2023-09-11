#include "gchannelsetdelegate.h"

#include "gglobal.h"
#include "gfluoreditor.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QTableView>
#include <QFontMetrics>
#include <QTimer>
#include <QDebug>

#ifdef DEVICE_TYPE_TL22
#define FONTSIZE    11
#else
#define FONTSIZE    12
#endif

GChannelSetDelegate::GChannelSetDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_enable(true),
    m_editing(false)
{    
}

void GChannelSetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
        qDebug() << Q_FUNC_INFO << "selected color:" << QString::number(option.palette.highlight().color().rgba(),16);
    }else{
        painter->setPen(option.palette.windowText().color());
    }

    QFont font = painter->font();
    font.setPointSize(FONTSIZE);
    painter->setFont(font);
    //    painter->setPen(Qt::darkBlue);

    switch(index.column()){
    case 0:{    //显示通道号及是否选择控件
        int isSelect = index.data(Qt::EditRole).toInt();
        int width = option.rect.width() / 2;
        QRect urect = option.rect;
        urect.setWidth(width);
        //        QStyleOptionButton optionButton;
        //        optionButton.rect = QRect(urect.center().x()-10, urect.center().y()-10, 20, 20);
        //        optionButton.state = QStyle::State_Enabled|QStyle::State_Active;
        //        optionButton.state |= (isSelect==0)?QStyle::State_Off:QStyle::State_On;
        //        qApp->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &optionButton, painter);
        QPixmap pixmap;
        pixmap.load(isSelect!=0?":/png/select":":/png/noselect");
        QPoint point = urect.center() - pixmap.rect().center();
        painter->drawPixmap(point,pixmap);

        urect = option.rect;
        urect.setLeft(option.rect.left()+width);
        urect.setWidth(width);
        painter->drawText(urect, Qt::AlignCenter, QString::number(index.row()+1));
        break;
    }
    case 1:{
        QString showText;
        QByteArray fluor = index.data(Qt::EditRole).toByteArray();
        for(int i=0; i<fluor.count(); i++){
            if(fluor.at(i)>0 && fluor.at(i) < 255){
                showText += GHelper::keyOfFluor(1, fluor.at(i));
                if(i<fluor.count()-1) showText+=" ; ";
            }
        }
        if(showText.isEmpty()){

            if((GHelper::total_instrument_id == 201 && index.row()==4) || ((GHelper::total_instrument_id==202 || GHelper::total_instrument_id==203) && index.row()==5)){
                showText = GHelper::keyOfFluor(index.row()+1,1);
            }else if( GHelper::total_instrument_id == 204 && index.row()==5 ){
                showText = GHelper::keyOfFluor(index.row()+1,4);
            }else if( GHelper::total_instrument_id == 205 && index.row()==5 ){
                showText = GHelper::keyOfFluor(index.row()+1,5);
            }else{
                showText = GHelper::keyOfFluor(index.row()+1,0);
            }
        }

        font.setPointSize(FONTSIZE);
        painter->setFont(font);

        QFontMetrics metrics(font);
        int width = metrics.width(showText);

        bool hasArrow = GHelper::countOfFluorGroup(index.row()+1) > 1;
        if(width > 410){    //CHANNEL_COL1_WIDTH
            showText = metrics.elidedText(showText,Qt::ElideRight,410);
            width = 410;
        }
        // qDebug()<<"----------探针类型--showText--"<<showText<<hasArrow;
        int left = (option.rect.width()-width)/2;
        QRect urect = option.rect;
        urect.setLeft(option.rect.left()+left);
        urect.setWidth(width);

        painter->drawText(urect, Qt::AlignLeft | Qt::AlignVCenter, showText);

        //如果探针组有多个探针类型，显示下拉控件
        if(hasArrow){
            QStyleOptionHeader optionHeader;
            optionHeader.rect = option.rect;
            optionHeader.rect.setLeft(option.rect.left()+left+width);
            optionHeader.rect.setWidth(20);
            int height = optionHeader.rect.height() / 4;
            optionHeader.rect.setTop(option.rect.top()+height);
            optionHeader.rect.setHeight(3*height);
            qApp->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &optionHeader, painter);
        }

        break;
    }
    default:{   //显示设置值
        painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, index.data().toString());
        break;
    }
    }

    painter->restore();
}

QWidget *GChannelSetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{    
    qDebug() << Q_FUNC_INFO << "00000000000000000" << index;
    int col = index.column();
    if(col == 1){
        if(m_enable){
            if(m_editing) return NULL;
            m_editing = true;

            int groupId = index.row();
            const QAbstractItemModel *model = index.model();
            if(model){
                int left = GHelper::indexOfFluorGroup(groupId+1);
                int right = GHelper::countOfFluorGroup(groupId+1)+left-1;

                QPoint point = parent->mapToGlobal(option.rect.topLeft());

qDebug() << Q_FUNC_INFO << "111111111111111";
                GFluorEditor *editor = new GFluorEditor(left, right, index.data(Qt::EditRole).toByteArray(), parent);
                editor->setRange(point.x(), point.y(), option.rect.width()/*, option.rect.height()*/);//
                connect(editor, SIGNAL(keyPressed()), this, SIGNAL(keyPressed()));
                emit keyPressed();
qDebug() << Q_FUNC_INFO << "22222222222222";
                emit editCtrl(true);
qDebug() << Q_FUNC_INFO << "33333333333333";
                return editor;
            }
        }else{
            emit itemUnableClicked();
        }
    }else if(col == 2){
        emit keyPressed();
    }
    return NULL;
}

void GChannelSetDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int col = index.column();
    if(col == 1){
        GFluorEditor *editor_ = static_cast<GFluorEditor *>(editor);
        Q_ASSERT(editor_);
        if(editor_){
qDebug() << Q_FUNC_INFO << "4444444444444" << index;
            editor_->setCurrentKey(index.data(Qt::EditRole).toByteArray());
qDebug() << Q_FUNC_INFO << "55555555555555";
            editor_->showList();
qDebug() << Q_FUNC_INFO << "6666666666666";
        }
    }
}

void GChannelSetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    int col = index.column();
    if(col == 1){
        GFluorEditor *editor_ = static_cast<GFluorEditor *>(editor);
        Q_ASSERT(editor_);
        if(editor_){
qDebug() << Q_FUNC_INFO << "777777777777" << index;
            QByteArray probe = editor_->currentKey();
            editor_->disconnect(this);
            editor_->hideList();

qDebug() << Q_FUNC_INFO << "88888888888";
            QByteArray val = model->data(index,Qt::EditRole).toByteArray();
            if(probe != val){
qDebug() << Q_FUNC_INFO << "999999999999";
                //如果探针为空,则取消该通道选择
                model->blockSignals(true);
                if(probe.isEmpty()){
                    model->setData(model->index(index.row(),0),QVariant(0),Qt::EditRole);
                }else{
                    if(model->data(model->index(index.row(),0),Qt::EditRole).toInt() == 0){
                        model->setData(model->index(index.row(),0),QVariant(1),Qt::EditRole);
                    }
                }
qDebug() << Q_FUNC_INFO << "10101010101010" << model << GHelper::byteArrayToHexStr(probe) << GHelper::byteArrayToHexStr(val);
                model->setData(index, QVariant::fromValue(probe), Qt::EditRole);
                model->blockSignals(false);
qDebug() << Q_FUNC_INFO << "121212121212" << index.row() << GHelper::byteArrayToHexStr(probe);
                emit itemEdited(index.row(),probe);                
qDebug() << Q_FUNC_INFO << "1313131313131";
            }else{
                emit keyPressed();
            }
qDebug() << Q_FUNC_INFO << "141414141414";
            emit editCtrl(false);
qDebug() << Q_FUNC_INFO << "1515151515151515";
            m_editing = false;
        }
    }
}

void GChannelSetDelegate::setEnable(bool enable)
{
    m_enable = enable;
}

bool GChannelSetDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    qDebug() << Q_FUNC_INFO << m_enable << index.column() << event->type();
    if(index.column() == 0 && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton)
        {
            if(m_enable){
                int flag = model->data(index, Qt::EditRole).toInt();
                flag = flag==0 ? 1 : 0;
                model->setData(index, flag, Qt::EditRole);

                QByteArray probe = model->data(model->index(index.row(),1), Qt::EditRole).toByteArray();
                qDebug()<<" --------------editorEvent-----probe----"<<probe.size();

                if(flag){
                    if(probe.size() == 0){
                        probe.append(GHelper::indexOfFluorGroup(index.row()+1));
                        model->setData(model->index(index.row(),1), probe, Qt::EditRole);
                    }else if(probe.size() > 1){
                        probe.resize(1);
                        model->setData(model->index(index.row(),1), probe, Qt::EditRole);
                    }
                }else{
                    model->setData(model->index(index.row(),1), QVariant(), Qt::EditRole);
                }
                emit itemEdited(index.row(), probe);
            }else{
                emit itemUnableClicked();
            }
        }
        return true;
    }else{
        bool isOk = QStyledItemDelegate::editorEvent(event, model, option, index);
        qDebug() << Q_FUNC_INFO << "end" << isOk;
        return isOk;
    }

}
