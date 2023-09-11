/*!
* \file gexperimentfile.cpp
* \brief ARM板软件中实验文件定义头文件
*
*实现了ARM板软件实验文件的定义、打开、保存等功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-11-28 10:09
*
*/

//-----------------------------------------------------------------------------
//include declare
//-----------------------------------------------------------------------------
#include "gexperimentfile.h"
#include "gdatapool.h"
#include "quazip.h"
#include "quazipfile.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>
#include <QApplication>
//-----------------------------------------------------------------------------
//class declare
//-----------------------------------------------------------------------------
/*!
* \class GExperimentFile
* \brief ARM板实验文件类
*
* 实验文件的定义、打开、保存等功能
*/

/*!
* \brief 类GExperimentFile的构造函数
* \param parent = NULL
* \return 无
*/
GExperimentFile::GExperimentFile(QObject *parent) :
    QObject(parent)
{
    is_compressed_file = 0;
}

/*!
* \brief 类GExperimentFile的构造函数
* \param fileName 实验文件名
* \param parent = NULL
* \return 无
*/
GExperimentFile::GExperimentFile(const QString &fileName, QObject *parent) :
    QObject(parent)
{
    is_compressed_file = 0;
    setFile(fileName);
}

/*!
* \brief 类GExperimentFile的析构函数
* \param 无
* \return 无
*/
GExperimentFile::~GExperimentFile()
{
    clear();
}

bool GExperimentFile::isValid() const
{
    return !m_expFileInfo.fileName.isEmpty() && QFile(m_expFileInfo.fileName).exists();
}

/*!
* \brief 类GExperimentFile的公共函数，实现实验文件的定义和扫描
* \param fileName 实验文件名
* \return 无
*/
void GExperimentFile::setFile(const QString &fileName)
{    
    if(m_expFileInfo.fileName != fileName.trimmed()){
        m_expFileInfo.fileName = fileName.trimmed();
    }

    this->is_compressed_file = QFileInfo(m_expFileInfo.fileName).suffix() != QStringLiteral("tlpp");    
}

/*!
* \brief 类GExperimentFile的公共函数，实现文件配置的复制,复制完成后生成tlpp文件
* \param fileConfig 需要拷贝设置的文件
* \return 无
*/

void GExperimentFile::setConfig(const GExperimentFile &fileConfig)
{
    qDebug() << Q_FUNC_INFO << fileConfig.m_expRunMethod.count();

    //复制运行信息
    this->m_expRunInfo = fileConfig.m_expRunInfo;

    //复制运行方法
    for(int i=0; i<fileConfig.m_expRunMethod.count(); i++){
        if(fileConfig.m_expRunMethod.at(i) == NULL){
            this->m_expRunMethod << NULL;
        }else{
            _STAGE_INFO *stage = new _STAGE_INFO;
            memcpy((void*)stage, (const void*)(fileConfig.m_expRunMethod.at(i)), sizeof (_STAGE_INFO));
            this->m_expRunMethod << stage;
        }
    }

    this->is_compressed_file = 0;
}

void GExperimentFile::clear()
{
    m_expFileInfo.fileName.clear();
    clearAllExceptFileName();
}

