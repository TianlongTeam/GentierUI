#ifndef GTIMEINPUTDIALOG_H
#define GTIMEINPUTDIALOG_H

#include <QDialog>
#include <QSignalMapper>

namespace Ui {
class GTimeInputDialog;
}

class GTimeInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GTimeInputDialog(const QString &title, int minute, int second, bool hasInfinite, QWidget *parent = 0);
    ~GTimeInputDialog();

    void setIntRange(int bottom, int top);

    QVariant value() const;

signals:
    void keyPressed();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
private slots:
    void on_buttonBackspace_clicked();
    void on_panelButton_Infinite_clicked();
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();

    void buttonClicked(const QString &text);

private:
    Ui::GTimeInputDialog *ui;
    int m_focusIndex;
    bool m_hasInfinite;

    int m_savedMin, m_savedSec;     //当设置为无穷大时,显示在秒的输入框中,保存的秒
    QSignalMapper signalMapper;
};

#endif // GTIMEINPUTDIALOG_H
