/*!
* \file ginputstringdialog.h
* \brief ARM板软件中字符串输入对话框
*
*实现了ARM板软件中字符串的输入功能
*
*\author Gzf
*\version V1.0.0
*\date 2014-12-08 11:42
*
*/

#ifndef GINPUTSTRINGDIALOG_H
#define GINPUTSTRINGDIALOG_H

#include <QDialog>
#include <QSignalMapper>

namespace Ui {
class GInputStringDialog;
}

class GInputStringDialog : public QDialog
{
    Q_OBJECT

public:
    enum InputType{
        IT_Default,
        IT_Name
    };

    explicit GInputStringDialog(const QString &suffix = QString(), const QString &path = QString(), const QString &text = QString(), QWidget *parent = 0);
    ~GInputStringDialog();

    QString input() const;

    void setMode(InputType type = IT_Default);

signals:
    void keyPressed();
private slots:
    void buttonClicked(QWidget *w);

    void on_toolButton_shift_clicked();

    void on_toolButton_backspace_clicked();

    void on_toolButton_blank_clicked();

    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
private:
    Ui::GInputStringDialog *ui;
    QString m_defaultSuffix;        ///< 缺省文件扩展名
    QString m_path;                 ///< 文件的路径
    QString m_fileName;             ///< 设置的文件名
    bool    m_textSelected;         ///< 文件名输入框中是否有字符被选择
    QSignalMapper signalMapper;    
};

#endif // GINPUTSTRINGDIALOG_H
