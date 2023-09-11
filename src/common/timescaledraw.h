#ifndef TIMESCALEDRAW_H
#define TIMESCALEDRAW_H

#include <qwt_scale_draw.h>
#include <qwt_scale_div.h>

//-----------------------------------------------------------------------------
//child class declare in xbottom
//-----------------------------------------------------------------------------
class TimeScaleDraw : public QwtScaleDraw
{
public:
    TimeScaleDraw()
    {
    }
    virtual QwtText label(double v) const
    {
        int h,m;
        h = (int)v / 3600;
        m = ((int)v % 3600) / 60;
//        s = ((int)v % 3600) % 60;
        QString tmp = QObject::tr("%1:%2").arg(QString::number(h).rightJustified(2,'0',true))
                .arg(QString::number(m).rightJustified(2,'0',true));
//                .arg(QString::number(s).rightJustified(2,'0',true));

        return tmp;
    }
};

#endif // TIMESCALEDRAW_H
