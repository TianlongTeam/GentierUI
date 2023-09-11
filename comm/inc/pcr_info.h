#ifndef PCR_INFO_H
#define PCR_INFO_H

#include <QMap>

typedef struct __STRUCT_DEVICE_VERSION_BITS{
    quint32 VerMajor;
    quint32 VerMinor;
    quint32 VerRevision;
}_DEVICE_VERSION_BITS;

typedef union __UNION_DEVICE_VERSION{
    quint32       Value;
    _DEVICE_VERSION_BITS  Bits;
}_DEVICE_VERSION;

////-------------------------------------------------------------------------------------------------------

typedef struct __STRUCT_FLUOR_DATA
{
    quint32 Fluor[576];
    __STRUCT_FLUOR_DATA() { this->_memset(); }
    void _memset()      { memset( (void*)(this),        '\0', this->_size()); }
    uint _size()        { return sizeof(__STRUCT_FLUOR_DATA); }
    uint _sizeFluor()   { return sizeof(this->Fluor); }

    ///< 结构体赋值
    bool _assign(const void* const value) {
        if (value == 0) { return false; }
        this->_memset();
        memcpy( (void*)(this), value, this->_size() );
        return true;
    }
}_FLUOR_DATA;

typedef struct __STRUCT_FLUOR_SCAN_INFO
{
    _FLUOR_DATA FluorValue;
    quint32 SaveFluorFileNo;
    quint32 No;
    quint32 Type;
    __STRUCT_FLUOR_SCAN_INFO() { this->_memset(); }
    void _memset()      { memset( (void*)(this), '\0', this->_size() ); }
    uint _size()        { return sizeof(__STRUCT_FLUOR_SCAN_INFO); }

    ///< 结构体赋值
    bool _assign(const void* const value) {
        if (value == 0) { return false; }
        this->_memset();
        memcpy( (void*)(this), value, this->_size() );
        return true;
    }
}_FLUOR_SCAN_INFO;

typedef struct __STAGE_INFO
{
public:
    __STAGE_INFO(){
        memset(this, 0, sizeof(__STAGE_INFO));
    };
    char Name[64];              /*! Stage名称  */
    quint16 Property;           /*! Stage 属性  */
    quint16 Cycles;             /*! Stage 循环数  */
    quint16 SubNum;
    float   Temp[5];
    quint32 Ramp[5];
    quint32 Time[5];
    quint32 SubProperty[5];
    float   ReadInterval[5];
    quint32 ReadFluor[5];
    quint32 ReadMode[5];
    float   TarValue[5];
    quint32 BeginCycle[5];
    quint32 Delta[5];
}_STAGE_INFO;


typedef struct __STRUCT_PCR_CTRL_BITS
{
    quint8 Error               :1;
    quint8 Warning             :1;
    quint8 DrvLockSoft         :1;
    quint8 ExpState            :2;
    quint8 ThState             :1;
    quint8 ThKeep              :1;
    quint8 ThFlag              :1;
    quint8 OpDataReady         :1;
    quint8 OpHeadMoveState     :3;
    quint8 DrvLidTCtrl         :1;
    quint8 DrvLidKeep          :1;
    quint8 DrvDrawerPos        :1;
    quint8 DrvLidPos           :1;
    quint8 DrvLockMechanical   :1;
    quint8 UpdateMcu           :2;
    quint8 AmpFlag             :1;
    quint8 DrvMotoBusy         :1;
    quint8 SyncFlag            :3;
    ///< 与结构体内存无关快捷函数------------------------------------------
    __STRUCT_PCR_CTRL_BITS()    { this->_memset(); }
    void _memset()              { memset( (void*)(this), '\0', this->_size() ); }
    uint _size()                { return sizeof( __STRUCT_PCR_CTRL_BITS ); }
}_PCR_CTRL_BITS;

typedef struct __STRUCT_RUN_SIZE_NO
{
    quint16 Stage;
    quint16 Cycle;
    quint16 Step;
    quint16 Resv;
}_RUN_SIZE_NO;

typedef struct __STRUCT_PCR_TIMES
{
    quint32 CurrentTimes;
    quint32 TotalTimes;
}_PCR_TIMES;

typedef struct __STRUCT_PCR_RUN_SIZE_INFO
{
    _RUN_SIZE_NO Size;
    _RUN_SIZE_NO No;
    _PCR_TIMES StepTime;
    _PCR_TIMES ExpTime;
}_PCR_RUN_SIZE_INFO;

typedef struct __STRUCT_ERROR_CODE
{
    quint8 Errorstate;
    quint8 ErrorCode;
    quint8 WarningState;
    quint8 WarningCode;
}_ERROR_CODE;

typedef struct __STRUCT_PCR_RUN_ERROR_INFO
{
    _ERROR_CODE Arm;
    _ERROR_CODE Driver;
    _ERROR_CODE Optical;
    _ERROR_CODE Thermal;
    ///< 与结构体内存无关快捷函数------------------------------------------
    __STRUCT_PCR_RUN_ERROR_INFO()    { this->_memset(); }
    void _memset()              { memset( (void*)(this), '\0', this->_size() ); }
    uint _size()                { return sizeof( __STRUCT_PCR_RUN_ERROR_INFO ); }
}_PCR_RUN_ERROR_INFO;

typedef struct __STRUCT_PCR_RUN_CTRL_INFO
{
    _PCR_RUN_ERROR_INFO ErrorInfo;
    _PCR_RUN_SIZE_INFO SizeInfo;
    _PCR_CTRL_BITS State;
    quint16 Temp[5];
    ///< 与结构体内存无关快捷函数------------------------------------------
    __STRUCT_PCR_RUN_CTRL_INFO()    { this->_memset(); }
    void _memset()              { memset( (void*)(this), '\0', this->_size() ); }
    uint _size()                { return sizeof( __STRUCT_PCR_RUN_CTRL_INFO ); }
}_PCR_RUN_CTRL_INFO;

typedef struct __STRUCT_PCR_INSTRUMENT_INFO
{
    char    InstruTypeName[32]; //仪器类型名称
    char    InstruSN[32];       //仪器序列号
    char    InstruName[32];     //仪器名称
    _DEVICE_VERSION ArmVer;     //主控版本
    _DEVICE_VERSION OptVer;     //光学版本
    _DEVICE_VERSION TherVer;    //热学版本
    _DEVICE_VERSION DrvVer;     //驱动板版本
    ///< 与结构体内存无关快捷函数------------------------------------------
    __STRUCT_PCR_INSTRUMENT_INFO()  { this->_memset(); }
    void _memset()              { memset( (void*)(this), '\0', this->_size() ); }
    uint _size()                { return sizeof( __STRUCT_PCR_INSTRUMENT_INFO ); }
}_PCR_INSTRUMENT_INFO;

#endif  //PCR_INFO_H
