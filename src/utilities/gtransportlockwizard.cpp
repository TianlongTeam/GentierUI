#include "gtransportlockwizard.h"

#include "gdatapool.h"
#include "gwizardpage.h"

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QTimerEvent>
#include <QDebug>

GTransportLockWizard::GTransportLockWizard(GDataPool *pool, QWidget * parent, Qt::WindowFlags flags) \
    :   QWizard(parent, flags)
    ,   m_pool(pool)
    ,   m_btnSendCmd(NULL)
    ,   m_timeId(0)
{
    addPage( createIntroPage() );
    addPage( createSoftwareLockPage() );
    addPage( createHardwareLockPage() );
    addPage( createFinishPage() );

    connect(this, SIGNAL(currentIdChanged(int)), SLOT(g_currentIdChanged(int)));
    connect(m_pool, SIGNAL(sig_dealReadData(quint8,quint16,QByteArray)), this, SLOT(g_dealReadData(quint8,quint16,QByteArray)));
}

GTransportLockWizard::~GTransportLockWizard()
{
    if(m_timeId){
        killTimer(m_timeId);
        m_timeId = 0;
    }

    m_pool = NULL;
    m_btnSendCmd = NULL;
}

void GTransportLockWizard::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    if(id == m_timeId){
        if(m_pool->isInitialzied())
            m_pool->WriteData(1, 1);
    }

    QWizard::timerEvent(e);
}

QWizardPage* GTransportLockWizard::createIntroPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Introduction"));

    QLabel *label = new QLabel(tr("This wizard will help you to perform transport lock operation."
                                " After the operation, the instrument will be shut down."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GTransportLockWizard::createSoftwareLockPage()
{
    GWizardPage *page = new GWizardPage;
    page->setTitle(tr("Mechanical component moving reset"));

    m_btnSendCmd = new QPushButton(tr("Lock"));
    m_btnSendCmd->setMaximumSize(120,40);
    m_btnSendCmd->setMinimumSize(120,40);
    connect(m_btnSendCmd, SIGNAL(clicked()), this, SLOT(g_btnSendCmd_clicked()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_btnSendCmd);
    layout->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Preferred));
    page->setLayout(layout);

    return page;
}

QWizardPage* GTransportLockWizard::createHardwareLockPage()
{
    GWizardPage *page = new GWizardPage;
    page->setTitle(tr("Tighten the transport lock screw"));

    QLabel *label = new QLabel(tr("Wait for the signal of operator completion ..."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GTransportLockWizard::createFinishPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Lock operation completed"));

    QLabel *label = new QLabel(tr("The instrument will be shut down."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

void GTransportLockWizard::g_btnSendCmd_clicked()
{
    m_pool->screen_sound();
    if(m_btnSendCmd) m_btnSendCmd->setEnabled(false);
    //发送锁定命令
    if(m_pool->isInitialzied()){
        QByteArray dat;
        dat.resize(sizeof(quint16));
        quint16 currentCmd = 0;
        memcpy((void*)dat.data(), (const void*)&currentCmd, sizeof(quint16));
        m_pool->WriteData(3, 55, dat);
    }

    //按时查询状态
    m_timeId = startTimer(1000);
}

void GTransportLockWizard::g_currentIdChanged(int id)
{
    if(id != 2) return;

    //按时查询状态
    m_timeId = startTimer(1000);
}

void GTransportLockWizard::g_dealReadData(quint8 type1, quint16 type2, const QByteArray &dat)
{
    if(type1 == 1 && type2 == 1){
        int infoSize = sizeof(_PCR_RUN_CTRL_INFO);
        _PCR_RUN_CTRL_INFO pcrInfo;
        memset((void*)&pcrInfo, '\0', infoSize);
        memcpy((void*)&pcrInfo, (const void*)(dat.data()), infoSize);

        bool isOk = false;      ///< 等待操作已完成
        switch(this->currentId()){
        case 1:
            if(pcrInfo.State.DrvLockSoft){
                GWizardPage *page = (GWizardPage*)(this->page(1));
                if(page){
                    page->setPause(true);
                    if(m_btnSendCmd) m_btnSendCmd->setEnabled(true);
                    this->back();
                    this->next();
                }
                isOk = true;
            }
            break;
        case 2:
            if(pcrInfo.State.DrvLockMechanical){
                GWizardPage *page = (GWizardPage*)(this->page(2));
                if(page){
                    page->setPause(true);
                    this->back();
                    this->next();
                }
                isOk = true;
            }
            break;
        default:isOk = true;break;
        }

        if(isOk){
            killTimer(m_timeId);
            m_timeId = 0;
        }

    }
}
