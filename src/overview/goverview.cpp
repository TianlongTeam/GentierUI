/*!
* \file goverview.cpp
* \brief ARM板软件中实验管理界面cpp文件
*
*实现了ARM板软件实验文件的新建、重命名、删除、导入和导出功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:35
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "goverview.h"
#include "ui_goverview.h"
#include "gglobal.h"
#include "gfileitemdelegate.h"
#include "gproxymodel.h"
#include "gexperimentfile.h"
#include "gdatapool.h"

#include "ginputdialog.h"
#include "gusbselectdialog.h"
#include "DMSNavigation.h"
#include "gfilesystemmodel.h"
#include "gfilecopydialog.h"
#include "gfiledeletedialog.h"
#include <QRegularExpression>
#include <QTextCodec>
#include <QDateTime>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QProcess>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include "mymessagebox.h"
#include <QMenu>
#include <QProgressBar>

//-----------------------------------------------------------------------------
//define declare
//-----------------------------------------------------------------------------
#ifdef DEVICE_TYPE_TL22
#define SHOW_WIDTH_PIXEL   749  //792
#define SHOW_HEIGHT_PIXEL  330
#else
#define SHOW_WIDTH_PIXEL   673  //792
#define SHOW_HEIGHT_PIXEL  300
#endif

#define DETAIL_ROW_COUNT    8

#ifdef DEVICE_TYPE_TL22
#define DETAIL_COL0_WIDTH   300
#define DETAIL_COL3_WIDTH   220
#define DETAIL_COL2_WIDTH   125
#else
#define DETAIL_COL0_WIDTH   300
#define DETAIL_COL3_WIDTH   240
#define DETAIL_COL2_WIDTH   105
#endif

#define DETAIL_HEIGHT       (SHOW_HEIGHT_PIXEL/DETAIL_ROW_COUNT)

#define ICON_COLUMN_COUNT   4
#define ICON_ROW_COUNT      4
#define ICON_PAGE_TOTAL     ICON_COLUMN_COUNT * ICON_ROW_COUNT
#define ICON_WIDTH    SHOW_WIDTH_PIXEL/ICON_COLUMN_COUNT
#define ICON_HEIGHT   SHOW_HEIGHT_PIXEL/ICON_ROW_COUNT
#define ICON_FILEOFPAGE_COUNT   ICON_COLUMN_COUNT*ICON_ROW_COUNT

//-----------------------------------------------------------------------------
//private data class declare
//-----------------------------------------------------------------------------

/*!
* \class PrivateData
* \brief 类GOverView内部的私有数据类
*
* 用于统一管理私有数据
*/
class GOverView::PrivateData
{
public:
    explicit PrivateData() : \
        fileModel(NULL),
        proxyModel(NULL),
        iconDelegate(NULL),
        detailDelegate(NULL),
    #ifndef DEVICE_TYPE_TL13
        createMenu(NULL),
    #endif
        swapMenu(NULL),
    #ifdef DEVICE_TYPE_TL13
        dirMenu(NULL),
    #endif
        createEmptyExp(NULL),
        createFolder(NULL),
        createExpBaseOnCurrent(NULL),
        swapExport(NULL),
        swapImport(NULL),
    #ifdef DEVICE_TYPE_TL13
        getinDir(NULL),
        dirDeep(0),
    #endif
        showType(GOverView::st_Detail),
        globalPosY(0),
        currentPos(0),
        isFileClicked(false)
    {
        fileCounts.clear();
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/3);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/4);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/5);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/6);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/7);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/8);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/9);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/10);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/20);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/30);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/40);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/50);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/50);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/70);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/80);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/90);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT - EXPERIMENT_TOTAL_COUNT/100);
        fileCounts.append(EXPERIMENT_TOTAL_COUNT);
    }

    ~PrivateData(){
        if(fileModel) delete fileModel;
        if(proxyModel) delete proxyModel;
        if(iconDelegate) delete iconDelegate;
        if(detailDelegate) delete detailDelegate;
        if(createMenu) delete createMenu;
        if(swapMenu) delete swapMenu;
#ifdef DEVICE_TYPE_TL13
        if(getinDir) delete getinDir;
        if(dirMenu) delete dirMenu;
#endif
    }

    GFileSystemModel *fileModel;        ///< 文件系统模板（列表显示方式）
    GProxyModel *proxyModel;            ///< 文件系统代理模板（图标显示方式）
    GFileIconDelegate   *iconDelegate;  ///< 图标显示方式的视图代理
    GFileDetailDelegate *detailDelegate;///< 列表显示方式的视图代理
    QMenu *createMenu;                  ///< 新建按键上的菜单
    QMenu *swapMenu;                    ///< 与U盘交换文件的菜单
#ifdef DEVICE_TYPE_TL13
    QMenu *dirMenu;                         ///<
#endif
    QAction *createEmptyExp;            ///< 新建空实验
    QAction *createFolder;            ///< 新建空Folder
    QAction *createExpBaseOnCurrent;    ///< 基于当前实验设置新建实验
    QAction *swapExport;                ///< 导出文件至U盘
    QAction *swapImport;                ///< 从U盘导入文件
#ifdef DEVICE_TYPE_TL13
    QAction *getinDir;
    int dirDeep;
#endif
    int showType;                       ///< 文件显示方式
    int globalPosY;                     ///< 鼠标当前Y轴坐标,用于手动调整文件栏
    int currentPos;                     ///< ScrollBar当前值
    bool isFileClicked;                 ///< 选择不同的文件

    QModelIndex rootIndex;              ///< 文件系统根目录的index
    QString selectedFile;               ///< 当前选择或新建的文件名
    QString runFile;                    ///< 当前运行的实验名
#ifdef DEVICE_TYPE_TL13
    QString selectedDir;
#endif
    QList<int> fileCounts;              ///< 显示提示信息对应的文件个数
};

//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------

/*!
* \class GOverView
* \brief ARM板实验管理界面类
*
* 实验文件的显示、新建、重命名、删除和导入导出功能
*/

/*!
* \brief 类GOverView的构造函数
* \param parent = NULL
* \return 无
*/
GOverView::GOverView(GDataPool *dataPool, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GOverView),
    m_d(new PrivateData()),
    m_pool(dataPool)
{    
    ui->setupUi(this);

    //初始化
    initVariables();
    initUi();
}

/*!
* \brief 类GOverView的析构函数
* \param 无
* \return 无
*/
GOverView::~GOverView()
{
    delete ui;
    delete m_d;

    qDebug() << "delete GOverView widget";
}

/*!
* \brief 类GOverView的公共函数，设置当前选择的实验文件
* \param 无
* \return 无
*/
int GOverView::setSelectFile(const QString &fileName)
{
    //空文件名
    if(fileName.isEmpty()) return 1;

    //文件不存在
#ifdef Q_OS_LINUX
    QString fn = QDir::toNativeSeparators(SYS_DIR_FILE) + fileName;
#else
    QString fn = QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_FILE + QDir::separator() + fileName);
#endif

    if(!QFile::exists(fn)) return 2;

    QFileInfo fileInfo(fn);
    m_d->selectedFile = fileInfo.fileName();

#ifdef DEVICE_TYPE_TL13
    QString currentPath = QDir::toNativeSeparators(fileInfo.absolutePath() + QDir::separator());
    qDebug() << "1111111111111111111111111" << m_pool->filePath <<  fileName << currentPath << m_pool->filePath << m_d->selectedDir << m_d->dirDeep ;
    if(currentPath != m_pool->filePath){
        qDebug() << "222222222222222";
        m_d->selectedDir = fileInfo.absolutePath();

        m_d->dirDeep = 1;
        m_d->fileModel->setFilter(QDir::Files|QDir::Dirs|QDir::NoDot);
        m_d->createFolder->setVisible(false);

        m_pool->filePath = QDir::toNativeSeparators(m_d->selectedDir+QDir::separator());
        m_d->rootIndex = m_d->fileModel->setRootPath(m_pool->filePath);
        m_d->proxyModel->setRootIndex(m_d->rootIndex);
        ui->tableViewFile->setRootIndex(m_d->rootIndex);
    }
#endif

    //查找定位当前文件位置
    QModelIndex findIndex = m_d->fileModel->index(fn);
    if(findIndex.isValid()){
        int row = findIndex.row();
        int col = m_d->fileModel->columnCount(m_d->rootIndex);

        QItemSelection nowSelection(m_d->fileModel->index(row,0,m_d->rootIndex), m_d->fileModel->index(row,col-1,m_d->rootIndex));
        if(m_d->showType == st_Icon){
            nowSelection = m_d->proxyModel->mapSelectionFromSource(nowSelection);
        }

        ui->tableViewFile->selectionModel()->select(nowSelection, QItemSelectionModel::ClearAndSelect);

        displayInfo(nowSelection.indexes().first());
    }else
        return 3;

    m_pool->expFile->setFile(fn);
    return 0;
}

