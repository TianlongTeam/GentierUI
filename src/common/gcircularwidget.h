#ifndef GCIRCULARWIDGET_H
#define GCIRCULARWIDGET_H

#include <QLabel>
#include "QProgressIndicator.h"

class GCircularWidget : public QWidget
{
public:
    explicit GCircularWidget(QSize desktopSize, QWidget *parent = 0);

    void showProcess(const QString &text);
    void hideProcess();

    bool isShowing() {return m_processIndicator.isAnimated();}

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_desktopWidth, m_desktopHeight;

    QProgressIndicator m_processIndicator;
    QLabel m_label;
};

#endif // GCIRCULARWIDGET_H
