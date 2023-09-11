#include "gfluoreditor.h"

#include "gglobal.h"

#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QVBoxLayout>

#ifdef DEVICE_TYPE_TL22
#define FONTSIZE    11
#else
#define FONTSIZE    12
#endif

GFluorEditor::GFluorEditor(int min, int max, const QByteArray &current, QWidget *parent) :
    QWidget(parent),
    m_min(min),
    m_max(max),
    m_x(0),
    m_y(0),
    m_width(0),
    m_height(0),
    m_margin(0),
    m_spacing(0),
    m_popup(false),
    m_listWidget(NULL)
{
    setWindowFlags(Qt::Popup);
//    setWindowModality(Qt::WindowModal);

//    setWindowOpacity(0);
    //初始化判断荧光最小最大序号，设置当前序号
    if(m_min > m_max){
        m_min = (int)0;
        m_max = (int)0;
    }
    if(m_min < (int)0 || m_min > (int)255)
        m_min = (int)0;
    if(m_max < (int)0 || m_max > (int)255)
        m_max = (int)0;

    QVBoxLayout *vLayout = new QVBoxLayout;

    QString currentText;
    for(int i=0; i<current.size(); i++){
        currentText += GHelper::keyOfFluor(1, current.at(i));
        if(i<current.size()-1) currentText += ";";
    }

    qDebug() << Q_FUNC_INFO << "step1" << current.size() << currentText;

    m_label = new QLabel(currentText);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setFrameShape(QFrame::Box);
    m_label->setFrameShadow(QFrame::Sunken);
    m_label->installEventFilter(this);
    vLayout->addWidget(m_label);

    m_listWidget = new QListWidget;
    m_spacing = 1;
    m_listWidget->setSpacing(m_spacing);
    vLayout->addWidget(m_listWidget);

    qDebug() << Q_FUNC_INFO << "step2" << m_min << m_max;

    m_current.clear();
    for(int i=m_min; i<=m_max; i++){
        QCheckBox *checkBox = new QCheckBox(GHelper::keyOfFluor(0, i), this);

        checkBox->setProperty("val",i);
        checkBox->setStyleSheet("QCheckBox::indicator:unchecked {image: url(:/png/noselect);} \nQCheckBox::indicator:checked {image: url(:/png/select);}");
        QFont font = checkBox->font();
        font.setBold(true);
        font.setPointSize(FONTSIZE);
        checkBox->setFont(font);

        qDebug() << Q_FUNC_INFO << "step3" << current.contains(i);

        if(current.contains(i)){
            checkBox->setChecked(true);
        }        

        connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));

        QListWidgetItem* widgetItem = new QListWidgetItem(m_listWidget);
        widgetItem->setSizeHint(QSize(widgetItem->sizeHint().width(), 35));
        m_listWidget->setItemWidget(widgetItem, checkBox);

        m_current.append(i);
    }

    m_margin = 1;
    vLayout->setMargin(m_margin);    
    vLayout->setSpacing(1);
    this->setLayout(vLayout);
}

GFluorEditor::~GFluorEditor()
{
    m_label = NULL;
    m_listWidget = NULL;
}

void GFluorEditor::setCurrentKey(const QByteArray &key)
{
    qDebug()<<Q_FUNC_INFO<<key.count();
    m_current.clear();
    for(int i=0; i<key.count(); i++){
        if(key[i] > 0 && key[i] < 255){
            m_current.append(key[i]);
        }
    }
}

void GFluorEditor::showList()
{
    m_popup = true;

    m_label->setMinimumSize(m_width-m_margin*2, m_height-m_margin*2);
    m_label->setMaximumSize(m_width-m_margin*2, m_height-m_margin*2);
#ifdef DEVICE_TYPE_TL22
    m_listWidget->setMinimumSize(m_width-m_margin*2, (m_height+1)*(m_max-m_min+1)+(m_max-m_min+2)*m_spacing);
    m_listWidget->setMaximumSize(m_width-m_margin*2, (m_height+1)*(m_max-m_min+1)+(m_max-m_min+2)*m_spacing);
#endif
    this->show();
    if(!m_listWidget->isVisible()) m_listWidget->show();
    this->move(m_x,m_y);
}

void GFluorEditor::hideList()
{
    m_popup = false;
    this->close();
}

bool GFluorEditor::eventFilter(QObject *obj, QEvent *e)
{
    if(obj == m_label){
        if(e->type() == QEvent::MouseButtonPress){
            if(m_popup){
                emit keyPressed();
                hideList();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj,e);
}

void GFluorEditor::setRange(int x, int y, int width/*, int height*/)
{
    m_x = x;
    m_y = y;
    m_width = width;
//    m_height = height;
    m_height = 36;
}

void GFluorEditor::stateChanged()
{
    emit keyPressed();

    m_current.clear();

    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    int index = checkBox->property("val").toInt();

    QString txt;
    int nCount = m_listWidget->count();

    qDebug()<<Q_FUNC_INFO<<nCount;
    for (int i = 0; i < nCount; ++i){
        QListWidgetItem *pItem = m_listWidget->item(i);
        QWidget *pWidget = m_listWidget->itemWidget(pItem);
        QCheckBox *pCheckBox = (QCheckBox *)pWidget;

        if(pCheckBox->property("val").toInt() == index){
            if(pCheckBox->isChecked()){
                m_current.append(index);
                txt = GHelper::keyOfFluor(0, index);
            }
        }else if(pCheckBox->isChecked()){
            pCheckBox->blockSignals(true);
            pCheckBox->setChecked(false);
            pCheckBox->blockSignals(false);
        }
    }
    qDebug()<<Q_FUNC_INFO<<nCount<<txt;

    m_label->setText(txt.isEmpty() ? GHelper::keyOfFluor(0, index) : txt);




//    close();
}