/*!
* \brief 类GOverView的公共函数，用于启动或停止实验时使按键无效有效
* \param run 是否运行实验
* \return 无
*/
void GOverView::experimentState(bool run)
{
    ui->buttonNew->setEnabled(!run);
    ui->buttonRename->setEnabled(!run);
    ui->buttonDelete->setEnabled(!run);
    ui->buttonInterflow->setEnabled(!run && m_pool->usbMaps.count()>0);

    ui->tableViewFile->viewport()->update();
}

/*!
* \brief 类GOverView的公共函数，清空文件显示窗口的选择
* \param 无
* \return 无
*/
void GOverView::clearSelection()
{
    ui->tableViewFile->clearSelection();
    ui->tableViewFile->viewport()->update();
    m_d->isFileClicked = false;
}

/*!
* \brief 公共槽函数，通用设置界面USB设备插拔时的操作
* \param on USB设备是否插入
* \param disk USB设备的盘符
* \return 无
*/
void GOverView::slot_usbDeviceChanged()
{
    bool hasUsb = m_pool->usbMaps.count() > 0;
    _PCR_RUN_CTRL_INFO pcrInfo;
    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)(m_pool->runCtrlInfo.data()), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState==0){
        ui->buttonInterflow->setEnabled(hasUsb);
    }
}

/*!
* \brief 公共槽函数，通用设置界面清除存储区和debug界面恢复出厂设置时清除当前选择的实验
* \param on USB设备是否插入
* \param disk USB设备的盘符
* \return 无
*/
void GOverView::slot_clearCurrentFile()
{
    //清空选择
    ui->tableViewFile->selectionModel()->clear();

    ui->buttonRename->setEnabled(false);

    m_d->selectedFile.clear();
#ifdef DEVICE_TYPE_TL13
    if(m_d->dirDeep==1)
    {
        m_d->selectedDir.clear();
        m_pool->filePath= SYS_DIR_FILE;
        m_d->fileModel->setFilter(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
        m_d->rootIndex = m_d->fileModel->setRootPath(m_pool->filePath);
        m_d->proxyModel->setRootIndex(m_d->rootIndex);

        ui->tableViewFile->setRootIndex(m_d->rootIndex);
        ui->tableViewFile->clearSelection();
        m_d->dirDeep=0;

        m_d->createFolder->setVisible(true);
    }
#endif
    emit expFileFocus(false);
    emit expFileChanged(QString(), FOT_Clear);
}

/*!
* \brief 类GOverView的继承事件，用于动态切换语言
* \param 无
* \return 无
*/
void GOverView::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);//在此处刷新语言的

        //显示文件个数
        int fileCount = rootFileCount();
        ui->labelFileCount->setText(tr("Experiment File: %1/%2").arg(fileCount).arg(fileCount<EXPERIMENT_TOTAL_COUNT?EXPERIMENT_TOTAL_COUNT:fileCount));

        //显示文件信息
        QModelIndex select;
        if(ui->tableViewFile->selectionModel()->selectedIndexes().count() > 0)
            select = ui->tableViewFile->selectionModel()->selectedIndexes().first();

        displayInfo(select);

        m_d->createEmptyExp->setText(tr("New Experiment"));
        m_d->createExpBaseOnCurrent->setText(tr("New Experiment From Selected Experiment"));
#ifdef DEVICE_TYPE_TL13
        m_d->createFolder->setText(tr("New Folder"));
        if(m_d->getinDir) m_d->getinDir->setText(tr("Open"));
#endif
        m_d->swapExport->setText(tr("Export Experiment..."));
        m_d->swapImport->setText(tr("Import Experiment..."));
    }
    QWidget::changeEvent(e);
}

/*!
* \brief 类GOverView的继承事件，用于文件栏滚动
* \param 无
* \return 无
*/
bool GOverView::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->tableViewFile->viewport()){
        switch(event->type()){
        case QEvent::MouseButtonPress:{
            m_pool->screen_sound();
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if(e){
                QModelIndex index = ui->tableViewFile->indexAt(e->pos());
                if(index.isValid()){
                    m_d->isFileClicked = true;
                }
            }
            break;
        }
        case QEvent::MouseButtonRelease:{
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if(e){
                QModelIndex index = ui->tableViewFile->indexAt(e->pos());
                if(index.isValid()){
                    if(m_d->isFileClicked){
                        ui->tableViewFile->clearSelection();
                        ui->tableViewFile->viewport()->update();
                        m_d->isFileClicked = false;
                    }
#ifdef DEVICE_TYPE_TL13
                    else{

                        if(m_d->showType == st_Icon){
                            index = m_d->proxyModel->mapToSource(index);
                        }
                        bool isDir = m_d->fileModel->isDir(index);
                        m_d->createExpBaseOnCurrent->setVisible(!isDir);

                        if(isDir){
                            m_d->selectedDir = m_d->fileModel->filePath(index);

                            QString fn = QFileInfo(m_d->selectedDir).fileName();

                            if(m_d->dirDeep==0 && fn!=".."){
                                m_d->dirMenu->exec(QCursor::pos());
                            }else if(m_d->dirDeep > 0){
                                g_openDir_triggered();
                            }
                        }
                    }
#endif
                    return true;
                }else{
#ifdef DEVICE_TYPE_TL13
                    //如果在子目录中,按下移动到无效的区域再释放,则判断是否有选择的项,是不是返回
                    if(m_d->dirDeep>0 && ui->tableViewFile->selectionModel()->selectedIndexes().count()>0){
                        QModelIndex subIndex = ui->tableViewFile->selectionModel()->selectedIndexes().first();
                        if(m_d->showType == st_Icon){
                            subIndex = m_d->proxyModel->mapToSource(subIndex);
                        }
                        QFileInfo subInfo = m_d->fileModel->fileInfo(subIndex);
                        if(subInfo.isDir() && subInfo.fileName()==QString("..")){
                            m_d->selectedDir = m_d->fileModel->filePath(subIndex);
                            g_openDir_triggered();
                        }
                    }
#endif
                }
            }
            break;
        }
        default:break;
        }
    }else if(obj == ui->tableViewFile->verticalScrollBar()){
        if(event->type() == QEvent::MouseButtonRelease){
            m_pool->screen_sound();
        }
    }
    return QWidget::eventFilter(obj,event);
}

/*!
* \brief 类GOverView的私有槽函数，没有实验文件时直接新建空实验
* \param 无
* \return 无
*/
void GOverView::on_buttonNew_clicked()
{
    if(ui->buttonNew->menu() == NULL){
        slot_createEmptyExp();
    }
}
/*!
* \brief 类GOverView的私有槽函数，实现文件的重命名
* \param 无
* \return 无
*/
void GOverView::on_buttonRename_clicked()
{
    m_pool->screen_sound();
    //slot_openFolder(QString folderpath );
    //得到当前选择的文件或当前实验文件
    QFileInfo fi;
    if(ui->tableViewFile->selectionModel()->selectedIndexes().count() == 0){
        if(m_pool->expFile->fileName().trimmed().isEmpty()){
            ui->labelFileCount->setFocus();
            return;
        }
        qDebug()<<"到当前选择的文件或当前实验文件!!!!!!!";
        fi = QFileInfo(m_pool->expFile->fileName());
    }else{
        QModelIndex select = ui->tableViewFile->selectionModel()->selectedIndexes().first();
        if(!select.isValid()){
            ui->labelFileCount->setFocus();
            return;
        }

        if(m_d->showType == st_Icon){
            int row = select.row() * ICON_COLUMN_COUNT + select.column();
            select = m_d->fileModel->index(row, 0, m_d->rootIndex);
        }

        fi = m_d->fileModel->fileInfo(select);
    }

    bool isDir = fi.isDir();

    QString path = QDir::toNativeSeparators(fi.absolutePath() + QDir::separator());
    GInputDialog dialog(fi.suffix(), path, fi.completeBaseName(), (isDir?GInputDialog::IT_Folder:GInputDialog::IT_ExpName), this);
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    dialog.setTitle(isDir?tr("Rename Folder:"):tr("Rename Experiment:"));
    ui->buttonRename->setEnabled(false);
    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    QPoint point = this->rect().topLeft();
    point.setY(this->rect().height() - dialog.height());
    dialog.move(xx,this->mapToGlobal(point).y());

    m_pool->set_ui_busy_state(1);
    int ret_ = dialog.exec();
    if(ret_ == QDialog::Accepted){
        QString newName = dialog.input();

        newName = QDir::toNativeSeparators(fi.absolutePath() + QDir::separator() +newName);

        bool isOk = QFile::rename(fi.filePath(), newName);
        if(isOk){
            m_d->selectedFile = dialog.input();
            ui->buttonRename->setProperty("ISRENAME", true);
            QString fn = path + dialog.input();
            emit expFileChanged(fn);

            emit operatorLog(tr("Rename Experiment"));
        }else{
            My_MessageBox mb;
            mb.gwarning(m_pool, NULL, tr("Rename"), tr("Failed to rename."), tr("Ok"));
        }
    }
    ui->buttonRename->setEnabled(true);
}

