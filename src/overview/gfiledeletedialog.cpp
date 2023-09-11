#include "gfiledeletedialog.h"
#include "ui_gfiledeletedialog.h"

#include "gglobal.h"
#include "gdatapool.h"
#include "gexperimentfile.h"
#include "gfilecopydelegate.h"

#include <QStandardItemModel>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

#include <QPainter>
#include <QtMath>
#include <QRadialGradient>

GFileDeleteDialog::GFileDeleteDialog(GDataPool *dataPool, const QString &currentFile, const QString &loaclPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GFileDeleteDialog),
    m_pool(dataPool),
    modelFile(NULL),
    fileDelegate(NULL)
{
    ui->setupUi(this);

//    setAttribute(Qt::WA_TranslucentBackground);

    fileDelegate = new GFileCopyDelegate(true);

    modelFile = new QStandardItemModel;
    if(modelFile){
        QStringList headers;
        headers << tr("File Name") << tr("Date Created");
        modelFile->setHorizontalHeaderLabels(headers);

        ui->tableViewFile->setModel(modelFile);
        ui->tableViewFile->horizontalHeader()->setHighlightSections(false);
        ui->tableViewFile->setItemDelegateForColumn(0, fileDelegate);
        ui->tableViewFile->setColumnWidth(0,310);
        ui->tableViewFile->verticalHeader()->setDefaultSectionSize(50);

        //如果实验正在运行,退出
        _PCR_RUN_CTRL_INFO pcrInfo;

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        int row = 0;
        foreach(const QFileInfo &fileInfo, QDir(loaclPath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files)){
            //如果正在实验,不显示正在实验中的文件名
            if(pcrInfo.State.ExpState != 0 && QFileInfo(m_pool->expFile->fileName()).fileName() == fileInfo.fileName()) continue;

            QStandardItem *item = new QStandardItem;
            item->setData(fileInfo.fileName(), Qt::DisplayRole);

            //如果是当前选择的文件
            if(!currentFile.isEmpty() && currentFile == fileInfo.completeBaseName()){
                item->setData(1,Qt::UserRole+1);
            }

            modelFile->setItem(row,0,item);
            modelFile->setItem(row,1,new QStandardItem(fileInfo.created().toString(m_pool->dateFormat+" "+(m_pool->is24HourFormat?"hh:mm:ss":"hh:mm:ss AP"))));

            row++;
        }

        //判断是否有文件选择
        updateButton();
    }

    connect(fileDelegate, SIGNAL(itemClicked(bool)), this, SLOT(g_itemClicked()));
    connect(ui->tableViewFile->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), m_pool, SLOT(screen_sound()));

#if (defined DEVICE_TYPE_TL13) || (defined DEVICE_TYPE_TL23)
    showMaximized();
#endif
}

GFileDeleteDialog::~GFileDeleteDialog()
{
    m_pool = NULL;

    if(fileDelegate){
        delete fileDelegate;
        fileDelegate = NULL;
    }

    if(modelFile){
        delete modelFile;
        modelFile = NULL;
    }

    delete ui;
}

QStringList GFileDeleteDialog::selectedFiles() const
{
    QStringList files;
    for(int i=0; i<modelFile->rowCount(); i++){
        bool isSelected = modelFile->data(modelFile->index(i,0),Qt::UserRole+1).toInt() != 0;
        if(isSelected){
            files << modelFile->data(modelFile->index(i,0),Qt::DisplayRole).toString().trimmed();
        }
    }
    return files;
}

void GFileDeleteDialog::on_checkBoxSelectAll_toggled(bool checked)
{
    m_pool->screen_sound();

    for(int i=0; i<modelFile->rowCount(); i++){
        modelFile->setData(modelFile->index(i,0),checked?1:0,Qt::UserRole+1);
    }

    ui->buttonOk->setEnabled(checked && modelFile->rowCount()>0);
}

void GFileDeleteDialog::on_buttonCancel_clicked()
{
    m_pool->screen_sound();
    reject();
}

void GFileDeleteDialog::on_buttonOk_clicked()
{
    m_pool->screen_sound();
    accept();
}

void GFileDeleteDialog::g_itemClicked()
{
    m_pool->screen_sound();
    updateButton();
}


void GFileDeleteDialog::updateButton()
{
    bool isSelected = false;
    for(int i=0; i<modelFile->rowCount(); i++){
        isSelected = modelFile->data(modelFile->index(i,0),Qt::UserRole+1).toInt() != 0;
        if(isSelected) break;
    }
    ui->buttonOk->setEnabled(isSelected);
}

void GFileDeleteDialog::paintEvent(QPaintEvent */*event*/)
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

//    QColor color(0, 0, 0, 50);
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
