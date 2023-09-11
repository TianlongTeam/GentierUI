#ifndef __WIDGETKEYBOARD_H_
#define __WIDGETKEYBOARD_H_

#include "ui_widgetkeyboard.h"

class QSignalMapper;
class WidgetKeyboard : public QWidget, public Ui::WidgetKeyboard
{
    Q_OBJECT
public:
    WidgetKeyboard(QWidget *parent=0);
    ~WidgetKeyboard();

    void Set_Parent(QWidget *parent, QWidget *focus_parent);
    bool Set_ChEnable(bool _ch_enable); //if false, give a "", else give the right dbname.
    void clearCtrl();
    void setDatabase();
    void setLanguage(const int &lan);

    void setPeriodVisible(bool visible);
private:
    QWidget *m_pParent;
    bool isCaps;
    QSignalMapper *signalMapper;
    QSignalMapper *signalMapperHZ;
    QList<QToolButton*> allButtons;
    QList<QPushButton*> HZButtons;

    bool isEnglish;
    int hzoffset;
    int QueryHZ(const QString &str);
    void init_keyBoard_Language();

signals:
    void sig_key_press();
    void sig_press_over(bool isOk);
private slots:
    void on_pushButton_next_clicked();
    void on_pushButton_previous_clicked();
    void on_Switch_ZH_clicked(bool checked);
    void slt_btn_clicked(int btn);
    void slt_HZ_clicked(int btn);
    void on_btnCaps_toggled(bool checked);
    void changeTextCaps(bool isCaps);
    bool checkNotTextKey(int keyId);
};
#endif /*__WIDGETKEYBOARD_H_*/
