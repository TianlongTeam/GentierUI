/*!
* \file gexperimentfile.h
* \brief ARM板软件中实验文件定义头文件
*
*实现了ARM板软件实验文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-28 10:09
*
*/

#ifndef GEXPERIMENTFILE_H
#define GEXPERIMENTFILE_H

//#include <QObject>
#include <QRect>
#include <QPair>
#include<qmap.h>
#include <QMetaType>

#include "gglobal.h"
#include "pcr_info.h"

#ifdef Q_OS_LINUX
#define TMP_DIRCTORY    ".pcrtmp"
#else
#define TMP_DIRCTORY    "~pcrtmp"
#endif

#define ERROR_FILENAME_EMPTY        1
#define ERROR_FILE_NOT_EXIST        2
#define ERROR_NO_INSTRUMENT_INFO    3
#define ERROR_INSTRUMENT_TYPEID     4
#define ERROR_INSTRUMENT_TYPE_FLAG  5
#define ERROR_CHANNEL_COUNT         6
#define ERROR_WELL_COUNT            7
#define ERROR_HAS_GRADIENT          8
#define ERROR_NO_FILE_INFO          9
#define ERROR_NO_RUN_INFO           10
#define ERROR_RUNINFO_CHANNEL_COUNT 11
#define ERROR_COEFFICIENT_COUNT     12
#define ERROR_NO_RUN_METHOD         13
#define ERROR_NO_SAMPLE_INFO        14
#define ERROR_SAMPLE_COUNT          15
#define ERROR_SAMPLE_NO_INFO        16
#define ERROR_SAMPLE_CHANNEL_COUNT  17
#define ERROR_NO_SAMPLE_DATA_INFO   18
#define ERROR_SAMPLE_DATA_COUNT     19
#define ERROR_DATA_CHANNEL_COUNT    20
#define ERROR_DATA_COUNT_NOT_SAME   21
#define ERROR_NO_ANALYSIS_METHOD    22
#define ERROR_NO_TEMP_DATA_INFO     23

class GInstrumentInfo{
public:
    GInstrumentInfo(){
        clear();
    }

    void clear(){
        typeId = GHelper::total_instrument_id;
        channelCount = 0;
        rowCount = 0;
        columnCount = 0;
    }

    void setTypeId(int id){
        typeId = id;
        switch(id){
        case 1:{
            channelCount = 6;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 2:{
            channelCount = 6;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 3:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 4:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 5:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 6;
            break;
        }
        case 6:{
            channelCount = 2;
            rowCount = 8;
            columnCount = 6;
            break;
        }
        case 7:{
            channelCount = 2;
            rowCount = 8;
            columnCount = 6;
            break;
        }
        case 8:{
            channelCount = -1;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 9:{
            channelCount = -1;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 10:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 11:{
            channelCount = 2;
            rowCount = 2;
            columnCount = 24;
            break;
        }
        case 12:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 14:{
            channelCount = 4;
            rowCount = 8;
            columnCount = 6;
            break;
        }
        case 15:{
            channelCount = 2;
            rowCount = 8;
            columnCount = 6;
            break;
        }
        case 201:
        case 202:
        case 204:
        case 205:{
            channelCount = 6;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        case 203:{
            channelCount = 6;
            rowCount = 8;
            columnCount = 12;
            break;
        }
        default:break;
        };
    }

    GInstrumentInfo &operator= (const GInstrumentInfo &info){
        this->typeId = info.typeId;
        this->channelCount = info.channelCount;
        this->rowCount = info.rowCount;
        this->columnCount = info.columnCount;
        return *this;
    }

    int typeId;         ///< 仪器类型 0~6
    int channelCount;   ///< 通道数
    int rowCount;       ///< 行数
    int columnCount;    ///< 列数
};

class GExpFileInfo{
public:
    GExpFileInfo(){
        clear();
    }

    void clear(){
        createTime.clear();
        modifyTime.clear();
    }
    QString fileName;   ///< 文件名
    QString createTime; ///< 文件生成时间
    QString modifyTime; ///< 文件修改时间
};
Q_DECLARE_METATYPE(GExpFileInfo *)

class GExpRunInfo{
public:
    GExpRunInfo(){
        clear();
    }

    void clear(){
        reactionValume = 25;
        lidStatus = 1;
        lidValue = 105.0;
    }

    GExpRunInfo &operator= (const GExpRunInfo &info){
        this->reactionValume = info.reactionValume;
        this->lidStatus = info.lidStatus;
        this->lidValue = info.lidValue;

        return *this;
    }

    int     reactionValume;
    int     lidStatus;
    double  lidValue;
};

typedef QList< _STAGE_INFO* > GExpRunMethod;
Q_DECLARE_METATYPE( _FLUOR_SCAN_INFO* )
typedef QList< _FLUOR_SCAN_INFO * > GWellFluorDataInfo_n;


class GExperimentFile : public QObject
{
    Q_OBJECT
public:
    explicit GExperimentFile(QObject *parent = 0);
    explicit GExperimentFile(const QString &fileName, QObject *parent = 0);
    ~GExperimentFile();

    QString fileName() const {return m_expFileInfo.fileName;}
    QString createTime() const {return m_expFileInfo.createTime;}
    QString modifyTime() const {return m_expFileInfo.modifyTime;}

    bool isValid() const;

    void setFile(const QString &fileName);
    void setConfig(const GExperimentFile &fileConfig);

    void clear();
    int  open(double *maxSpeed, bool not_del_dat = false);
    int  save();
signals:
    void fileSizeChanged();
public:
    int is_compressed_file;
    GExpFileInfo    m_expFileInfo;
    GExpRunInfo     m_expRunInfo;
    GExpRunMethod   m_expRunMethod;     ///< 运行方法
    GWellFluorDataInfo_n mn_wellAmpData;
    GWellFluorDataInfo_n mn_wellMeltData;
private:   
    void clearAllExceptFileName(bool not_del_dat = false);
    void clearRunMethod();
    void clearWellData();

    void saveRunCtrlFile(const QString &fileName, const QString &baseName);
    int  openRunCtrlFile(const QString &fileName, double *maxSpeed);
    int  openExpDataFile(const QString &fileName);

    QString getUnzipPath(const QString &fileName) const;

    QString extract(const QString &zip_file);
};


#endif // GEXPERIMENTFILE_H
