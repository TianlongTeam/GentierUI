#ifndef GTRANSPORTLOCKWIZARD_H
#define GTRANSPORTLOCKWIZARD_H

#include <QWizard>
#include <QWizardPage>

class GDataPool;
class QPushButton;
class GTransportLockWizard : public QWizard
{
    Q_OBJECT
public:
    explicit GTransportLockWizard(GDataPool *pool, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~GTransportLockWizard();

protected:
    void timerEvent(QTimerEvent *e);

private:
    QWizardPage *createIntroPage();
    QWizardPage *createSoftwareLockPage();
    QWizardPage *createHardwareLockPage();
    QWizardPage *createFinishPage();
private slots:
    void g_btnSendCmd_clicked();
    void g_currentIdChanged(int id);

    void g_dealReadData(quint8 type1, quint16 type2, const QByteArray &dat);
private:
    GDataPool *m_pool; ///< 私有自检报告文件指针
    QPushButton* m_btnSendCmd;

    int m_timeId;
};

#endif // GTRANSPORTLOCKWIZARD_H
