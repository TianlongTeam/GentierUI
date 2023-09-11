#ifndef GFILECOPYDIALOG_H
#define GFILECOPYDIALOG_H

#include <QDialog>

namespace Ui {
class GFileCopyDialog;
}

class GDataPool;
class QStandardItemModel;
class GFileCopyDelegate;
class GFileCopyDialog : public QDialog
{
    Q_OBJECT

public:
    enum operatorType{
        ExportFile = 0,
        ImportFile
    };

    explicit GFileCopyDialog(GDataPool *dataPool, operatorType type, const QString &localPath, const QString &usbPath, QWidget *parent = 0);
    ~GFileCopyDialog();

#ifdef DEVICE_TYPE_TL13
    QStringList selectedDirs() const;
#endif
    QStringList selectedFiles() const;
private slots:
    void on_checkBoxSelectAll_toggled(bool checked);
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::GFileCopyDialog *ui;

    GDataPool *m_pool;
    QStandardItemModel *modelLocal, *modelUsb;
    GFileCopyDelegate *fileInDelegate, *fileOutDelegate;
    operatorType opType;
};

#endif // GFILECOPYDIALOG_H