int GExperimentFile::open(double *maxSpeed, bool not_del_dat)
{
    qDebug() << Q_FUNC_INFO << m_expFileInfo.fileName << (*maxSpeed);

    //文件名为空,退出
    if(m_expFileInfo.fileName.isEmpty()) return ERROR_FILENAME_EMPTY;

    QFileInfo fileInfo(m_expFileInfo.fileName);
    //文件是否存在(包括实验前tlpp和实验后tlpd和tlpe)
    QString file1_ = QDir::toNativeSeparators(fileInfo.path()+ QDir::separator() + fileInfo.completeBaseName() + QStringLiteral(".tlpp"));
    QString file2_ = QDir::toNativeSeparators(fileInfo.path()+ QDir::separator() + fileInfo.completeBaseName() + QStringLiteral(".tlpd"));
    QString file3_ = QDir::toNativeSeparators(fileInfo.path()+ QDir::separator() + fileInfo.completeBaseName() + QStringLiteral(".tlpe"));
    if(!QFile::exists(file1_) && !QFile::exists(file2_) && !QFile::exists(file3_)) return ERROR_FILE_NOT_EXIST;

    qDebug() << Q_FUNC_INFO << "openning" << not_del_dat;

    clearAllExceptFileName(not_del_dat);

    if(fileInfo.suffix()==QStringLiteral("tlpd") || fileInfo.suffix()==QStringLiteral("tlpe")){
        is_compressed_file = (fileInfo.suffix()==QStringLiteral("tlpd")) ? 1 : 2;

        //得到解压路径
        QString out_file_path = extract(m_expFileInfo.fileName);

        //找到运行控制文件, 如果文件存在则打开，如果不存在, 则添加
        if(QFile(QDir::toNativeSeparators(out_file_path + "run")).exists()){
            int ret = openRunCtrlFile(out_file_path + "run", maxSpeed);
            if(ret != 0){
                qDebug() << Q_FUNC_INFO << "open runctrl file error:" << ret;
                clearAllExceptFileName();
                return ret;
            }
        }else{
            saveRunCtrlFile(out_file_path + "run", fileInfo.completeBaseName());
        }

        //找到数据文件, 如果文件存在,保存为tlpd, 如果不存在,保存为tlpp
        QString datFile = QDir::toNativeSeparators(out_file_path + "_data");

        if(QFile(datFile).exists()){
            int ret = openExpDataFile(out_file_path + "_data");
            if(ret != 0){
                qDebug() << Q_FUNC_INFO << "open expdata file error:" << ret;
                clearAllExceptFileName();
                return ret;
            }
        }
    }else{
        int ret = openRunCtrlFile(m_expFileInfo.fileName, maxSpeed);
        if(ret != 0){
            qDebug() << Q_FUNC_INFO << "open runctrl file error:" << ret;
            clearAllExceptFileName();
            return ret;
        }
    }

    return 0;
}

/*!
* \brief 类GExperimentFile的公共函数，实现实验文件的保存
* \param 无
* \return 无
*/
int GExperimentFile::save()
{    
    QFileInfo fileInfo(m_expFileInfo.fileName);

    if(fileInfo.suffix()==QStringLiteral("tlpd") || fileInfo.suffix()==QStringLiteral("tlpe")){
        is_compressed_file = (fileInfo.suffix()==QStringLiteral("tlpd")) ? 1 : 2;

        //找到当前解压文件的文件夹位置
        QString out_file_path = getUnzipPath(m_expFileInfo.fileName);

        //如果没有文件夹则退出
        QDir fileDir(out_file_path);
        if(!fileDir.exists()){
            if(fileInfo.exists()){
                //解压tlpd文件
                extract(m_expFileInfo.fileName);
            }else{
                return -1;
            }
        }

        //删除存在的运行控制文件, 添加新文件
        QString runctrlFile = QDir::toNativeSeparators(out_file_path + "run");
        if(QFile(runctrlFile).exists()){
            QFile(runctrlFile).remove();
        }
        saveRunCtrlFile(out_file_path + "run", fileInfo.completeBaseName());

        //压缩的文件名
        QString zip_file = QDir::toNativeSeparators(fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName());
        zip_file += (is_compressed_file==1) ? QStringLiteral(".tlpd") : QStringLiteral(".tlpe");
        qDebug() << Q_FUNC_INFO << "zip_file:" << zip_file;

        QStringList wrapper ;
        foreach(QString fn, fileDir.entryList(QDir::Files)){
            wrapper.append(out_file_path+fn);
        }

        QuaZip zip(zip_file);
        zip.setFileNameCodec("UTF-8");

        if(!zip.open(QuaZip::mdCreate)){
            qDebug() << "save zip error ....1";
            return 1;
        }

        QuaZipFile outFile(&zip);
        for(int i = 0 ; i < wrapper.size() ; i++){
            QFileInfo file(wrapper.at(i));

            if(file.exists()){

                QFile inFile ;
                QFile inFileTemp ;

                inFileTemp.setFileName(file.fileName());
                inFile.setFileName(file.filePath());

                if(!inFile.open(QIODevice::ReadOnly)){
                    qDebug() << "save zip error ....2";
                    qDebug() << inFile.errorString().toLocal8Bit().constData();
                    return 2;
                }

                if(!outFile.open(QIODevice::WriteOnly , QuaZipNewInfo(inFileTemp.fileName() , inFile.fileName()), GDataPool::adminPassword.data())){
                    qDebug() << "save zip error ...3";
                    return 3;
                }

                outFile.write(inFile.readAll());

                if(outFile.getZipError() != UNZ_OK){
                    qDebug() << "save zip error ...4";
                    return 4;
                }

                outFile.close();

                if(outFile.getZipError() != UNZ_OK){
                    qDebug() << "save zip error ....5";
                    return 5;
                }

                inFile.close();
            }
        }

        zip.close();

        if(zip.getZipError()){
            qDebug() << "save zip error ...6";
            return 6;
        }
    }else{
        qDebug() << Q_FUNC_INFO << "save tlpp:" << m_expFileInfo.fileName;
        saveRunCtrlFile(m_expFileInfo.fileName, fileInfo.baseName());
    }

    emit fileSizeChanged();

    return 0;
}