/*!
* \brief 类GOverView的私有槽函数，实现文件的删除（实际上是移动到trash目录下）
* \param 无
* \return 无
*/
void GOverView::on_buttonDelete_clicked()
{
    m_pool->screen_sound();

    //得到当前选择的文件或当前实验文件
    QFileInfo fi;
    if(ui->tableViewFile->selectionModel()->selectedIndexes().count() == 0){
        if(!m_pool->expFile->fileName().trimmed().isEmpty() && QFileInfo(m_pool->expFile->fileName()).path()==QFileInfo(m_pool->filePath).path()){
            fi = QFileInfo(m_pool->expFile->fileName());
        }
    }else{
        QModelIndex select = ui->tableViewFile->selectionModel()->selectedIndexes().first();
        if(select.isValid()){
            if(m_d->showType == st_Icon){
                int row = select.row() * ICON_COLUMN_COUNT + select.column();
                select = m_d->fileModel->index(row, 0, m_d->rootIndex);
            }

            fi = m_d->fileModel->fileInfo(select);
        }
    }
#ifdef DEVICE_TYPE_TL13

    if(fi.isDir()){
        My_MessageBox mb;
        if(mb.gquestion(m_pool, NULL, tr("Removed"), tr("Folder %1 will be deleted, are you sure?").arg(fi.fileName()), tr("Ok"), tr("Cancel")) == 1)
            return;

        slot_DelFolder(fi.filePath());
        slot_clearCurrentFile();
        emit operatorLog(tr("Delete Folder"));
        return;
    }else
#endif       
        if(fi.isFile()){
            My_MessageBox mb;
            if(mb.gquestion(m_pool, NULL, tr("Removed"), tr("File %1 will be deleted, are you sure?").arg(fi.completeBaseName()), tr("Ok"), tr("Cancel")) == 1){
                return;
            }
            if(QFile::exists(fi.filePath())){
                bool isOk = QFile::remove(fi.filePath());
                if(isOk){
                    slot_clearCurrentFile();
                    emit operatorLog(tr("Delete Experiment"));
                }
            }
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
            ui->tableViewFile->setFocus();
            return;
        }

    QString fileName;
    GFileDeleteDialog dialog(m_pool,fileName,m_pool->filePath,this);

    m_pool->set_ui_busy_state(1);
    int ret_ = dialog.exec();
    if(ret_ == QDialog::Rejected){
        ui->tableViewFile->setFocus();
        return;
    }
    QStringList files = dialog.selectedFiles();
    My_MessageBox mb;
    if(mb.gquestion(m_pool, NULL, tr("Removed"), tr("The %1 will be deleted, are you sure?").arg(files.count()<=1?tr("file"):tr("files")), tr("Ok"), tr("Cancel")) == 1){
        ui->tableViewFile->setFocus();
        return;
    }

    QProgressBar progressBar(NULL);
    progressBar.setStyleSheet("QProgressBar{  color : solid #383e83;  border: 2px solid gray;  border-radius: 5px;  background: transparent;  padding: 0px;  text-align : center ;}  QProgressBar::chunk{  background: #c9e5fe;  }");
    progressBar.setRange(0, files.count());
    progressBar.setValue(0);
    progressBar.show();
    int xx = (m_pool->desktopWidth - progressBar.width())/2;
    int yy = (m_pool->desktopHeight - progressBar.height())/2;
    progressBar.move(xx,yy);

    for(int i=0; i<files.count(); i++){
        QString fn = m_pool->filePath + files.at(i);
        QFileInfo fi(fn);
        if(QFile::exists(fi.filePath())){
            QFile::remove(fi.filePath());
        }
        progressBar.setValue(i+1);
        QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
    }
    progressBar.hide();
    slot_clearCurrentFile();
    emit operatorLog(tr("Delete Experiment"));

    //转移焦点
    ui->tableViewFile->setFocus();
}

/*!
* \brief 类GOverView的私有槽函数，实现本地文件与USB设备之间的互通
* \param 无
* \return 无
*/
void GOverView::on_buttonInterflow_clicked()
{
    emit operatorLog(tr("Export/Import Experiment"));
}

/*!
* \brief 类GOverView的私有槽函数，实现文件窗口显示类型的转换：详细信息模式与图片模式
* \param 无
* \return 无
*/
void GOverView::g_btnFileShow_clicked()
{
    //    if(ui->tableViewFile->selectionModel()->selectedIndexes().count() > 0)
    //        ui->tableViewFile->clearSelection();
    //    else
    m_pool->screen_sound();

    //保存转换的显示类型
    ShowType type = (m_d->showType == st_Icon) ? st_Detail : st_Icon;

    //转换的实现
    fileViewChanged(type, true);
}

#ifdef DEVICE_TYPE_TL13
/*!
* \brief 类GOverView的私有槽函数
* \param 无
* \return 无
*/
bool GOverView::slot_DelFolder(QString folderpath)
{    
    if(folderpath.isEmpty())
        return false;

    QDir dir(folderpath);
    return dir.removeRecursively();
}

