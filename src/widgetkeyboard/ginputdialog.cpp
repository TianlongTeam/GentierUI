#include "ginputdialog.h"

#include "mymessagebox.h"

#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include "widgetkeyboard.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

GInputDialog::GInputDialog(const QString &suffix, const QString &path, const QString &text, int inputType, QWidget *parent) :
    QDialog(parent),
    m_inputType(inputType),
    m_defaultSuffix(suffix)
{
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QDialog {\nborder: 2px solid #2f74d0;\nborder-radius: 5px;\npadding: 2px 4px;\n\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n} \nQWidget#hzpanel, QLabel {\nbackground-color: transparent;\n} \nQAbstractButton {\nborder: 1px solid #2f74d0;\nfont: 11pt \"文泉驿等宽微米黑\";\nborder-radius: 2px;\npadding: 5px;\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #c9e5fe);\n}\n\nQAbstractButton:hover {\nbackground: qradialgradient(cx: 0.3, cy: -0.4,\nfx: 0.3, fy: -0.4,\nradius: 1.35, stop: 0 #fff, stop: 1 #e4f2fe);\n}\n\n QAbstractButton:pressed {\nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #f1f8fe);\n}\n\nQAbstractButton:disabled { \nbackground: qradialgradient(cx: 0.4, cy: -0.1,\nfx: 0.4, fy: -0.1,\nradius: 1.35, stop: 0 #fff, stop: 1 #F1F0EF);\n}");

    label = new QLabel(tr("File Name:"), this);
    label->setMinimumHeight(35);
    lineEdit = new QLineEdit(this);
    lineEdit->setMinimumHeight(35);

    if(!text.isEmpty()){
        lineEdit->setText(text);
        lineEdit->selectAll();
    }

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(label);
    hLayout1->addWidget(lineEdit);
    hLayout1->addSpacerItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Preferred));

    keyboard = new WidgetKeyboard(this);
    keyboard->setPeriodVisible(inputType == IT_Name);
  //  keyboard->setPeriodVisible(inputType != IT_Folder);
//  keyboard->setPeriodVisible(inputType != IT_ExpName);

    connect(keyboard, SIGNAL(sig_key_press()), this, SLOT(g_keyPressed()));
    connect(keyboard, SIGNAL(sig_press_over(bool)), this, SLOT(pushButton_clicked(bool)));
    keyboard->clearCtrl();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout1);
    vLayout->addWidget(keyboard);
//    vLayout->addSpacerItem(new QSpacerItem(20,10,QSizePolicy::Preferred,QSizePolicy::Fixed));
//    vLayout->addLayout(hLayout2);
    this->setLayout(vLayout);

    //如果文件路径为空，设置为当前路径
    if(path.isEmpty()){
        m_path = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator());
    }else{
        m_path = path;
    }

    m_fileName = QString::null;
    m_textSelected = false;
    m_maxCharLength = 64;
}

GInputDialog::~GInputDialog()
{
}

QString GInputDialog::input() const
{
    return m_fileName;
}

void GInputDialog::setMode(InputType )
{

}

void GInputDialog::setTitle(const QString &title)
{
    label->setText(title);
}

void GInputDialog::setMaxCharLength(int maxchar)
{
    m_maxCharLength = maxchar;
}

void GInputDialog::paintEvent(QPaintEvent *)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(2, 2, this->width()-4, this->height()-4);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient gradient(0.3,-0.4,1.35,0.3,-0.4);
    QVector<QGradientStop> gradientStops;
    gradientStops.append(qMakePair(0,0x000fff));
    gradientStops.append(qMakePair(1,0xc9e5fe));
    gradient.setStops(gradientStops);
    painter.fillPath(path, QBrush(gradient));

    QColor color(47, 116, 208, 50);
    for(int i=0; i<1; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(2-i, 2-i, this->width()-(2-i)*2, this->height()-(2-i)*2);
//        color.setAlpha(150 - qSqrt(i)*50);
        color.setAlpha(255);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

void GInputDialog::pushButton_clicked(bool isOk)
{
    if(isOk){
        QString txt = lineEdit->text().trimmed();
        if(txt.isEmpty()) return;

        if(m_inputType == IT_ExpName){
            QFileInfo fileInfo(txt);
            m_fileName = txt + ((fileInfo.suffix() == m_defaultSuffix) ? QString::null : ("."+m_defaultSuffix));
            fileInfo.setFile(m_fileName);

            if(QFile::exists(m_fileName) || QFile::exists(m_path+fileInfo.completeBaseName()+".tlpd") || QFile::exists(m_path+fileInfo.completeBaseName()+".tlpp")){
                My_MessageBox mb;
                mb.ginformation(NULL, NULL, tr("New"), tr("File %1 already exists.").arg(fileInfo.completeBaseName()));
                m_textSelected = true;
                lineEdit->selectAll();
                return;
            }
        }else{
            m_fileName = txt;
        }

        accept();
    }else{
        reject();
    }
}

void GInputDialog::g_keyPressed()
{    
    QString txt = lineEdit->text().trimmed();

    //如果第一个字符是".",提示
//    if(txt.count()>0 && txt.at(0).toLatin1()=='.'){
//        QString warnStr;
//        switch(m_inputType){
//        case IT_Name:
//            warnStr = tr("The first character of device name cannot be \".\"");
//            break;
//        case IT_Folder:
//            warnStr = tr("The first character of folder name cannot be \".\"");
//            break;
//        default:
//            warnStr = tr("The first character of experiment file name cannot be \".\"");
//            break;
//        }
//        My_MessageBox mb;
//        mb.gwarning(m_pool, NULL, tr("Warning"), warnStr);

//        lineEdit->blockSignals(true);
//        lineEdit->clear();
//        lineEdit->blockSignals(false);
 //       return;
 //   }

qDebug() << Q_FUNC_INFO << "prev" << m_maxCharLength << txt << txt.size();
    QRegExp rx("[\u4e00-\u9fa5]");
    txt.replace(rx, "zz");

    qDebug() << Q_FUNC_INFO << "next" << m_maxCharLength << txt << txt.size();

    if(txt.size() > m_maxCharLength){
        txt = lineEdit->text().trimmed();
        lineEdit->blockSignals(true);
        lineEdit->setText(getString(txt,m_maxCharLength));
        lineEdit->blockSignals(false);

//        My_MessageBox mb;
      //  mb.gwarning(m_pool, NULL, tr("Warning"), tr("The number of bytes entered is full[0~%1]!").arg(m_maxCharLength));
        lineEdit->setFocus();
    }else{
        emit keyPressed();
    }
}

//把中英文混合的字符串截取一定长度，但不能出现中文被从中间截断的情况
QString GInputDialog:: getString(QString s, int l)
{
    QString temp = s;
    QRegExp rx("[\u4e00-\u9fa5]");
    if (temp.replace(rx, "zz").length() <= l) {
        return s;
    }

    for (int i=s.length(); i>=0; i--) {
        temp = s.mid(0,i);
        if (temp.replace(rx, "zz").length() <= l) {
            return s.mid(0,i);
        }
    }
    return QString("");
}
