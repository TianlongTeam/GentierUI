#ifndef GINPUTDIALOG_H
#define GINPUTDIALOG_H

#include <QDialog>

class QLabel;
class QToolButton;
class QLineEdit;
class WidgetKeyboard;
class GInputDialog : public QDialog
{
    Q_OBJECT
public:
    enum InputType{
        IT_ExpName,
        IT_Name,
        IT_Folder
    };

    explicit GInputDialog(const QString &suffix = QString(), const QString &path = QString(), const QString &text = QString(), int inputType = IT_ExpName, QWidget *parent = 0);
    ~GInputDialog();

    QString input() const;

    void setMode(InputType type = IT_ExpName);
    void setTitle(const QString &title);

    void setMaxCharLength(int maxchar);
signals:
    void keyPressed();
protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void pushButton_clicked(bool isOk);
    void g_keyPressed();
private:
    QString getString(QString s, int l);
private:
    QLabel      *label;
    QLineEdit   *lineEdit;
    WidgetKeyboard *keyboard;

    int m_inputType;    ///< 输入类型

    QString m_defaultSuffix;        ///< 缺省文件扩展名
    QString m_path;                 ///< 文件的路径
    QString m_fileName;             ///< 设置的文件名
    QString m_folderName;
    bool    m_textSelected;         ///< 文件名输入框中是否有字符被选择
    int     m_maxCharLength;        ///< 最长输入字节数
};

#endif // GINPUTDIALOG_H
