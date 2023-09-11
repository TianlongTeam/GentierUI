#include "gcalendar.h"
#include "ui_gcalendar.h"

GCalendar::GCalendar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GCalendar)
{
    ui->setupUi(this);

    ui->labelYear->setText(QString::number(ui->calendarWidget->yearShown()));
    ui->labelMonth->setText(QString::number(ui->calendarWidget->monthShown()));

    connect(ui->calendarWidget, SIGNAL(selectionChanged()), this, SIGNAL(dateChanged()));
}

GCalendar::~GCalendar()
{
    delete ui;
}

QDate GCalendar::currentDate() const
{
    return ui->calendarWidget->selectedDate();
}

void GCalendar::setCurrentDate(const QDate &date)
{
    ui->calendarWidget->setSelectedDate(date);
}

void GCalendar::on_buttonMonthLeft_clicked()
{
    ui->calendarWidget->showPreviousMonth();
    ui->labelMonth->setText(QString::number(ui->calendarWidget->monthShown()));
    ui->calendarWidget->setSelectedDate(QDate(ui->calendarWidget->yearShown(), ui->calendarWidget->monthShown(), ui->calendarWidget->selectedDate().day()));
}

void GCalendar::on_buttonMonthRight_clicked()
{
    ui->calendarWidget->showNextMonth();
    ui->labelMonth->setText(QString::number(ui->calendarWidget->monthShown()));
    ui->calendarWidget->setSelectedDate(QDate(ui->calendarWidget->yearShown(), ui->calendarWidget->monthShown(), ui->calendarWidget->selectedDate().day()));
}

void GCalendar::on_buttonYearLeft_clicked()
{
    ui->calendarWidget->showPreviousYear();
    ui->labelYear->setText(QString::number(ui->calendarWidget->yearShown()));
    ui->calendarWidget->setSelectedDate(QDate(ui->calendarWidget->yearShown(), ui->calendarWidget->monthShown(), ui->calendarWidget->selectedDate().day()));
}

void GCalendar::on_buttonYearRight_clicked()
{
    ui->calendarWidget->showNextYear();
    ui->labelYear->setText(QString::number(ui->calendarWidget->yearShown()));
    ui->calendarWidget->setSelectedDate(QDate(ui->calendarWidget->yearShown(), ui->calendarWidget->monthShown(), ui->calendarWidget->selectedDate().day()));
}
