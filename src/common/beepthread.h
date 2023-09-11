#ifndef BEEPTHREAD_H
#define BEEPTHREAD_H

#include <QThread>
#include <QColor>
//#include <common_define.h>

class BeepThread : public QThread
{
    Q_OBJECT
public:
    enum Type{
        GeneralSound = 0,
        AlarmSound   = 1,
        KeySound     = 2
    };
    BeepThread(QObject *parent = 0);
    ~BeepThread();

signals:

public slots:
    void on_StartBeep(char _beepType,const qint8 _num = 1);

protected:
    void run();
    void stop();

private:
    qint8 num;
    int sleep_time;
    int freq;
};

#endif // BEEPTHREAD_H
