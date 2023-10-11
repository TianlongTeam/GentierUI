/*!
* \file gglobal.h
* \brief ARM板软件中通用定义的头文件
*
*ARM板软件中使用到的包含头文件、常量、宏、变量或数据类型的定义等
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-18 9:41
*
*/

#ifndef GGLOBAL_H
#define GGLOBAL_H

#include <QDebug>

#define DATABASE_NAME   "pinyin.db"

#define REBOOT_CODE     -99
#define SHUTDOWN_CODE   0

#define LANG_ENGLISH    0
#define LANG_CHINESE    1

#define EXPERIMENT_TOTAL_COUNT  1000

#define SYS_DIR_FILE        "./data/"
#define ERR_WARN_INFO_FILE  "./config/error_info.ini"

#define SYS_DIR_LANGUAGE    "language"
#define SYS_DIR_CONFIG      "config"
#define SYS_DIR_LOG         "log"

#define FILEBASENAME_LEN    64
#define FILENAME_LEN        69

#define REMOTE_MAX_COUNT    5

#define PREINCUBATION_AND_COOL  40      ///< 定义过程的预变性与冷却的温度界限
#define TEMPCURVE_START_TEMP    37      ///< 运行设置界面温度曲线的起始温度
#define TEMP_RAMP               3.5     ///< 缺省的温度速率

#define MAX_STAGE   99
#define MAX_STEP    99
#define MAX_CYCLE   99

#define MIN_RAMP    0.1
#define MAX_RAMP   6.1

#define MAX_DURATION    3600
#define MAX_DURATION_DETA   600

#define MIN_REACTION_VOL    0
#define MAX_REACTION_VOL    100
#define MIN_HOTCOVER_VOL    40
#define MAX_HOTCOVER_VOL    110

#define MIN_STD_TEMP    0.0
#define MAX_STD_TEMP    100.0
#define MIN_GRADIENT_TEMP    35.0
#define MAX_GRADIENT_TEMP    100.0

#define MIN_HOTCOVER_TEMP   30.0
#define MAX_HOTCOVER_TEMP   125.0
#define TEMP_DECIMAL    1
#define MIN_CENTER_TEMP    35.5
#define MAX_CENTER_TEMP    99.5

#define MIN_OFFSET_TEMP    0.5
#define MAX_OFFSET_TEMP    20.0

#define MIN_TOUCHDOWN_TEMP  35.0
#define MAX_TOUCHDOWN_TEMP  100.0

#define MIN_TEMP_DETA   0.1
#define MAX_TEMP_DETA   5.0

#define MIN_OFFSET_TIME    1.0
#define MAX_OFFSET_TIME    40.0

#define MIN_MELT_TEMP1   0.0
#define MIN_MELT_TEMP2   35.0
#define MAX_MELT_TEMP   100.0

#define MIN_SINGLESTEP_TEMP 0.1
#define MAX_SINGLESTEP_TEMP 5.0

#define MIN_CONTINUE_TIMES  2
#define MAX_CONTINUE_TIMES  15

#define MIN_OPTICAL_ORIGIN  -32768
#define MAX_OPTICAL_ORIGIN  32767

#define MIN_LED_DRV_CUR     0
#define MAX_LED_DRV_CUR     100

#define PRINTER_ADDR    "/dev/usblp0"

///////////////////////////////////////////////////////

static const QString Ch1FluorContent[] = { \
    QLatin1String("FAM"), \
    QLatin1String("SYBR Green I"), \
    QLatin1String("LCGreen"), \
    QLatin1String("EvaGreen"), \
    QLatin1String("SYTO 9")
};
static const QString Ch2FluorContent[] = { \
    QLatin1String("HEX"), \
    QLatin1String("TET"), \
    QLatin1String("VIC"), \
    QLatin1String("JOE")
};
static const QString Ch3FluorContent[] = { \
    QLatin1String("Texas Red"), \
    QLatin1String("ROX")
};
static const QString Ch4FluorContent[] = { \
    QLatin1String("Cy5")
};
static const QString Ch5FluorContent[] = { \
    QLatin1String("Alexa Fluor 680"),
    QLatin1String("Tamra"),
    QLatin1String("Cy3"),
    QLatin1String("NED")
};
static const QString Ch6FluorContent[] = { \
    QLatin1String("FRET"),
    QLatin1String("Tamra"),
    QLatin1String("Cy3"),
    QLatin1String("NED"),
    QLatin1String("Qdot 585"),
    QLatin1String("HiLyte Fluor 750")
};

class GHelper : public QObject
{
    Q_OBJECT
public:
    static int total_instrument_id;

    static int channel_count;
    static int row_count;
    static int column_count;
    static bool has_gradient;

    static QString deviceName;
    static QString deviceSerial;
    static QString deviceTypeName;