/*!
* \brief 类GOverView的私有槽函数
* \param 无
* \return 无
*/
void GOverView::g_openDir_triggered()
{
    QString fn = QFileInfo(m_d->selectedDir).fileName();
    bool useDir = false;
    if(m_d->dirDeep == 0){
        if(fn!=".." && fn!="."){
            m_d->dirDeep++;
            useDir = true;
            m_d->fileModel->setFilter(QDir::Files|QDir::Dirs|QDir::NoDot);
            m_d->createFolder->setVisible(false);
        }
    }else if(m_d->dirDeep > 0){
        if(fn==".."){
            m_d->dirDeep = 0;
            useDir = true;
            m_d->fileModel->setFilter(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
            m_d->createFolder->setVisible(true);
        }
    }

    if(useDir){
        m_pool->filePath = (m_d->dirDeep>0)?QDir::toNativeSeparators(m_d->selectedDir+QDir::separator()):QDir::toNativeSeparators(SYS_DIR_FILE);
        m_d->rootIndex = m_d->fileModel->setRootPath(m_pool->filePath);
        m_d->proxyModel->setRootIndex(m_d->rootIndex);
        ui->tableViewFile->setRootIndex(m_d->rootIndex);
        ui->tableViewFile->clearSelection();

        qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << m_d->dirDeep <<m_pool->filePath << m_d->rootIndex << m_d->fileModel->rowCount(m_d->rootIndex) <<m_d->proxyModel->rowCount(m_d->rootIndex);
    }
}
#endif

/*!
* \brief 类GOverView的私有槽函数，新建空实验，转到运行设置界面
* \param 无
* \return 无
*/
void GOverView::slot_createEmptyExp()
{
    qDebug() << Q_FUNC_INFO << ui->tableViewFile->viewport()->size();
    m_pool->screen_sound();

    m_pool->set_ui_busy_state(1);

    QString defaultFn = QDateTime::currentDateTime().toString("exp-yyyyMMddhhmmss");
    GInputDialog dialog("tlpp", m_pool->filePath, defaultFn, GInputDialog::IT_ExpName, this);
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    dialog.setTitle(tr("Experiment Name:"));
    ui->buttonNew->setEnabled(false);
    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    //    int yy = (m_pool->desktopHeight - dialog.height())/2;
    //    dialog.move(xx,yy);

    QPoint point = this->rect().topLeft();
    point.setY(this->rect().height() - dialog.height());
    dialog.move(xx,this->mapToGlobal(point).y());

    if(dialog.exec() == QDialog::Accepted){
        QString fn = m_pool->filePath + dialog.input();

        GExperimentFile file(fn);

        //保存
        int ret = file.save();
        if(ret != 0)
            qDebug() << "New experiment error :" << ret;
        m_d->selectedFile = dialog.input();

        emit expFileChanged(fn, FOT_New);
        emit expFileFocus(true);

        this->update();
        //自动转到运行设置界面
        emit next();
        emit operatorLog(tr("New Experiment"));

        if(!ui->buttonRename->isEnabled())
            ui->buttonRename->setEnabled(true);

        m_d->createExpBaseOnCurrent->setVisible(true);
    }
    ui->buttonNew->setEnabled(true);

    promptFileCount();

    ui->labelFileCount->setFocus();
}

/*!
* \brief 类GOverView的私有槽函数，新建空 Folder
* \param 无
* \return 无
*/
void GOverView::slot_createFolder()
{
    qDebug() << Q_FUNC_INFO << ui->tableViewFile->viewport()->size();
    m_pool->screen_sound();
    QString defaultFn = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    GInputDialog dialog("",m_pool->filePath, defaultFn, GInputDialog::IT_Folder, this);
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    dialog.setTitle(tr("Folder Name:"));
    ui->buttonNew->setEnabled(false);
    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;

    QPoint point = this->rect().topLeft();
    point.setY(this->rect().height() - dialog.height());
    dialog.move(xx,this->mapToGlobal(point).y());

    m_pool->set_ui_busy_state(1);
    int ret_ = dialog.exec();
    if(ret_ == QDialog::Accepted){
        QDir dir(m_pool->filePath);
        QString fn = m_pool->filePath + dialog.input();
        dir.mkdir(fn);
        m_d->selectedFile = dialog.input();
        emit expFileChanged(fn, FOT_Clear);
        emit expFileFocus(false);

        this->update();

        if(!ui->buttonRename->isEnabled())
            ui->buttonRename->setEnabled(true);
    }
    ui->buttonNew->setEnabled(true);

    ui->labelFileCount->setFocus();
}

/*!
* \brief 类GOverView的私有槽函数，基于当前实验设置新建实验，转到运行设置界面
* \param 无
* \return 无
*/
void GOverView::slot_createExpBaseOnCurrent()
{
    qDebug()<<"到当前选择的文件或当前实验文件!!!!!!!";
    //   m_pool->screen_sound();

    //得到当前选择的文件或当前实验文件
    QFileInfo fi;
    if(ui->tableViewFile->selectionModel()->selectedIndexes().count() == 0){
        if(m_pool->expFile->fileName().trimmed().isEmpty()){
            ui->labelFileCount->setFocus();
            return;
        }
        qDebug()<<"到当前选择的文件或当前实验文件!!!!!!!";
        fi = QFileInfo(m_pool->expFile->fileName());
    }else{
        QModelIndex select = ui->tableViewFile->selectionModel()->selectedIndexes().first();
        if(!select.isValid()){
            ui->labelFileCount->setFocus();
            return;
        }

        if(m_d->showType == st_Icon){
            int row = select.row() * ICON_COLUMN_COUNT + select.column();
            select = m_d->fileModel->index(row, 0, m_d->rootIndex);
        }

        fi = m_d->fileModel->fileInfo(select);
    }

    m_pool->set_ui_busy_state(1);

    int row = 0;
    int repeat = 1;
    QString defaultFile;
    QString completeBaseName = fi.completeBaseName();
    //    int pos = completeBaseName.indexOf('[');
    int pos = completeBaseName.indexOf('(');
    if(pos > 0) completeBaseName = completeBaseName.left(pos);
    while(row < m_d->fileModel->rowCount(m_d->rootIndex)){
        defaultFile = completeBaseName + QString("(%1)").arg(repeat);
        if(m_d->fileModel->fileInfo(m_d->fileModel->index(row,0,m_d->rootIndex)).completeBaseName() == defaultFile){
            row = 0;
            repeat ++;
        }else
            row ++;
    }

    GInputDialog dialog("tlpp", m_pool->filePath, defaultFile, GInputDialog::IT_ExpName, this);
    connect(&dialog, SIGNAL(keyPressed()), m_pool, SLOT(screen_sound()));
    dialog.setTitle(tr("Experiment Name:"));
    ui->buttonNew->setEnabled(false);
    dialog.show();
    int xx = (m_pool->desktopWidth - dialog.width())/2;
    QPoint point = this->rect().topLeft();
    point.setY(this->rect().height() - dialog.height());
    dialog.move(xx,this->mapToGlobal(point).y());

    if(dialog.exec() == QDialog::Accepted){
        QString fn = m_pool->filePath + dialog.input();
        GExperimentFile file(fn);
        GExperimentFile ff(fi.filePath());
        ff.open(m_pool->maxSpeed);
        file.setConfig(ff);
        int ret = file.save();
        qDebug() << Q_FUNC_INFO << fn << fi.filePath() << ret;
        if(ret != 0){
            My_MessageBox mb;
            mb.gwarning(m_pool, NULL, tr("Warning"), tr("New Experiment From Selected Experiment error: %1").arg(ret));
            ui->buttonNew->setFocus();
            ui->labelFileCount->setFocus();
            qDebug() << "New Experiment From Selected Experiment error :" << ret;

            ui->labelFileCount->setFocus();


            return;
        }

        m_d->selectedFile = dialog.input();
        emit expFileChanged(fn, FOT_New);
        emit expFileFocus(true);
        this->update();

        //自动转到运行设置界面
        emit next();
        emit operatorLog(tr("New Experiment From Selected Experiment"));
    }
    ui->buttonNew->setEnabled(true);

    qDebug() << Q_FUNC_INFO << m_pool->expFile->is_compressed_file << m_pool->expFile->fileName();

    if(!ui->buttonRename->isEnabled())
        ui->buttonRename->setEnabled(true);

    promptFileCount();


    ui->labelFileCount->setFocus();
}

/*!
* \brief 类GOverView的私有槽函数，加载根目录文件变化后更新软件显示
* \param 无
* \return 无
*/
void GOverView::slot_directoryLoaded()
{
    qDebug() << Q_FUNC_INFO << QTime::currentTime().toString("hh:mm:ss:zzz");

    if(m_d->fileModel==NULL || m_d->proxyModel==NULL) return;

    //更新model
    while(m_d->fileModel->canFetchMore(m_d->rootIndex)){
        m_d->fileModel->fetchMore(m_d->rootIndex);
    }

    //如果是启动时，转为图标显示方式
    if(this->property("FirstLoadDirectory").toBool()){
        fileViewChanged(st_Icon);
    }else{
        if(m_d->showType == st_Detail){
            //延时1秒,等待文件传输完毕,再得到文件大小,刷新显示
            QTimer::singleShot(1000,this,SLOT(g_reflesh_detailShow()));
        }else{
            ui->tableViewFile->setModel(m_d->fileModel);
            ui->tableViewFile->setRootIndex(m_d->rootIndex);
            fileViewChanged(st_Icon);
        }
    }

    //重新排序刷新显示
    Qt::SortOrder order = ui->tableViewFile->horizontalHeader()->sortIndicatorOrder();
    int sortColumn = ui->tableViewFile->horizontalHeader()->sortIndicatorSection();
    m_d->fileModel->sort(sortColumn, order);
    ui->tableViewFile->viewport()->update();
    qDebug() << "GOverView selected file is" << m_d->selectedFile;
    qDebug() << "data pool experiment file is" << m_pool->expFile->fileName();

    //显示文件个数
    int fileCount = rootFileCount();
    ui->labelFileCount->setText(tr("Experiment File: %1/%2").arg(fileCount).arg(fileCount<EXPERIMENT_TOTAL_COUNT?EXPERIMENT_TOTAL_COUNT:fileCount));

    qDebug() << "directory state 7";
    //根据文件个数给“新建”按键添加菜单
#ifndef DEVICE_TYPE_TL13
    bool hasSelect = ui->tableViewFile->selectionModel()->selectedIndexes().count() > 0;
    ui->buttonNew->setShowMode(GPushButton::UpLeft);
    ui->buttonNew->setMenu(hasSelect?m_d->createMenu:NULL);
    ui->buttonRename->setEnabled(hasSelect);
#endif
#ifdef DEVICE_TYPE_TL13
    bool isSelectedDir = false;
#endif
    //根据文件个数及是否运行判读删除键和导出菜单是否无效
    _PCR_RUN_CTRL_INFO pcrInfo;

    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState == 0){
        //判断当前目录下是否有文件或文件夹
#ifdef DEVICE_TYPE_TL13
        ui->buttonDelete->setEnabled((currentFileCount()>0) || isSelectedDir);
#else
        ui->buttonDelete->setEnabled(fileCount>0);
#endif
        //判断数据根目录下是否有文件或文件夹
        m_d->swapExport->setEnabled(fileCount>0);
    }

    if(this->property("FirstLoadDirectory").toBool()){
        this->setProperty("FirstLoadDirectory",false);
        ui->tableViewFile->setFocus();
        return;
    }
    //定位选择的文件
    QString fn = m_pool->expFile->fileName();
    if(!fn.isEmpty()){
        QFileInfo fileInfo(fn);
        QString fns = fileInfo.fileName();
        //查找定位当前文件位置
        QModelIndexList indexes = m_d->fileModel->match(m_d->fileModel->index(0,0,m_d->rootIndex), Qt::DisplayRole, fns);
#ifdef DEVICE_TYPE_TL13
        bool findFile = false;
        if((indexes.count()>0) && m_d->fileModel->filePath(indexes.first())==m_pool->expFile->fileName()){
            findFile = true;
        }
#else
        bool findFile = indexes.count() > 0;
#endif
        //如果当前文件不存在,查看是否文件从tlpp改为tlpd
        if(!findFile){
            if(fileInfo.suffix().toLower() == QString(".tlpp")){
                fns = fileInfo.completeBaseName() + tr(".tlpd");
                indexes = m_d->fileModel->match(m_d->fileModel->index(0,0,m_d->rootIndex), Qt::DisplayRole, fns);
                findFile = indexes.count() > 0;
                if(findFile){
                    //文件不存在
                    QString fnn = m_pool->filePath + fns;

                    m_d->selectedFile = fns;
                    m_pool->expFile->setFile(fnn);                    
                }
            }
        }

        if(findFile){
            int row = indexes.first().row();
            int col = m_d->fileModel->columnCount(m_d->rootIndex);

            QItemSelection nowSelection(m_d->fileModel->index(row,0,m_d->rootIndex), m_d->fileModel->index(row,col-1,m_d->rootIndex));
            if(m_d->showType == st_Icon){
                nowSelection = m_d->proxyModel->mapSelectionFromSource(nowSelection);
            }

            ui->tableViewFile->selectionModel()->blockSignals(true);
            ui->tableViewFile->selectionModel()->clear();
            ui->tableViewFile->selectionModel()->select(nowSelection, QItemSelectionModel::ClearAndSelect);
            ui->tableViewFile->selectionModel()->blockSignals(false);

            displayInfo(nowSelection.indexes().first());

            //如果实验正在运行,屏蔽删除键
            _PCR_RUN_CTRL_INFO pcrInfo;
            m_pool->mutex.lock();
            memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
            m_pool->mutex.unlock();
            if(pcrInfo.State.ExpState != 0){
                ui->buttonDelete->setEnabled(false);
                ui->buttonRename->setEnabled(false);
            }
        }else{
            ui->tableViewFile->selectionModel()->blockSignals(true);
            ui->tableViewFile->selectionModel()->clear();
            ui->tableViewFile->selectionModel()->blockSignals(false);

            displayInfo();
        }
    }
#ifdef DEVICE_TYPE_TL13
    else{
        //如果是子目录
        if(!m_d->selectedFile.isEmpty()){
            //查找定位当前文件夹
            QModelIndexList indexes = m_d->fileModel->match(m_d->fileModel->index(0,0,m_d->rootIndex), Qt::DisplayRole, m_d->selectedFile);
            QModelIndex dirIndex;
            foreach(const QModelIndex &index, indexes){
                if(m_d->fileModel->fileInfo(index).isDir()){
                    dirIndex = index;
                    break;
                }
            }

            if(dirIndex.isValid()){
                int row = dirIndex.row();
                int col = m_d->fileModel->columnCount(m_d->rootIndex);

                QItemSelection nowSelection(m_d->fileModel->index(row,0,m_d->rootIndex), m_d->fileModel->index(row,col-1,m_d->rootIndex));
                if(m_d->showType == st_Icon){
                    nowSelection = m_d->proxyModel->mapSelectionFromSource(nowSelection);
                }

                ui->tableViewFile->selectionModel()->blockSignals(true);
                ui->tableViewFile->selectionModel()->clear();
                ui->tableViewFile->selectionModel()->select(nowSelection, QItemSelectionModel::ClearAndSelect);
                ui->tableViewFile->selectionModel()->blockSignals(false);

                isSelectedDir = true;
            }
        }
    }
#endif

    ui->tableViewFile->setFocus();
    qDebug() << "file directory loaded end";
}

