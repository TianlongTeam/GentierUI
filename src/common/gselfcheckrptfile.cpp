/*!
* \file gselfcheckrptfile.cpp
* \brief ARM板软件中自检报告文件定义
*
*实现了ARM仪器自检报告文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2015-04-20 09:44
*
*/

#include "gselfcheckrptfile.h"

#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QDir>

GSelfCheckRptFile::GSelfCheckRptFile()
{
    fileName = QDir::toNativeSeparators(qApp->applicationDirPath()+ QDir::separator() + SYS_DIR_CONFIG + QDir::separator() + SELFTEST_FILE);
    clear();

    openReport();
}

GSelfCheckRptFile::~GSelfCheckRptFile()
{
    saveReport();
}

void GSelfCheckRptFile::openReport()
{
    bool exist_lock_file_ = false;
    bool not_first_file_ = false;
    QString lockfile = QStringLiteral(SELFTEST_FILE)+".*";
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

    QSettings reportSettings(fileName, QSettings::IniFormat);
    reportSettings.setIniCodec("UTF-8");

    reportSettings.beginGroup("SelfTest");
    runCounter = reportSettings.value("RunCounter", 0).toULongLong();     ///< 运行实验的次数
    runMinutes = reportSettings.value("RunMinutes", 0).toULongLong();       ///< 仪器运行时间
    thermodynamics = reportSettings.value("Thermodynamics", false).toBool(); ///< 热学模块
    coverDrive = reportSettings.value("CoverDrive", false).toBool();     ///< 热盖驱动
    photometer = reportSettings.value("Photometer", false).toBool();     ///< 光学模块
    reportSettings.endGroup();
}

void GSelfCheckRptFile::saveReport()
{
    QString lockfile = SELFTEST_FILE;
    lockfile += ".*";
    foreach(const QFileInfo &fileInfo, QDir(qApp->applicationDirPath() + QDir::separator() + SYS_DIR_CONFIG).entryInfoList(QStringList() << lockfile, QDir::NoDotAndDotDot|QDir::Files)){
        QFile(fileInfo.filePath()).remove();
    }

    QSettings reportSettings(fileName, QSettings::IniFormat);
    reportSettings.setIniCodec("UTF-8");

    reportSettings.beginGroup("SelfTest");
    reportSettings.setValue("Result", checkResult);    ///< 自检结果：成功或者失败
    reportSettings.setValue("Datetime", dateTime);       ///< 自检时间
    reportSettings.setValue("RunCounter", runCounter);     ///< 运行实验的次数
    reportSettings.setValue("RunMinutes", runMinutes);       ///< 仪器运行时间
    reportSettings.setValue("Thermodynamics", thermodynamics); ///< 热学模块
    reportSettings.setValue("CoverDrive", coverDrive);     ///< 热盖驱动
    reportSettings.setValue("Photometer", photometer);     ///< 光学模块
    reportSettings.endGroup();
}

void GSelfCheckRptFile::clear()
{
    checkResult = false;    ///< 自检结果：成功或者失败
    dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");       ///< 自检时间
    runCounter = 0;     ///< 运行实验的次数
    runMinutes = 0;       ///< 仪器运行时间
    thermodynamics = false; ///< 热学模块
    coverDrive = false;     ///< 热盖驱动
    photometer = false;     ///< 光学模块
}
