/*!
* \file goverview.h
* \brief ARM板软件中实验管理界面头文件
*
*实现了ARM板软件实验文件的新建、重命名、删除、导入和导出功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 10:35
*
*/

#ifndef GOVERVIEW_H
#define GOVERVIEW_H

#include <QWidget>
//#include <QMetaType>
#include <QModelIndex>

//#include <QWindowsStyle>

namespace Ui {
class GOverView;
}

class QModelIndex;
class GDataPool;
class GOverView : public QWidget
{
    Q_OBJECT

public:
    enum ShowType{
        st_Icon,
        st_Detail
    };

    enum FileOperatorType{
        FOT_None = -1,
        FOT_Clear = 0,
        FOT_New,
        FOT_Open
    };

    explicit GOverView(GDataPool *dataPool, QWidget *parent = 0);
    ~GOverView();

    int  setSelectFile(const QString &fileName);
    void experimentState(bool run);
    void clearSelection();
public slots:
    void slot_usbDeviceChanged();
    void slot_clearCurrentFile();
signals:
    void expFileFocus(bool focus);
    void expFileChanged(const QString &fileName, int type = FOT_None);
    void next();

    void operatorLog(const QString &log);

    void fileOperator(bool);
protected:
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
private slots:    
    void on_buttonNew_clicked();
    void on_buttonRename_clicked();
    void on_buttonDelete_clicked();
    void on_buttonInterflow_clicked();

    void g_btnFileShow_clicked();
    void slot_createEmptyExp();
    void slot_createExpBaseOnCurrent();
    void slot_createFolder();
    void slot_directoryLoaded();    
    void slot_fileChanged();
    void slot_sortIndicatorChanged(int, Qt::SortOrder);
    void g_reflesh_detailShow();

#ifdef DEVICE_TYPE_TL13
    void g_openDir_triggered();
    bool slot_DelFolder(QString folderpath);
#endif    

    void slot_exportExp();
    void slot_importExp();
private:
    void initVariables();
    void initUi();    
    void initFileTableView();
    void fileViewChanged(ShowType _type, bool sectionChange = false);
    QString displayInfo(const QModelIndex &index = QModelIndex());
    void promptFileCount();
    int currentFileCount();     //当前目录中实验文件的个数
    int rootFileCount();        //数据根目录中实验文件的个数
#ifdef DEVICE_TYPE_TL13
    bool rootFileAndDirIsEmpty();  //数据根目录中实验文件和文件夹的个数
#endif
private:
    Ui::GOverView *ui;

    class PrivateData;
    PrivateData *m_d;           ///<私有成员变量类指针

    GDataPool *m_pool;
};

Q_DECLARE_METATYPE(GOverView::ShowType)

//调整Menu中icon的大小，使用setStyle()
//class NoFocusRectangleStyle : public QWindowsStyle
//{
//    Q_OBJECT

//public:
//    NoFocusRectangleStyle() {}

//    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
//                                              QPainter *painter, const QWidget *widget) const
//    {
//        if (QStyle::PE_FrameFocusRect == element && widget &&( widget->inherits("QAbstractItemView")))
//            return;
//        QWindowsStyle::drawPrimitive(element, option, painter, widget);
//    }
//    int pixelMetric ( PixelMetric pm, const QStyleOption * opt, const QWidget * widget) const
//    {
//        int s = QWindowsStyle::pixelMetric(pm, opt, widget);
//        if (pm == QStyle::PM_SmallIconSize) {
//            s = 35;//返回ICON的大小
//        }
//        return s;

//    }
//};

#endif // GOVERVIEW_H
