#include "glog.h"

#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>

GLog::GLog(const QString &fileName, QObject *parent) : \
    QObject(parent),
    d_fileName(fileName)
{
    QFileInfo fileInfo(fileName);
    QDir path(fileInfo.absolutePath());
    if(!path.exists()){
        path.mkpath(fileInfo.absolutePath());
    }
}

void GLog::gLog(const QString &text)
{
    QFile file(d_fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("GBK");
        stream << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss:zzz") + "  " + text + "\r\n";
        file.close();
    }
}

void GLog::gCsv(const QStringList &list, bool prev)
{
    QFile file(d_fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("GBK");
        QString tmp;
        if(prev)
            tmp = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss:zzz") + ",";
        if(!list.isEmpty()){
            for(int i=0; i<list.count(); i++){
                tmp += list.at(i);
                if(i < list.count()-1)
                    tmp += ",";
            }
        }

        if(!tmp.isEmpty()){
            tmp += "\r\n";
            stream << tmp;
        }
        file.close();
    }
}

void GLog::operator << (const QString &text)
{
    gLog(text);
}

void GLog::operator << (const QStringList &list)
{
    gCsv(list);
}
