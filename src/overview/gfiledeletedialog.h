#ifndef GFILEDELETEDIALOG_H
#define GFILEDELETEDIALOG_H

#include <QDialog>

namespace Ui {
class GFileDeleteDialog;
}

class GDataPool;
class QStandardItemModel;
class GFileCopyDelegate;
class GFileDeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GFileDeleteDialog(GDataPool *dataPool, const QString &currentFile, const QString &loaclPath, QWidget *parent = 0);
    ~GFileDeleteDialog();

    QStringList selectedFiles() const;
private slots:
    void on_checkBoxSelectAll_toggled(bool checked);
    void on_buttonCancel_clicked();
    void on_buttonOk_clicked();

    void g_itemClicked();
protected:
    void paintEvent(QPaintEvent *event);
private:
    void updateButton();
private:
    Ui::GFileDeleteDialog *ui;

    GDataPool *m_pool;

    QStandardItemModel *modelFile;
    GFileCopyDelegate *fileDelegate;
};

#endif // GFILEDELETEDIALOG_H