    static void setInstuType(int id){
        total_instrument_id = id;
        switch(id){
        case 1:{
            channel_count = 6;
            row_count = 8;
            column_count = 12;
            has_gradient = true;
            break;
        }
        case 2:{
            channel_count = 6;
            row_count = 8;
            column_count = 12;
            has_gradient = false;
            break;
        }
        case 3:{
            channel_count = 4;
            row_count = 8;
            column_count = 12;
            has_gradient = true;
            break;
        }
        case 4:{
            channel_count = 4;
            row_count = 8;
            column_count = 12;
            has_gradient = false;
            break;
        }
        case 5:{
            channel_count = 4;
            row_count = 8;
            column_count = 6;
            has_gradient = true;
            break;
        }
        case 6:{
            channel_count = 2;
            row_count = 8;
            column_count = 6;
            has_gradient = false;
            break;
        }
        case 7:{
            channel_count = 2;
            row_count = 8;
            column_count = 6;
            has_gradient = true;
            break;
        }
        case 10:{
            channel_count = 4;
            row_count = 8;
            column_count = 12;
            has_gradient = false;
            break;
        }
        case 11:{
            channel_count = 2;
            row_count = 2;
            column_count = 24;
            has_gradient = false;
            break;
        }
        case 12:{
            channel_count = 4;
            row_count = 8;
            column_count = 12;
            has_gradient = true;
            break;
        }
        case 14:{
            channel_count = 4;
            row_count = 8;
            column_count = 6;
            has_gradient = false;
            break;
        }
        case 15:{
            channel_count = 2;
            row_count = 8;
            column_count = 6;
            has_gradient = false;
            break;
        }
        case 201:
        case 202:
        case 204:
        case 205:{
            channel_count = 6;
            row_count = 8;
            column_count = 12;
            has_gradient = true;
            break;
        }
        case 203:{
            channel_count = 6;
            row_count = 8;
            column_count = 12;
            has_gradient = false;
            break;
        }
        default:break;
        };
    }

    static int indexOfFluorGroup(int groupId)
    {
        switch(groupId)
        {
        case 0:return 0;
        case 1:return 1;
        case 2:return 2;
        case 3:return 3;
        case 4:return 4;
        case 5:
              return 5;
        case 6:
              return 7;

        default:return 255;
        }
    }

    static int countOfFluorGroup(int groupId)
    {
        switch(groupId)
        {
        case 1:
            return 5;
        case 2:
            return 4;
        case 3:
            return 2;
        case 4:
            return 1;
        case 5:
             return 4;
        case 6:
             return 6;
        default:return 255;
        }
    }

    static QString keyOfFluor(int groupId, int index)
    {
        switch(groupId){
        case 1:return (index)<countOfFluorGroup(1) ? Ch1FluorContent[index] : QString();
        case 2:return (index)<countOfFluorGroup(2) ? Ch2FluorContent[index] : QString();
        case 3:return (index)<countOfFluorGroup(3) ? Ch3FluorContent[index] : QString();
        case 4:return (index)<countOfFluorGroup(4) ? Ch4FluorContent[index] : QString();

        case 5:
            if(total_instrument_id == 201){
                return index<4 ? Ch5FluorContent[index] : QString();
            }else{
                return index<countOfFluorGroup(5) ? Ch5FluorContent[index] : QString();
            }
        case 6:
            if(total_instrument_id==202 || total_instrument_id == 203){
                return index<4 ? Ch6FluorContent[index] : QString();
            }else if(total_instrument_id==204){
                return index<5 ? Ch6FluorContent[index] : QString();
            }else if(total_instrument_id==205){
                return index<6 ? Ch6FluorContent[index] : QString();
            }else{
                return index<countOfFluorGroup(6) ? Ch6FluorContent[index] : QString();
            }      
        default:return QString();
        }
    }

    static QString keyOfAllFluor(int globalIndex)
    {
        switch((globalIndex>>0x04)&0x0f)
        {
            case 1:return (globalIndex&0x0f)<countOfFluorGroup(1) ? Ch1FluorContent[globalIndex&0x0f] : QString();
            case 2:return (globalIndex&0x0f)<countOfFluorGroup(2) ? Ch2FluorContent[globalIndex&0x0f] : QString();
            case 3:return (globalIndex&0x0f)<countOfFluorGroup(3) ? Ch3FluorContent[globalIndex&0x0f] : QString();
            case 4:return (globalIndex&0x0f)<countOfFluorGroup(4) ? Ch4FluorContent[globalIndex&0x0f] : QString();
            case 5:return (globalIndex&0x0f)<4 ? Ch5FluorContent[globalIndex&0x0f] : QString();
            case 6:return (globalIndex&0x0f)<6 ? Ch6FluorContent[globalIndex&0x0f] : QString();
            default:return QString();
        }
    }

    static QString byteArrayToHexStr(QByteArray data)
    {
        QString temp = "";
        QString hex = data.toHex();
        for(int i=0; i<hex.length(); i+= 2){
            temp += hex.mid(i,2);
        }
        return temp.trimmed().toUpper();
    }

    static QByteArray hexStrToByteArray(QString str)
    {
        QByteArray ba;
        while(str.count() > 0){
            bool isOk = false;
            ba.append(str.left(2).toInt(&isOk, 16));
            str.remove(0,2);
        }
        return ba;
    }            
};

#endif // GGLOBAL_H
