#ifndef GLOG_H
#define GLOG_H

#include <QObject>

class GLog : public QObject
{
    Q_OBJECT
public:
    GLog(const QString &fileName, QObject *parent = 0);

    void operator << (const QString &text);
    void operator << (const QStringList &list);
public slots:
    void gLog(const QString &text);
    void gCsv(const QStringList &list, bool prev = true);
private:
    QString d_fileName;
};

#endif // GLOG_H
