/*!
* \file gwarningfile.cpp
* \brief ARM板软件中告警信息文件定义头文件
*
*实现了ARM仪器告警信息文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2015-04-20 16:35
*
*/

#ifndef GWARNINGFILE_H
#define GWARNINGFILE_H

#include "gglobal.h"
#include <QMetaType>

#define WARNING_FILE    "warning.ini"

class GWarningItem{
public:
    int         type;       ///< 告警类型 0警告；１故障
    QString     src;        ///< 告警来源
    QString     code;       ///< 告警代码
    QString     dateTime;   ///< 告警日期
    QString     describe;   ///< 告警详细信息

    GWarningItem & operator=(const GWarningItem &item){
        this->type = item.type;
        this->src = item.src;
        this->code = item.code;
        this->dateTime = item.dateTime;
        this->describe = item.describe;
        return *this;
    }
};
Q_DECLARE_METATYPE(GWarningItem)

class GWarningFile
{
public:
    GWarningFile();
    ~GWarningFile();

    void openFile();
    void saveFile();

    void append(const GWarningItem &item);

    QList<GWarningItem> infoList;   ///< 告警信息
    QString fileName;   ///< 告警信息文件名
};

#endif // GWARNINGFILE_H
