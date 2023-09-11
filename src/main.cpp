#include "gwidget.h"
#include "gglobal.h"

#include "mymessagebox.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QDateTime>
#include <QPixmap>
#include <QSqlDatabase>
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QElapsedTimer>
#include <QBoxLayout>
#include <QProgressBar>
#include <QProcess>
#include <QLabel>
#include <QProxyStyle>

#define FONTSIZE   12

class MyProxyStyle : public QProxyStyle
{
public:
    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption * option,
                               QPainter * painter, const QWidget * widget = 0) const
    {
        if(element == PE_FrameFocusRect){
            /// do not draw focus rectangle
        }else{
            QProxyStyle::drawPrimitive(element, option,painter, widget);
        }
    }
};

// 创建数据库连接
bool create_DataBase_Connection()
{
    // check QSqlite dabase dirver
//    qDebug() << "Current WorkPath : " << QDir::currentPath();

    QStringList drivers = QSqlDatabase::drivers();
    foreach (QString str, drivers)
    {
        qDebug() << str;
    }
    if(!(drivers.contains("QSQLITE",Qt::CaseInsensitive)))
    {
        My_MessageBox::information(NULL,QObject::tr("Database initializing"),QObject::tr("QSQLITE database drivers not found!"));
        return false;
    }
    // check dabase file exists.
    QString dbFile = qApp->applicationDirPath() + QDir::separator() + DATABASE_NAME;
    if(!(QFile::exists(dbFile)))
    {
        My_MessageBox::information(NULL,QObject::tr("Database initializing"),QObject::tr("Database file does not exist!"));
        return false;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFile);
    if (!db.open())
    {
        My_MessageBox::warning(NULL,QObject::tr("Database initializing"),QObject::tr("Cannot open database"));
        //        QSqlDatabase::removeDatabase("db1");
        return false;
    }
    return true;
}


#if defined (DEVICE_TYPE_TL22)
int GHelper::total_instrument_id = 1;
int GHelper::channel_count = 6;
int GHelper::row_count = 8;
int GHelper::column_count = 12;
bool GHelper::has_gradient = true;
#endif

QString GHelper::deviceName = QString();
QString GHelper::deviceSerial = QString();
QString GHelper::deviceTypeName = QString();

///////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(armui_rcs);

    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    a.setApplicationVersion("3.8.029");

    QFont font;
    font.setPointSize(FONTSIZE);
    font.setFamily("WenQuanYi Zen Hei");
    font.setBold(false);
    a.setFont(font);

    //判断日志目录是否存在
    QProcess process;
    if(!QDir("~/log").exists()){
        process.start("mkdir ~/log");
        process.waitForFinished();
    }
    if(!QDir("~/log/ui.log").exists()){
        process.start("mkdir ~/log/ui.log");
        process.waitForFinished();
    }
    if(!QDir("~/log/control.log").exists()){
        process.start("mkdir ~/log/control.log");
        process.waitForFinished();
    }

    QFile file(":/file/stylefile");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        QString txt = in.readAll();
        a.setStyleSheet(txt);
        file.close();
        qDebug() << "Style file read Ok";
    }else{
        qDebug() << "Style file read Failed";
    }

    if(!create_DataBase_Connection()) return 1;

    if(argc >= 2){
        int _id = QString(argv[1]).toInt();
        GHelper::setInstuType(_id);
    }

    QRect deskRect = qApp->desktop()->availableGeometry();
    qDebug() << "Desktop available geometry:" << deskRect;

    GWidget w(deskRect);
    w.setOpenDateTime();

    w.g_changeLanguage(w.defaultLanguageId());

    w.showMaximized();
    a.setStyle(new MyProxyStyle);//添加style为这个工程

    return a.exec();
}
