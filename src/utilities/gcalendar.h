#ifndef GCALENDAR_H
#define GCALENDAR_H

#include <QWidget>

namespace Ui {
class GCalendar;
}

class GCalendar : public QWidget
{
    Q_OBJECT

public:
    explicit GCalendar(QWidget *parent = 0);
    ~GCalendar();

    QDate currentDate() const;
    void setCurrentDate(const QDate &date);

signals:
    void dateChanged();

private slots:
    void on_buttonMonthLeft_clicked();

    void on_buttonMonthRight_clicked();

    void on_buttonYearLeft_clicked();

    void on_buttonYearRight_clicked();

private:
    Ui::GCalendar *ui;
};

#endif // GCALENDAR_H