/*!
* \brief 类GOverView的私有槽函数，实验文件选择，并显示文件具体信息
* \param 无
* \return 无
*/
void GOverView::slot_fileChanged()
{    
    m_pool->screen_sound();

    //设置选择不同的文件标志,用于点击相同的文件,鼠标释放时取消
    m_d->isFileClicked = false;

    QModelIndex select;
    if(ui->tableViewFile->selectionModel()->selectedIndexes().count() > 0)
        select = ui->tableViewFile->selectionModel()->selectedIndexes().first();

    //显示文件具体信息
    QString fn = displayInfo(select);

    //如果实验正在运行,退出
    _PCR_RUN_CTRL_INFO pcrInfo;

    m_pool->mutex.lock();
    memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
    m_pool->mutex.unlock();

    if(pcrInfo.State.ExpState != 0){
        bool isOk = false;
#ifdef DEVICE_TYPE_TL13
        if(!fn.isEmpty() && (QFileInfo(fn).fileName()!=QString("..")) && (fn!=m_pool->expFile->fileName()) && (fn!=QFileInfo(m_pool->expFile->fileName()).path())){
            isOk = true;
        }
#else
        if(!fn.isEmpty() && (fn!=m_pool->expFile->fileName())){
            isOk = true;
        }
#endif
        ui->buttonDelete->setEnabled(isOk);
        ui->buttonRename->setEnabled(isOk);
        return;
    }

    emit expFileChanged(fn, FOT_Open);

    //其他控件

#ifdef DEVICE_TYPE_TL13
    emit expFileFocus(!QFileInfo(fn).isDir());
    if(currentFileCount() == 0){
        if(fn.isEmpty() && ui->buttonDelete->isEnabled()){
            ui->buttonDelete->setEnabled(false);
        }else if(QFileInfo(fn).isDir() && !ui->buttonDelete->isEnabled()){
            ui->buttonDelete->setEnabled(true);
        }
    }
#else
    ui->buttonDelete->setEnabled(true);
    emit expFileFocus(true);
#endif
}

/*!
* \brief 类GOverView的私有槽函数，实验文件排序
* \param 无
* \return 无
*/
void GOverView::slot_sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    qDebug() << "sort indicator changed :" << logicalIndex << order;
    if(m_d->fileModel == NULL) return;

    m_d->fileModel->sort(logicalIndex, order);

    qDebug() << "selected file:" << m_d->fileModel->fileName(m_d->fileModel->index(1,0,m_d->rootIndex));
}

/*!
* \brief 私有槽函数，等待文件传输完毕,再得到文件大小,刷新显示
* \param 无
* \return 无
*/
void GOverView::g_reflesh_detailShow()
{
    if(m_d->showType == st_Detail){
        ui->tableViewFile->viewport()->update();
    }
}

