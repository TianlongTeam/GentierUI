/*!
* \file gwarningfile.cpp
* \brief ARM板软件中告警信息文件定义
*
*实现了ARM仪器告警信息文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2015-04-20 16:35
*
*/

#include "gwarningfile.h"

#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QDir>

GWarningFile::GWarningFile()
{
    fileName = QDir::toNativeSeparators(qApp->applicationDirPath()+ QDir::separator() + SYS_DIR_CONFIG + QDir::separator() + WARNING_FILE);

    openFile();
}

GWarningFile::~GWarningFile()
{
    saveFile();
}

void GWarningFile::openFile()
{
    //如果存在lock文件，则删除lock文件读取带后缀的时间最近的文件配置
    bool exist_lock_file_ = false;
    bool not_first_file_ = false;
    QString lockfile = QStringLiteral(WARNING_FILE)+".*";
    QString last_conf_file_;
    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        if(fileInfo.suffix() == QStringLiteral("lock")){
            exist_lock_file_ = true;
        }else if(not_first_file_){
            if(fileInfo.lastModified() > QFileInfo(last_conf_file_).lastModified()){
                last_conf_file_ = fileInfo.filePath();
            }
        }else{
            not_first_file_ = true;
            last_conf_file_ = fileInfo.filePath();
        }
    }

    if(exist_lock_file_ && !last_conf_file_.isEmpty()){
        QFile(fileName).remove();
        QFile(last_conf_file_).rename(fileName);
    }

    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        QFile(fileInfo.filePath()).remove();
    }

    QSettings warnSettings(fileName, QSettings::IniFormat);
    warnSettings.setIniCodec("UTF-8");

    infoList.clear();
    warnSettings.beginGroup("Warning_Info");
    int size = warnSettings.beginReadArray("Item");
    for(int i=0; i<size; i++){
        warnSettings.setArrayIndex(i);
        GWarningItem item;
        item.type = warnSettings.value("Type").toInt();
        item.src = warnSettings.value("Source").toString();
        item.code = warnSettings.value("Code").toString();
        item.dateTime = warnSettings.value("DateTime").toString();
        item.describe = warnSettings.value("Describe").toString();
        infoList << item;
    }
    warnSettings.endArray();
    warnSettings.endGroup();
}

void GWarningFile::saveFile()
{
    QString lockfile = WARNING_FILE;
    lockfile += ".*";

    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        QFile(fileInfo.filePath()).remove();
    }

    QSettings warnSettings(fileName, QSettings::IniFormat);
    warnSettings.setIniCodec("UTF-8");

    warnSettings.beginGroup("Warning_Info");
    warnSettings.beginWriteArray("Item");
    for(int i=0; i<infoList.count(); i++){
        warnSettings.setArrayIndex(i);
        warnSettings.setValue("Type", infoList.at(i).type);
        warnSettings.setValue("Source", infoList.at(i).src);
        warnSettings.setValue("Code", infoList.at(i).code);
        warnSettings.setValue("DateTime", infoList.at(i).dateTime);
        warnSettings.setValue("Describe", infoList.at(i).describe);
    }
    warnSettings.endArray();
    warnSettings.endGroup();
}

void GWarningFile::append(const GWarningItem &item)
{
    qDebug() << "Warning file append from:" << infoList.count();

    QSettings warnSettings(fileName, QSettings::IniFormat);
    warnSettings.setIniCodec("UTF-8");

    warnSettings.beginGroup("Warning_Info");
    warnSettings.beginWriteArray("Item");
    warnSettings.setArrayIndex(infoList.count());
    warnSettings.setValue("Type", item.type);
    warnSettings.setValue("Source", item.src);
    warnSettings.setValue("Code", item.code);
    warnSettings.setValue("DateTime", item.dateTime);
    warnSettings.setValue("Describe", item.describe);
    warnSettings.endArray();
    warnSettings.endGroup();

    infoList.append(item);
}
