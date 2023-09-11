#ifndef GUSBSELECTDIALOG_H
#define GUSBSELECTDIALOG_H

#include <QDialog>
#include <QMap>

class QPushButton;
class GUsbSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GUsbSelectDialog(const QMap< QString, QString > &usbDevices, QWidget *parent = 0);
    ~GUsbSelectDialog();

    QString currentDev() const{
        return m_currentDev;
    }

private slots:
    void usbButtonClicked();
    void cancelButtonClicked();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString m_currentDev;
    QMap< QPushButton*, QString > m_buttons;
};

#endif // GUSBSELECTDIALOG_H
