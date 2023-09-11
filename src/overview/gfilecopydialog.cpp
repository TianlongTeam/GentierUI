#include "gfilecopydialog.h"
#include "ui_gfilecopydialog.h"

#include "gglobal.h"
#include "gdatapool.h"
#include "gfilecopydelegate.h"
#include "gexperimentfile.h"

#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

#define ROW_HEIGHT 50

GFileCopyDialog::GFileCopyDialog(GDataPool *dataPool, operatorType type, const QString &localPath, const QString &usbPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GFileCopyDialog),
    m_pool(dataPool),
    modelLocal(NULL),
    modelUsb(NULL),
    fileInDelegate(NULL),
    fileOutDelegate(NULL),
    opType(type)
{
    ui->setupUi(this);

//    setAttribute(Qt::WA_TranslucentBackground);

    QString title = (opType == ImportFile) ? tr("Import files") : tr("Export files");
    ui->labelTitle->setText(title);

    fileInDelegate = new GFileCopyDelegate;
    fileOutDelegate = new GFileCopyDelegate(true);
    connect(fileOutDelegate, SIGNAL(itemClicked(bool)), m_pool, SLOT(screen_sound()));

    //当前仪器状态
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    //显示所有本地文件
    modelLocal = new QStandardItemModel;
    if(modelLocal){
        modelLocal->setHorizontalHeaderLabels(QStringList() << tr("File Name"));

        ui->tableViewLocal->setModel(modelLocal);
        ui->tableViewLocal->horizontalHeader()->setHighlightSections(false);
        ui->tableViewLocal->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
        ui->tableViewLocal->setItemDelegate((opType==ImportFile)?fileInDelegate:fileOutDelegate);

        int row = 0;
#ifdef DEVICE_TYPE_TL13
        if(opType == ExportFile){
            //只有导出实验时才显示目录
            foreach(const QFileInfo &fileInfo, QDir(localPath).entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs)){
                QStandardItem *item = new QStandardItem;
                item->setData(fileInfo.fileName(), Qt::DisplayRole);
                item->setData(1,Qt::UserRole+1);
                item->setData(1,Qt::UserRole+2);
                modelLocal->setItem(row,0,item);
                row++;
            }
        }
#endif
        //显示实验文件
        foreach(const QFileInfo &fileInfo, QDir(localPath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files)){
            //去掉当前运行的实验名
            if(pcrInfo.State.ExpState != 0 && QFileInfo(m_pool->expFile->fileName()).completeBaseName() == fileInfo.completeBaseName()) continue;

            QStandardItem *item = new QStandardItem;
            item->setData(fileInfo.fileName(), Qt::DisplayRole);
            if(opType == ExportFile)
                item->setData(1,Qt::UserRole+1);
            modelLocal->setItem(row,0,item);
            row++;
        }
    }

    //显示USB设备中的文件
    modelUsb = new QStandardItemModel;
    if(modelUsb){
        modelUsb->setHorizontalHeaderLabels(QStringList() << tr("File Name"));

        ui->tableViewUsb->setModel(modelUsb);
        ui->tableViewUsb->horizontalHeader()->setHighlightSections(false);
        ui->tableViewUsb->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
        ui->tableViewUsb->setItemDelegate((opType==ImportFile)?fileOutDelegate:fileInDelegate);

        int row = 0;
        foreach(const QFileInfo &fileInfo, QDir(usbPath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files)){
            //去掉当前运行的实验名
            if(pcrInfo.State.ExpState != 0 && QFileInfo(m_pool->expFile->fileName()).completeBaseName() == fileInfo.completeBaseName()) continue;            

            QStandardItem *item = new QStandardItem;
            item->setData(fileInfo.fileName(), Qt::DisplayRole);
            if(opType == ImportFile)
                item->setData(1,Qt::UserRole+1);
            modelUsb->setItem(row,0,item);
            row++;
        }
    }

    connect(ui->tableViewUsb->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), m_pool, SLOT(screen_sound()));

#if (defined DEVICE_TYPE_TL13) || (defined DEVICE_TYPE_TL23)
    showMaximized();
#endif
}

GFileCopyDialog::~GFileCopyDialog()
{           
    m_pool = NULL;

    if(fileInDelegate){
        delete fileInDelegate;
        fileInDelegate = NULL;
    }

    if(fileOutDelegate){
        delete fileOutDelegate;
        fileOutDelegate = NULL;
    }

    if(modelLocal){
        delete modelLocal;
        modelLocal = NULL;
    }

    if(modelUsb){
        delete modelUsb;
        modelUsb = NULL;
    }

    delete ui;
}

void GFileCopyDialog::on_checkBoxSelectAll_toggled(bool checked)
{
    m_pool->screen_sound();

    QStandardItemModel *model = (opType == ImportFile) ? modelUsb : modelLocal;

    for(int i=0; i<model->rowCount(); i++)
        model->setData(model->index(i,0),checked?1:0,Qt::UserRole+1);
}

#ifdef DEVICE_TYPE_TL13
QStringList GFileCopyDialog::selectedDirs() const
{
    QStandardItemModel *model = (opType == ImportFile) ? modelUsb : modelLocal;

    QStringList dirs;
    for(int i=0; i<model->rowCount(); i++){
        bool isSelected = model->data(model->index(i,0),Qt::UserRole+1).toInt() != 0;
        bool isDir = model->data(model->index(i,0),Qt::UserRole+2).toInt() != 0;
        if(isDir){
            if(isSelected) dirs << model->data(model->index(i,0),Qt::DisplayRole).toString().trimmed();
        }else{
            break;
        }
    }
    return dirs;
}
#endif

QStringList GFileCopyDialog::selectedFiles() const
{
    QStandardItemModel *model = (opType == ImportFile) ? modelUsb : modelLocal;

    QStringList files;
    for(int i=0; i<model->rowCount(); i++){
        bool isSelected = model->data(model->index(i,0),Qt::UserRole+1).toInt() != 0;
        bool isFile = model->data(model->index(i,0),Qt::UserRole+2).toInt() == 0;
        if(isFile && isSelected){
            files << model->data(model->index(i,0),Qt::DisplayRole).toString().trimmed();
        }
    }
    return files;
}

void GFileCopyDialog::on_buttonOk_clicked()
{
    m_pool->screen_sound();
    accept();
}

void GFileCopyDialog::on_buttonCancel_clicked()
{
    m_pool->screen_sound();
    reject();
}

void GFileCopyDialog::paintEvent(QPaintEvent */*event*/)
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
