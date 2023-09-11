#ifndef GGRADIENTDETAIL_H
#define GGRADIENTDETAIL_H

#include <QDialog>

class QPushButton;
class QLabel;
class GGradientDetail : public QDialog
{
    Q_OBJECT
public:
    explicit GGradientDetail(double center, double offset, QWidget * parent = 0);
    ~GGradientDetail();
signals:
    void keyPressed();

protected:
    void paintEvent(QPaintEvent *event);
private:
    QPushButton *m_buttonClose;
    QList<QLabel*> m_labelTitles;
    QList<QLabel*> m_labelUnits;
};

#endif // GGRADIENTDETAIL_H
