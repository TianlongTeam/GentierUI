/*!
* \file gselfcheckrptfile.h
* \brief ARM板软件中自检报告文件定义头文件
*
*实现了ARM仪器自检报告文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2015-04-20 09:44
*
*/


#ifndef GSELFCHECKRPTFILE_H
#define GSELFCHECKRPTFILE_H

#include "gglobal.h"

#define SELFTEST_FILE   "selftest.ini"

class GSelfCheckRptFile
{
public:
    GSelfCheckRptFile();
    ~GSelfCheckRptFile();

    void openReport();
    void saveReport();
    void clear();

    bool        checkResult;    ///< 自检结果：成功或者失败
    QString     dateTime;       ///< 自检时间
    quint64     runCounter;     ///< 运行实验的次数
    quint64     runMinutes;       ///< 仪器运行时间
    bool        thermodynamics; ///< 热学模块
    bool        coverDrive;     ///< 热盖驱动
    bool        photometer;     ///< 光学模块

    QString     fileName;
};

#endif // GSELFCHECKRPTFILE_H
