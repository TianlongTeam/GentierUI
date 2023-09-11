#include "beepthread.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>


#include <QDebug>

BeepThread::BeepThread(QObject *parent)
    :QThread(parent)
{
    num = 1;
    sleep_time = 500;
    freq = 1400;

    this->setProperty("ring", false);
}

BeepThread::~BeepThread()
{
    quit();
//    wait();
}

void BeepThread::stop()
{
    quit();
//    wait();
}

void BeepThread::on_StartBeep(char _beepType, const qint8 _num)
{
    if(isRunning()){
        quit();
        wait();
    }

    switch(_beepType)
    {
        case AlarmSound:   // 报警音
            sleep_time = 800;
            freq = 910;
            break;
        case KeySound:  //  按键音
            sleep_time = 80;
            //freq = 1400;
            freq = 1000;
            break;
        case GeneralSound:  //其他
            sleep_time = 100;
            freq = 1400;
            break;
        default:break;
    }
    num = _num;
    start();
}

void BeepThread::run()
{
    msleep(20);
    if(isRunning())
    {
        int beep_fd = open("/dev/pwm",0);
        if(beep_fd < 0)
        {
            qDebug("open Beep device error!");
        }
        else
        {

            //        for(int i=0;i<num;i++)
            {
                qDebug("beep--");
                ioctl(beep_fd,1,freq);
                msleep(sleep_time);
                //            qmsleep(sleep_time);
                ioctl(beep_fd,1,0);
                //::close(beep_fd);
            }
            ioctl(beep_fd,2);

            #ifdef  TL_AM335X

            #else
            ::close(beep_fd);
            #endif

            msleep(10);
            num = 1;
        }
    }
    quit();
    this->setProperty("ring", false);

}
