#ifndef GFLUOREDITOR_H
#define GFLUOREDITOR_H

#include <QWidget>

class QLabel;
class QListWidget;
class GFluorEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GFluorEditor(int min, int max, const QByteArray &current, QWidget *parent = 0);
    ~GFluorEditor();
    void setCurrentKey(const QByteArray &key);
    QByteArray currentKey() const {return m_current;}

    void showList();
    void hideList();

    void setRange(int x, int y, int width/*, int height*/);
    bool isPopup() const {return m_popup;}

signals:
    void keyPressed();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void stateChanged();

private:
    int m_min;                  ///< 该组最小荧光
    int m_max;                  ///< 该组最大荧光

    int m_x, m_y, m_width, m_height, m_margin, m_spacing;
    bool m_popup;               ///< popup

    QLabel  *m_label;           ///< 选择结果显示栏
    QListWidget *m_listWidget;  ///< 下拉框

    QByteArray m_current;       ///< 该组当前荧光
};

#endif // GFLUOREDITOR_H
