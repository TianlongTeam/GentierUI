#ifndef GVERSIONDETAIL_H
#define GVERSIONDETAIL_H

#include <QDialog>

class QPushButton;
class QLabel;
class GVersionDetail : public QDialog
{
    Q_OBJECT
public:
    GVersionDetail(const QStringList &versions, QWidget *parent = 0);
    ~GVersionDetail();
signals:
    void keyPressed();
protected:
    void paintEvent(QPaintEvent *event);
private:
    QPushButton *m_buttonClose;
    QList<QLabel*> m_labelTitles;
    QList<QLabel*> m_labelUnits;
};

#endif // GVERSIONDETAIL_H
