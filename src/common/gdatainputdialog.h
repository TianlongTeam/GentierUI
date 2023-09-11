#ifndef GDATAINPUTDIALOG_H
#define GDATAINPUTDIALOG_H

#include <QDialog>
#include <QSignalMapper>

namespace Ui {
class GDataInputDialog;
}

class GDataInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GDataInputDialog(const QString &title, const QString &value, QWidget *parent = 0);
    ~GDataInputDialog();

    void setIntRange(int bottom, int top);
    void setDoubleRange(double bottom, double top, int decimals);

    void setPwdMode();

    QVariant value() const;

signals:
    void keyPressed();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_buttonBackspace_clicked();
    void on_panelButton_line_clicked();
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();

    void buttonClicked(const QString &text);

private:
    Ui::GDataInputDialog *ui;
    int m_type;         ///< int or double, default is int
    QSignalMapper signalMapper;
};

#endif // GDATAINPUTDIALOG_H