/*!
* \brief 类GExperimentFile的私有函数，实现实验文件的信息初始化设置
* \param 无
* \return 无
*/
void GExperimentFile::clearAllExceptFileName(bool not_del_dat)
{
    is_compressed_file = 0;
    m_expFileInfo.clear();
    m_expRunInfo.clear();
    clearRunMethod();

    if(!not_del_dat){
        clearWellData();
    }
}

/*!
* \brief 类GExperimentFile的私有函数，清空运行方法设置
* \param 无
* \return 无
*/
void GExperimentFile::clearRunMethod()
{
    qDeleteAll(m_expRunMethod);
    m_expRunMethod.clear();
}

/*!
* \brief 类GExperimentFile的私有函数，清空荧光数据设置
* \param 无
* \return 无
*/
void GExperimentFile::clearWellData()
{
    qDeleteAll(mn_wellAmpData);
    mn_wellAmpData.clear();

    qDeleteAll(mn_wellMeltData);
    mn_wellMeltData.clear();
}

void GExperimentFile::saveRunCtrlFile(const QString &fileName, const QString &baseName)
{
    QString txt;

    QSettings fileSetting(fileName, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");

    //文件信息
    fileSetting.beginGroup("FileInfo");

    fileSetting.setValue("FileName", baseName);

    if(m_expFileInfo.createTime.isEmpty()){
        txt = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
        m_expFileInfo.createTime = txt;
    }else
        txt = m_expFileInfo.createTime;
    fileSetting.setValue("CreateDate", txt);

    m_expFileInfo.modifyTime = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
    fileSetting.setValue("ModifyDate", m_expFileInfo.modifyTime);
    fileSetting.endGroup();

    //运行方法
    fileSetting.beginGroup("RunMethod");

    fileSetting.setValue("ReacVolume", m_expRunInfo.reactionValume);
    fileSetting.setValue("Lid", m_expRunInfo.lidStatus);
    fileSetting.setValue("Lid_T", m_expRunInfo.lidValue);

    fileSetting.beginWriteArray("Stage");
    for(int i=0; i<m_expRunMethod.count(); i++){
        if(m_expRunMethod.at(i) == NULL) continue;
        qDebug() << tr("~~~~~~~~~~~~~~~~~~~~~ stage[%1]' step count").arg(i) << m_expRunMethod.at(i)->SubNum;
        fileSetting.setArrayIndex(i);
        fileSetting.setValue("Name", QString::fromUtf8(m_expRunMethod.at(i)->Name));
        fileSetting.setValue("Property", m_expRunMethod.at(i)->Property);
        fileSetting.setValue("Cycles", m_expRunMethod.at(i)->Cycles);
        fileSetting.setValue("SubNum", m_expRunMethod.at(i)->SubNum);

        for(int j=0; j < m_expRunMethod.at(i)->SubNum; ++j){
            fileSetting.setValue(QString("Temp%1").arg(i), m_expRunMethod.at(i)->Temp[j]);
            fileSetting.setValue(QString("Ramp%1").arg(i), m_expRunMethod.at(i)->Ramp[j]);
            fileSetting.setValue(QString("Time%1").arg(i), m_expRunMethod.at(i)->Time[j]);
            fileSetting.setValue(QString("SubProperty%1").arg(i), m_expRunMethod.at(i)->SubProperty[j]);
            fileSetting.setValue(QString("ReadInterval%1").arg(i), m_expRunMethod.at(i)->ReadInterval[j]);
            fileSetting.setValue(QString("ReadFluor%1").arg(i), m_expRunMethod.at(i)->ReadFluor[j]);
            fileSetting.setValue(QString("ReadMode%1").arg(i), m_expRunMethod.at(i)->ReadMode[j]);
            fileSetting.setValue(QString("TarValue%1").arg(i), m_expRunMethod.at(i)->TarValue[j]);
            fileSetting.setValue(QString("BeginCycle%1").arg(i), m_expRunMethod.at(i)->BeginCycle[j]);
            fileSetting.setValue(QString("Delta%1").arg(i), m_expRunMethod.at(i)->Delta[j]);
        }
    }
    fileSetting.endArray();
    fileSetting.endGroup();
    qDebug() << Q_FUNC_INFO << "save end";
}

//maxSpeed 温控速率最大值
int GExperimentFile::openRunCtrlFile(const QString &fileName, double *maxSpeed)
{
    QSettings fileSetting(fileName, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");

    if(!fileSetting.childGroups().contains("FileInfo")) return ERROR_NO_FILE_INFO;

    //文件信息
    fileSetting.beginGroup("FileInfo");

    QString txt = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss");
    m_expFileInfo.createTime = fileSetting.value("CreateDate", txt).toString();
    m_expFileInfo.modifyTime = fileSetting.value("ModifyDate", txt).toString();

    fileSetting.endGroup();

    //运行方法
    fileSetting.beginGroup("RunMethod");
    m_expRunInfo.reactionValume = fileSetting.value("ReacVolume", 25).toInt();
    m_expRunInfo.lidStatus = fileSetting.value("Lid", 1).toBool();
    m_expRunInfo.lidValue = fileSetting.value("Lid_T", 105.0).toDouble();

    int stageSize = fileSetting.beginReadArray("Stage");
    for(int i=0; i<stageSize; i++){
        fileSetting.setArrayIndex(i);
        qDebug() << tr("---------------Stage %1----------").arg(i);
        _STAGE_INFO *runStage = new _STAGE_INFO;
        if(runStage){
            QString txt = QString::fromUtf8(fileSetting.value("Name").toByteArray());
            memset((void*)runStage->Name, '\0', 64);
            memcpy((void*)runStage->Name, (const void*)(txt.toUtf8().data()), txt.toUtf8().count());
            runStage->Property = fileSetting.value("Property",0).toInt();
            runStage->Cycles = fileSetting.value("Cycles",1).toInt();
            runStage->SubNum = fileSetting.value("SubNum",0).toInt();

            for(int j=0; j < runStage->SubNum; ++j){
                runStage->Temp[j] = fileSetting.value(QString("Temp%1").arg(i)).toFloat();
                runStage->Ramp[j] = fileSetting.value(QString("Ramp%1").arg(i)).toInt();
                runStage->Time[j] = fileSetting.value(QString("Time%1").arg(i)).toInt();
                runStage->SubProperty[j] = fileSetting.value(QString("SubProperty%1").arg(i)).toInt();
                runStage->ReadInterval[j] = fileSetting.value(QString("ReadInterval%1").arg(i)).toFloat();
                runStage->ReadFluor[j] = fileSetting.value(QString("ReadFluor%1").arg(i)).toInt();
                runStage->ReadMode[j] = fileSetting.value(QString("ReadMode%1").arg(i)).toInt();
                runStage->TarValue[j] = fileSetting.value(QString("TarValue%1").arg(i)).toFloat();
                runStage->BeginCycle[j] = fileSetting.value(QString("BeginCycle%1").arg(i)).toInt();
                runStage->Delta[j] = fileSetting.value(QString("Delta%1").arg(i)).toInt();
            }
        }

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);


        fileSetting.endArray();
        m_expRunMethod << runStage;
    }
    fileSetting.endArray();
    fileSetting.endGroup();

    return 0;
}

int GExperimentFile::openExpDataFile(const QString &fileName)
{
    qDebug() << "open experiment data file";
    QSettings fileSetting(fileName, QSettings::IniFormat);
    fileSetting.setIniCodec("UTF-8");

    if(!fileSetting.childGroups().contains("FileInfo")) return ERROR_NO_FILE_INFO;

    //文件信息
    fileSetting.beginGroup("FileInfo");
    int amp_stage_size_ = fileSetting.value("AmpSize").toDouble();
    int melt_stage_size_ = fileSetting.value("MeltSize").toDouble();

    qDebug() << Q_FUNC_INFO << "FileInfo:" << fileSetting.value("FileVer").toInt() << amp_stage_size_ << melt_stage_size_;
    fileSetting.endGroup();


    //扩增数据
    do{
        QString amp_section_ = (amp_stage_size_ <= 1) ? QStringLiteral("AmpData") : QString("AmpData%1").arg(amp_stage_size_-1);
        if(fileSetting.childGroups().contains(amp_section_, Qt::CaseInsensitive)){
            bool is_err_ = false;
            fileSetting.beginGroup(amp_section_);
            int size_ = fileSetting.beginReadArray("Cycle");
            if(size_ > 0){
                for(int i=0; i<size_; i++){
                    fileSetting.setArrayIndex(i);

                    QByteArray dat = GHelper::hexStrToByteArray(fileSetting.value("Value").toString());
                    if(dat.size() != sizeof(_FLUOR_DATA)){
                        is_err_ = true;
                        break;
                    }

                    _FLUOR_SCAN_INFO *scan_info_ = new _FLUOR_SCAN_INFO;
                    scan_info_->_memset();
                    scan_info_->SaveFluorFileNo = (-1);
                    scan_info_->No = (-1);
                    memcpy((void*)scan_info_->FluorValue.Fluor, (const void*)dat.data(), sizeof(_FLUOR_DATA));
                    mn_wellAmpData.append(scan_info_);

                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }else{
                is_err_ = true;
            }

            fileSetting.endArray();
            fileSetting.endGroup();

            if(is_err_){
                amp_stage_size_--;
            }else{
                if(amp_stage_size_<1) amp_stage_size_ = 1;
                break;
            }
        }else{
            amp_stage_size_--;
        }

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    }while(amp_stage_size_ > 0);

    //熔解数据
    do{
        QString melt_section_ = (melt_stage_size_ <= 1) ? QStringLiteral("MeltData") : QString("MeltData%1").arg(melt_stage_size_-1);
        if(fileSetting.childGroups().contains(melt_section_, Qt::CaseInsensitive)){
            bool is_err_ = false;
            fileSetting.beginGroup(melt_section_);
            int size_ = fileSetting.beginReadArray("Cycle");
            if(size_ > 0){
                for(int i=0; i<size_; i++){
                    fileSetting.setArrayIndex(i);

                    QByteArray dat = GHelper::hexStrToByteArray(fileSetting.value("Value").toString());
                    if(dat.size() != sizeof(_FLUOR_DATA)){
                        is_err_ = true;
                        break;
                    }

                    _FLUOR_SCAN_INFO *scan_info_ = new _FLUOR_SCAN_INFO;
                    scan_info_->_memset();
                    scan_info_->Type = 1;
                    scan_info_->SaveFluorFileNo = (-1);
                    scan_info_->No = (-1);
                    memcpy((void*)scan_info_->FluorValue.Fluor, (const void*)dat.data(), sizeof(_FLUOR_DATA));
                    mn_wellMeltData.append(scan_info_);

                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }else{
                is_err_ = true;
            }

            fileSetting.endArray();
            fileSetting.endGroup();

            if(is_err_){
                melt_stage_size_--;
            }else{
                if(melt_stage_size_<1) melt_stage_size_ = 1;
                break;
            }
        }else{
            melt_stage_size_--;
        }

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }while(melt_stage_size_ > 0);

    return 0;
}

QString GExperimentFile::getUnzipPath(const QString &fileName) const
{
    QFileInfo fileInfo(fileName);
    return QDir::toNativeSeparators(qApp->applicationDirPath() + QDir::separator() + TMP_DIRCTORY + QDir::separator() + fileInfo.fileName() + QDir::separator());
}

QString GExperimentFile::extract(const QString &zip_file)
{
    QFileInfo fileInfo(zip_file);
    QString out_file_path = getUnzipPath(zip_file);

    QDir dir(out_file_path);
    if(!dir.exists()) dir.mkpath(out_file_path);

    QuaZip archive(zip_file);
    archive.setFileNameCodec("UTF-8");

    if(!archive.open(QuaZip::mdUnzip)) return QString();

    for(bool f=archive.goToFirstFile(); f; f=archive.goToNextFile()){
        QString filePath = archive.getCurrentFileName();
        QuaZipFile zFile(archive.getZipName(), filePath);
        zFile.open(QIODevice::ReadOnly, NULL, NULL, false, GDataPool::adminPassword.data());
        QByteArray ba = zFile.readAll();
        zFile.close();

        if(filePath.endsWith("/")){
            dir.mkpath(filePath);
        }else{
            QFile dstFile(out_file_path + filePath);
            if(!dstFile.open(QIODevice::WriteOnly))
                return QString();
            dstFile.write(ba);
            dstFile.close();
        }
    }

    return out_file_path;
}