/*!
* \brief 私有槽函数，点击menu时导出文件到U盘
* \param 无
* \return 无
*/
void GOverView::slot_exportExp()
{
    m_pool->screen_sound();

    //设置主控为编辑状态
    m_pool->set_ui_busy_state(1);

    //得到U盘路径
    QString devStr;
    if(m_pool->usbMaps.count() < 1){
        //无U盘
    }else if(m_pool->usbMaps.count() == 1){
        devStr = m_pool->usbMaps.firstKey();
    }else{
        GUsbSelectDialog dialog(m_pool->usbMaps, this);
        dialog.show();
        int xx = (m_pool->desktopWidth - dialog.width())/2;
        int yy = (m_pool->desktopHeight - dialog.height())/2;
        dialog.move(xx,yy);

        if(dialog.exec() == QDialog::Accepted){
            devStr = dialog.currentDev();
        }
    }

    if(devStr.isEmpty()){
        ui->labelFileCount->setFocus();
        return;
    }
    qDebug() << "select Usb device:" << devStr;

    //显示导出选择对话框
    GFileCopyDialog dialog(m_pool, GFileCopyDialog::ExportFile, m_pool->filePath, devStr, this);

    if(dialog.exec() == QDialog::Rejected){
        return;
    }

    QStringList files = dialog.selectedFiles();

    //计算文件总数
    int fileCount = files.count();

#ifdef DEVICE_TYPE_TL13
    QStringList dirs = dialog.selectedDirs();

    //添加文件夹中的文件个数
    for(int i=0; i<dirs.count(); i++){
        QString dirPath = QDir::toNativeSeparators(SYS_DIR_FILE+dirs.at(i));
        fileCount += QDir(dirPath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
    }
#endif
    //无效按键
    emit fileOperator(true);

    //显示
    QProgressBar progressBar(NULL);
    progressBar.setStyleSheet("QProgressBar{  color : solid #383e83;  border: 2px solid gray;  border-radius: 5px;  background: transparent;  padding: 0px;  text-align : center ;}  QProgressBar::chunk{  background: #c9e5fe;  }");
    progressBar.setRange(0, fileCount);
    progressBar.setValue(0);
    progressBar.show();
    int xx = (m_pool->desktopWidth - progressBar.width())/2;
    int yy = (m_pool->desktopHeight - progressBar.height())/2;
    progressBar.move(xx,yy);

    int file_cnt = 0;   ///< 拷贝的文件个数计数

#ifdef DEVICE_TYPE_TL13
    //先拷贝目录中的文件
    for(int i=0; i<dirs.count(); i++){
        QString dirstxt = dirs.at(i);
        QString dirPath = QDir::toNativeSeparators(QStringLiteral(SYS_DIR_FILE)+dirstxt);
        //首先在U盘中新建目录
        QString usbPath = QDir::toNativeSeparators(devStr + QDir::separator() + dirs.at(i));

        QProcess process;
        if(!QDir(usbPath).exists()){
            QString tmpPath = QDir::toNativeSeparators(devStr + QDir::separator() + "\""+ dirs.at(i) + "\"");
            process.start("mkdir -p "+tmpPath);
            process.waitForFinished(50);
        }

        foreach(const QFileInfo &fileInfo, QDir(dirPath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files)){
            //如果拔掉了U盘,退出
            if(m_pool->usbMaps.count() < 1){
                progressBar.hide();
                emit fileOperator(false);
                emit operatorLog(tr("Files export interrupt."));

                ui->labelFileCount->setFocus();
                return;
            }

            QFile file(fileInfo.absoluteFilePath());
            bool isOk = file.copy(usbPath+"/"+fileInfo.fileName());
            if(!isOk) qDebug() << Q_FUNC_INFO << "拷贝失败:" << fileInfo.absoluteFilePath() << (usbPath+"/"+fileInfo.fileName());

            process.start("sync");
            process.waitForFinished();

            QElapsedTimer time;
            time.start();
            while(time.elapsed() < 100){
                QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
            }
            progressBar.setValue(file_cnt++);

        }
    }
#endif

    //开始拷贝文件
    for(int i=0; i<files.count(); i++){
        //如果拔掉了U盘,退出
        if(m_pool->usbMaps.count() < 1){
            progressBar.hide();
            emit fileOperator(false);
            emit operatorLog(tr("Files export interrupt."));

            ui->labelFileCount->setFocus();
            return;
        }

        //得到要拷贝的文件名
        QString tmpTxt = "\""+files.at(i)+"\"";
        QString fileName = m_pool->filePath + tmpTxt;
        QFileInfo fileInfo(fileName);
        qDebug()<<"-------------------//得到要拷贝的文件名-------------files.at(i);--------------------"<<fileName<<tmpTxt;
        QString fn = QDir::toNativeSeparators(devStr + QDir::separator() + fileInfo.fileName());
        int count = 1;
        while(QFile::exists(fn)){
            fn = QDir::toNativeSeparators(devStr + QDir::separator() + fileInfo.completeBaseName() + QString("(%1).").arg(count) + fileInfo.suffix());
            count++;
        }
        fileName = QDir::toNativeSeparators( m_pool->filePath + tmpTxt);
        qDebug()<<"-------------------//得到要拷贝的文件名--------------222-------------------"<<fileName;
        QProcess process;
        QString kk= QString("cp -a ")+fileName+" "+fn;
        process.start(kk);

        bool isOk = process.waitForFinished();
        if(isOk){
            process.start("sync");
            isOk = process.waitForFinished();
            qDebug()<<"-------------------//得到要拷贝的文件名----------555--------"<<isOk;
        }

        QElapsedTimer time;
        time.start();
        while(time.elapsed() < 100){
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }

        progressBar.setValue(file_cnt++);
        qDebug()<<"-------------------//得到要拷贝的文件名----file_cnt------"<<file_cnt;
    }

    progressBar.setValue(fileCount);
    progressBar.hide();
    emit fileOperator(false);
    emit operatorLog(tr("Files export succeed."));

    ui->labelFileCount->setFocus();
}

/*!
* \brief 私有槽函数，点击menu时导入文件到仪器
* \param 无
* \return 无
*/
void GOverView::slot_importExp()
{
    m_pool->screen_sound();

    //设置主控为编辑状态
    m_pool->set_ui_busy_state(1);

    QString devStr;
    if(m_pool->usbMaps.count() < 1){
        //无U盘
    }else if(m_pool->usbMaps.count() == 1){
        devStr = m_pool->usbMaps.firstKey();
    }else{
        GUsbSelectDialog dialog(m_pool->usbMaps, this);
        //        QPoint pos = this->mapToGlobal(this->geometry().center());
        //        dialog.move(pos.x()-dialog.width()/2, pos.y()-dialog.height()/2);
        dialog.show();
        int xx = (m_pool->desktopWidth - dialog.width())/2;
        int yy = (m_pool->desktopHeight - dialog.height())/2;
        dialog.move(xx,yy);

        if(dialog.exec() == QDialog::Accepted){
            devStr = dialog.currentDev();
        }
    }

    if(devStr.isEmpty()){
        ui->labelFileCount->setFocus();
        return;
    }
    qDebug() << "select Usb device:" << devStr;

    GFileCopyDialog dialog(m_pool, GFileCopyDialog::ImportFile, m_pool->filePath, devStr, this);

    if(dialog.exec() == QDialog::Rejected){
        return;
    }

    QStringList files = dialog.selectedFiles();

    emit fileOperator(true);

    QProgressBar progressBar(NULL);
    progressBar.setStyleSheet("QProgressBar{  color : solid #383e83;  border: 2px solid gray;  border-radius: 5px;  background: transparent;  padding: 0px;  text-align : center ;}  QProgressBar::chunk{  background: #c9e5fe;  }");
    progressBar.setRange(0, files.count());
    progressBar.setValue(0);
    progressBar.show();
    int xx = (m_pool->desktopWidth - progressBar.width())/2;
    int yy = (m_pool->desktopHeight - progressBar.height())/2;
    progressBar.move(xx,yy);

    bool hasPrompt = false;     //是否需要提示文件过多的信息
    for(int i=0; i<files.count(); i++){
        if(m_pool->usbMaps.count() < 1){
            progressBar.hide();
            emit fileOperator(false);
            emit operatorLog(tr("Files import interrupt."));


            ui->labelFileCount->setFocus();
            return;
        }

        //判断文件是否已达到 EXPERIMENT_TOTAL_COUNT
        QString tmpTxt = "\""+files.at(i)+"\"";

        QString fileName = QDir::toNativeSeparators(devStr + QDir::separator() + files.at(i));
        QFileInfo fileInfo(fileName);

        QString file1_ = m_pool->filePath + fileInfo.completeBaseName() + QStringLiteral(".tlpp");
        QString file2_ = m_pool->filePath + fileInfo.completeBaseName() + QStringLiteral(".tlpd");
        QString file3_ = m_pool->filePath + fileInfo.completeBaseName() + QStringLiteral(".tlpe");

        int count = 0;
        while(QFile::exists(file1_) || QFile::exists(file2_) || QFile::exists(file3_)){
            count++;
            file1_ = m_pool->filePath + fileInfo.completeBaseName() + QString("(%1).tlpp").arg(count);
            file2_ = m_pool->filePath + fileInfo.completeBaseName() + QString("(%1).tlpd").arg(count);
            file3_ = m_pool->filePath + fileInfo.completeBaseName() + QString("(%1).tlpe").arg(count);
        }

        QString fn;
        if(count < 1){
            fn = m_pool->filePath + tmpTxt;
        }else{
            fn = m_pool->filePath + "\"" + fileInfo.completeBaseName() + QString("(%1).").arg(count) + fileInfo.suffix() + "\"";
        }

        qDebug() << QString("实验%1导入为%2").arg(files.at(i)).arg(fn);

        QString fnp = QDir::toNativeSeparators(devStr + QDir::separator() + tmpTxt);
        QProcess process;
        process.start(tr("cp -a %1 %2").arg(fnp).arg(fn));
        bool isOk = process.waitForFinished();
        if(isOk){
            process.start("sync");
            isOk = process.waitForFinished();
        }

        progressBar.setValue(i+1);

        QElapsedTimer time;
        time.start();
        while(time.elapsed() < 100){
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
        //qDebug() << "=====================================================333" << QTime::currentTime().toString("hh:mm:ss:zzz");
        int currentFileCount = QDir(m_pool->filePath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
        if(currentFileCount>EXPERIMENT_TOTAL_COUNT || m_d->fileCounts.contains(currentFileCount)) hasPrompt = true;
        //qDebug() << "=====================================================444" << QTime::currentTime().toString("hh:mm:ss:zzz");
    }

    progressBar.hide();
    emit fileOperator(false);
    emit operatorLog(tr("Files import succeed."));

    if(hasPrompt){
        My_MessageBox mb;
        connect(&mb, SIGNAL(finished(int)), m_pool, SLOT(screen_sound()));
        mb.ginformation(NULL, NULL, tr("Prompt"), tr("The experiment file is too many, please backup and delete useless files."));
        mb.disconnect(m_pool);
    }

    ui->labelFileCount->setFocus();
}

/*!
* \brief 类GOverView的私有函数，初始化私有变量
* \param 无
* \enum GOverView::ShowType
* \return 无
*/
void GOverView::initVariables()
{
    this->setProperty("FirstLoadDirectory", true);  ///< 是否第一次加载目录

    m_pool->filePath = QDir::toNativeSeparators(SYS_DIR_FILE);

    qDebug()<<"----添加文件模板------------";
    //添加文件模板
    m_d->fileModel = new GFileSystemModel(m_pool);

    if(m_d->fileModel){
        connect(m_d->fileModel, SIGNAL(directoryLoaded(QString)), this, SLOT(slot_directoryLoaded()));
        //设置过滤文件
#ifdef DEVICE_TYPE_TL13
        m_d->fileModel->setFilter(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
#else
        m_d->fileModel->setFilter(QDir::Files);
        m_d->fileModel->setNameFilters(QStringList() << QStringLiteral("*.tlpd") << QStringLiteral("*.tlpp") << QStringLiteral("*.tlpe"));
#endif

        // 没有通过过滤器的文件disable还是隐藏,true为disable false为隐藏
        m_d->fileModel->setNameFilterDisables(false);
        m_d->rootIndex = m_d->fileModel->setRootPath(m_pool->filePath);
    }
    //      代理模板
#ifdef DEVICE_TYPE_TL13
    m_d->proxyModel = new GProxyModel(ICON_COLUMN_COUNT, m_d->rootIndex, &m_d->dirDeep);
#else
    m_d->proxyModel = new GProxyModel(ICON_COLUMN_COUNT, m_d->rootIndex);
#endif
    if(m_d->proxyModel){
        m_d->proxyModel->setSourceModel(m_d->fileModel);
    }
    //两种显示代理
    m_d->iconDelegate = new GFileIconDelegate(ICON_WIDTH, ICON_HEIGHT);
    m_d->detailDelegate = new GFileDetailDelegate();

    //“新建”菜单
    m_d->createMenu = new QMenu(ui->buttonNew);
    //    m_d->createMenu->setStyleSheet("border: 1px solid #2f74d0;");
#ifdef DEVICE_TYPE_TL13
    m_d->createFolder= new QAction(tr("New Folder"), m_d->createMenu);
    m_d->createMenu->addAction(m_d->createFolder);
    m_d->createMenu->addSeparator();
    connect(m_d->createFolder, SIGNAL(triggered()), this, SLOT(slot_createFolder()));

    m_d->dirMenu = new QMenu;
    if(m_d->dirMenu){
        m_d->dirMenu->setStyleSheet(QString("QMenu{border: 1px solid #2f74d0;}\nQMenu::item{padding: 5px 20px 5px 20px;}\nQMenu::item:disabled{background: #c9caca;}"));
        m_d->getinDir = new QAction(tr("Open"), m_d->dirMenu);
        m_d->dirMenu->addAction(m_d->getinDir);
        connect(m_d->getinDir, SIGNAL(triggered()), this, SLOT(g_openDir_triggered()));
    }
#endif
    m_d->createEmptyExp = new QAction(tr("New Experiment"), m_d->createMenu);
    m_d->createExpBaseOnCurrent = new QAction(tr("New Experiment From Selected Experiment"), m_d->createMenu);

    m_d->createMenu->addAction(m_d->createEmptyExp);
    m_d->createMenu->addSeparator();
    m_d->createMenu->addAction(m_d->createExpBaseOnCurrent);

    connect(m_d->createMenu, SIGNAL(aboutToShow()), m_pool, SLOT(screen_sound()));
    connect(m_d->createMenu, SIGNAL(aboutToHide()), m_pool, SLOT(screen_sound()));
    connect(m_d->createEmptyExp, SIGNAL(triggered()), this, SLOT(slot_createEmptyExp()));
    connect(m_d->createExpBaseOnCurrent, SIGNAL(triggered()), this, SLOT(slot_createExpBaseOnCurrent()));

    m_d->swapMenu = new QMenu(ui->buttonInterflow);
    //    m_d->swapMenu->setStyleSheet("border: 1px solid #2f74d0;");
    connect(m_d->swapMenu, SIGNAL(aboutToShow()), m_pool, SLOT(screen_sound()));
    connect(m_d->swapMenu, SIGNAL(aboutToHide()), m_pool, SLOT(screen_sound()));

    m_d->swapExport = new QAction(tr("Export Experiment..."), m_d->swapMenu);
    //    m_d->swapExport = new QAction(tr("Export"), m_d->swapMenu);
    connect(m_d->swapExport, SIGNAL(triggered()), this, SLOT(slot_exportExp()));
    m_d->swapMenu->addAction(m_d->swapExport);
    m_d->swapMenu->addSeparator();
    m_d->swapImport = new QAction(tr("Import Experiment..."), m_d->swapMenu);
    //    m_d->swapImport = new QAction(tr("Import"), m_d->swapMenu);
    connect(m_d->swapImport, SIGNAL(triggered()), this, SLOT(slot_importExp()));
    m_d->swapMenu->addAction(m_d->swapImport);
}

/*!
* \brief 类GOverView的私有函数，初始化designer设计的界面
* \param 无
* \enum GOverView::ShowType
* \return 无
*/
void GOverView::initUi()
{    
    //初始化QTableView
#ifdef DEVICE_TYPE_TL13
    ui->buttonNew->setShowMode(GPushButton::UpLeft);
    ui->buttonNew->setMenu(m_d->createMenu);
#endif
    ui->buttonInterflow->setShowMode(GPushButton::UpRight);
    ui->buttonInterflow->setMenu(m_d->swapMenu);
    ui->buttonInterflow->setEnabled(false);

    //初始化实验文件显示窗口
    ui->tableViewFile->verticalHeader()->hide();
    ui->tableViewFile->verticalScrollBar()->setTracking(false);
    ui->tableViewFile->verticalScrollBar()->installEventFilter(this);
    ui->tableViewFile->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->tableViewFile->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
    ui->tableViewFile->horizontalHeader()->setSortIndicatorShown(true);
    //    ui->tableViewFile->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    connect(ui->tableViewFile->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(slot_sortIndicatorChanged(int, Qt::SortOrder)));

    //    ui->tableViewFile->setStyleSheet(QString::fromUtf8("gridline-color: rgb(0, 0, 0);\n"
    //                                                       "border-top-color: rgb(180, 180, 180);\n"
    //                                                       "alternate-background-color: rgb(207, 236, 255);\n"
    //                                                       "font: 14pt"));

    //自动调整最后一列的宽度使它和表格的右边界对齐
    ui->tableViewFile->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewFile->viewport()->installEventFilter(this);

    //定位文件浏览路径,等待目录加载后(slot_directoryLoaded()函数)再重新设置显示方式
    ui->tableViewFile->setModel(m_d->fileModel);
    ui->tableViewFile->setRootIndex(m_d->rootIndex);

    //其他控件
    ui->buttonRename->setEnabled(false);
    m_d->createExpBaseOnCurrent->setVisible(false);
    slot_usbDeviceChanged();
}

/*!
* \brief 类GOverView的私有函数，实现文件不同显示类型的转换
* \param _type 文件浏览器要显示的类型
* \enum GOverView::ShowType
* \return 无
*/
void GOverView::fileViewChanged(ShowType _type, bool sectionChange)
{
    if(m_d->fileModel==NULL || m_d->proxyModel==NULL) return;

    //取消原来的信号槽连接
    QItemSelection oldSelection;
    if(ui->tableViewFile->selectionModel() != NULL){
        ui->tableViewFile->selectionModel()->disconnect(this);
        oldSelection = ui->tableViewFile->selectionModel()->selection();
    }

    //根据不同的类型显示标题和设置大小
    if(_type == st_Detail){
        //设置显示方式的代理
        ui->tableViewFile->setModel(m_d->fileModel);
        ui->tableViewFile->setRootIndex(m_d->rootIndex);
        ui->tableViewFile->setItemDelegate(m_d->detailDelegate);

        //交叉替换“文件大小”和“修改日期”
        ui->tableViewFile->horizontalHeader()->swapSections(1,3);

        //设置列宽度
        ui->tableViewFile->setColumnWidth(0, DETAIL_COL0_WIDTH);
        ui->tableViewFile->setColumnWidth(3, DETAIL_COL3_WIDTH);
#ifdef DEVICE_TYPE_TL22
        ui->tableViewFile->setColumnWidth(2, DETAIL_COL2_WIDTH);
#else
        ui->tableViewFile->setColumnWidth(1, DETAIL_COL2_WIDTH);
#endif
        ui->tableViewFile->verticalHeader()->setDefaultSectionSize(DETAIL_HEIGHT);
        ui->tableViewFile->horizontalHeader()->show();
        qDebug()<<" -----------------------------------设置表 格的选择方式---------001-------------";
        //设置表 格的选择方式
        ui->tableViewFile->setSelectionBehavior(QAbstractItemView::SelectRows);

        //设置标题栏按键音
        connect(ui->tableViewFile->horizontalHeader(), SIGNAL(sectionClicked(int)), m_pool, SLOT(screen_sound()));

        qDebug() << "file view type changed to detail";
    }else{
        //设置显示方式的代理
        ui->tableViewFile->setModel(m_d->proxyModel);
        ui->tableViewFile->setItemDelegate(m_d->iconDelegate);

        //交叉替换“文件大小”和“修改日期”
        if(sectionChange){
            ui->tableViewFile->horizontalHeader()->swapSections(1,3);
        }

        //设置行、列的高度和宽度
        for(int i=0; i<ICON_COLUMN_COUNT; i++)
            ui->tableViewFile->setColumnWidth(i, ICON_WIDTH);
        for(int i=0; i<m_d->proxyModel->rowCount(); i++)
            ui->tableViewFile->setRowHeight(i, ICON_HEIGHT);

        //设置表格的选择方式
        ui->tableViewFile->setSelectionBehavior(QAbstractItemView::SelectItems);

        //设置颜色间隔
        ui->tableViewFile->setAlternatingRowColors(false);

        qDebug() << "file view type changed to icon";
    }

    //设置纵向Scrollbar范围
    int total = m_d->fileModel->rowCount(m_d->rootIndex);
    if(_type == st_Detail){
        ui->tableViewFile->verticalScrollBar()->setRange(0,total);
    }else{
        int total_row = total / ICON_COLUMN_COUNT;
        int total_col = total % ICON_COLUMN_COUNT;
        if(total_col) total_row++;
        ui->tableViewFile->verticalScrollBar()->setRange(0,total_row);
    }
    ui->tableViewFile->verticalScrollBar()->setPageStep(1);

    //定位到页面,如果有选择的实验文件,则定位到实验文件
    if(_type == st_Detail){
        if(oldSelection.indexes().count() > 0){
            const QModelIndex &index = oldSelection.indexes().first();
            //当前显示模式下的序号
            QModelIndex current = m_d->proxyModel->mapToSource(index);
            //按行排序,每页DETAIL_ROW_COUNT行,每行为1,设置scrollbar的值
            int page = (current.row() / DETAIL_ROW_COUNT) * DETAIL_ROW_COUNT;
            ui->tableViewFile->verticalScrollBar()->setValue(page);
        }else{
            ui->tableViewFile->scrollToTop();
        }
    }else{
        if(oldSelection.indexes().count() > 0){
            const QModelIndex &index = oldSelection.first().topLeft();
            //当前显示模式下的行
            int row = index.row() / ICON_COLUMN_COUNT;
            //按满页排序,每页ICON_COLUMN_COUNT*ICON_ROW_COUNT文件,每行为1,每行ICON_COLUMN_COUNT文件,设置scrollbar的值
            int page = (row / ICON_ROW_COUNT) * ICON_ROW_COUNT;
            ui->tableViewFile->verticalScrollBar()->setValue(page);
        }else{
            ui->tableViewFile->scrollToTop();
        }
    }

    //设置前次选择
    QItemSelection nowSelection;
    if(_type == st_Detail){
        nowSelection = m_d->proxyModel->mapSelectionToSource(oldSelection);
    }else{
        nowSelection = m_d->proxyModel->mapSelectionFromSource(oldSelection);
    }

    ui->tableViewFile->selectionModel()->select(nowSelection, QItemSelectionModel::ClearAndSelect);

    //选择信号槽连接
    connect(ui->tableViewFile->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slot_fileChanged()));

    //    //scrolbar槽连接
    //    connect(ui->tableViewFile->verticalScrollBar(), SIGNAL(sliderReleased()), m_pool, SLOT(screen_sound()));
    //    connect(ui->tableViewFile->verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(g_verticalScrollBarActionTriggered(int)));

    m_d->showType = _type;
    //设置焦点
    ui->tableViewFile->setFocus();
}

/*!
* \brief 类GOverView的私有函数，显示当前文件信息
* \param index 要显示文件的序号
* \return 无
*/
QString GOverView::displayInfo(const QModelIndex &index)
{
    qDebug() << "display file info";

    QString fileName;
    if(m_d->fileModel && index.isValid()){
        QModelIndex currentIndex;
        if(m_d->showType == st_Detail){
            currentIndex = index;
        }else{
            int row = index.row() * ICON_COLUMN_COUNT + index.column();
            currentIndex = m_d->fileModel->index(row,0,m_d->rootIndex);
            ui->tableViewFile->viewport()->update();
        }
        //文件名
        fileName = m_d->fileModel->filePath(currentIndex);
        QString fn = m_d->fileModel->fileName(currentIndex);
        //        ui->labelFileName->setText(tr("File Name: ") + fn);

        if(ui->buttonRename->property("ISRENAME").toBool()){
            ui->buttonRename->setProperty("ISRENAME", false);
        }else{
            m_d->selectedFile = fn;
        }

        //基于当前实验设置新建实验菜单设置有效
        m_d->createExpBaseOnCurrent->setVisible(true);

        //如果实验正在运行,重命名无效
        _PCR_RUN_CTRL_INFO pcrInfo;

        m_pool->mutex.lock();
        memcpy((void*)&pcrInfo, (const void*)m_pool->runCtrlInfo.data(), sizeof(_PCR_RUN_CTRL_INFO));
        m_pool->mutex.unlock();

        bool rename_is_enable_ = false;
        if(pcrInfo.State.ExpState != 0){
    #ifdef DEVICE_TYPE_TL13
            if((fn!=QString("..")) && (fileName!=m_pool->expFile->fileName()) && (fileName!=QFileInfo(m_pool->expFile->fileName()).path())){
                rename_is_enable_ = true;
            }
    #else
            rename_is_enable_ = (fileName != m_pool->expFile->fileName());
    #endif
        }else{
#ifdef DEVICE_TYPE_TL13
            rename_is_enable_ = fn != QString("..");
#else
            rename_is_enable_ = true;
#endif
        }

        //根据文件个数给“新建”按键添加菜单
#ifdef DEVICE_TYPE_TL13
        ui->buttonNew->setShowMode(GPushButton::UpLeft);
        ui->buttonNew->setMenu(m_d->createMenu);
        ui->buttonRename->setEnabled(rename_is_enable_);
#else
        if(ui->buttonNew->menu() != m_d->createMenu){
            ui->buttonNew->setShowMode(GPushButton::UpLeft);
            ui->buttonNew->setMenu(m_d->createMenu);
        }
        if(!ui->buttonRename->isEnabled() && rename_is_enable_)
            ui->buttonRename->setEnabled(true);

#endif        
    }else{
        if(ui->buttonRename->property("ISRENAME").toBool()){
            ui->buttonRename->setProperty("ISRENAME", false);
        }else
            m_d->selectedFile.clear();

        //基于当前实验设置新建实验菜单设置无效
        //        if(m_d->createExpBaseOnCurrent->isEnabled())
        //            m_d->createExpBaseOnCurrent->setEnabled(false);
        m_d->createExpBaseOnCurrent->setVisible(false);

        //根据文件个数给“新建”按键添加菜单
#ifndef DEVICE_TYPE_TL13
        if(ui->buttonNew->menu() != NULL){
            ui->buttonNew->setMenu(NULL);
        }
#endif       
        if(ui->buttonRename->isEnabled())
            ui->buttonRename->setEnabled(false);
    }
    return fileName.trimmed();
}

/*!
* \brief 类GOverView的私有函数，判断当前实验文件个数,提示对旧文件进行整理的信息
* \param 无
* \return 无
*/
void GOverView::promptFileCount()
{
    int currentFileCount = QDir(m_pool->filePath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();

    if(currentFileCount>EXPERIMENT_TOTAL_COUNT || m_d->fileCounts.contains(currentFileCount)){
        My_MessageBox mb;
        mb.ginformation(m_pool, NULL, tr("Prompt"), tr("Too many experiment files, please backup and delete useless files."));
    }
}

int GOverView::currentFileCount()
{
    return QDir(m_pool->filePath).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
}

int GOverView::rootFileCount()
{
    int fileCount = QDir(SYS_DIR_FILE).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
    foreach(QString dirStr, QDir(SYS_DIR_FILE).entryList(QDir::NoDotAndDotDot|QDir::Dirs)){
        QString subDir = QDir::toNativeSeparators(SYS_DIR_FILE+dirStr);
        fileCount += QDir(subDir).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
    }
    return fileCount;
}

#ifdef DEVICE_TYPE_TL13
bool GOverView::rootFileAndDirIsEmpty()
{
    int count = QDir(SYS_DIR_FILE).entryInfoList(QStringList() << "*.tlpp" << "*.tlpd"<<"*.tlpe", QDir::NoDotAndDotDot|QDir::Files).count();
    count += QDir(SYS_DIR_FILE).entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs).count();
    return (count == 0);
}
#endif
