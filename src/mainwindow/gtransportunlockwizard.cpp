#include "gtransportunlockwizard.h"

#include "gdatapool.h"
#include "gwizardpage.h"

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QTimerEvent>
#include <QDebug>

GTransportUnlockWizard::GTransportUnlockWizard(GDataPool *pool, QWidget * parent, Qt::WindowFlags flags) \
    :   QWizard(parent, flags)
    ,   m_pool(pool)
    ,   m_btnSendCmd(NULL)
    ,   m_timeId(0)
{
    setOptions(QWizard::NoBackButtonOnStartPage|QWizard::NoBackButtonOnLastPage|QWizard::NoCancelButton);

    addPage( createHardwareLockPage() );
    addPage( createSoftwareLockPage() );
    addPage( createFinishPage() );

    connect(this, SIGNAL(currentIdChanged(int)), SLOT(g_currentIdChanged(int)));
    connect(m_pool, SIGNAL(sig_dealReadData(quint8,quint16,QByteArray)), this, SLOT(g_dealReadData(quint8,quint16,QByteArray)));
}

GTransportUnlockWizard::~GTransportUnlockWizard()
{
    if(m_timeId){
        killTimer(m_timeId);
        m_timeId = 0;
    }
    m_pool = NULL;
    m_btnSendCmd = NULL;
}

void GTransportUnlockWizard::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    if(id == m_timeId){
        if(m_pool->isInitialzied())
            m_pool->WriteData(1, 1);
    }

    QWizard::timerEvent(e);
}

QWizardPage* GTransportUnlockWizard::createHardwareLockPage()
{
    GWizardPage *page = new GWizardPage;
    page->setTitle(tr("Transportation locked"));

    QLabel *label = new QLabel(tr("Please unscrew the transport lock screw."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* GTransportUnlockWizard::createSoftwareLockPage()
{
    GWizardPage *page = new GWizardPage;
    page->setTitle(tr("Mechanical component moving reset"));

    m_btnSendCmd = new QPushButton(tr("Unlock"));
    m_btnSendCmd->setMaximumSize(120,40);
    m_btnSendCmd->setMinimumSize(120,40);
    connect(m_btnSendCmd, SIGNAL(clicked()), this, SLOT(g_btnSendCmd_clicked()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_btnSendCmd);
    layout->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Preferred));
    page->setLayout(layout);

    return page;
}

QWizardPage* GTransportUnlockWizard::createFinishPage()
{
    QWizardPage *page = new QWizardPage;
    page->setTitle(tr("Unlock operation completed"));

//    QLabel *label = new QLabel(tr("The instrument will be shut down."));
//    label->setWordWrap(true);

//    QVBoxLayout *layout = new QVBoxLayout;
//    layout->addWidget(label);
//    page->setLayout(layout);

    return page;
}

void GTransportUnlockWizard::g_btnSendCmd_clicked()
{
    if(m_btnSendCmd) m_btnSendCmd->setEnabled(false);
    //发送锁定命令
    if(m_pool->isInitialzied()){
        m_pool->WriteData(3, 8);
    }

    //按时查询状态
    m_timeId = startTimer(1000);
}

void GTransportUnlockWizard::g_currentIdChanged(int id)
{
    if(id != 0) return;

    //按时查询状态
    m_timeId = startTimer(1000);
}

void GTransportUnlockWizard::g_dealReadData(quint8 type1, quint16 type2, const QByteArray &dat)
{
    if(type1 == 1 && type2 == 1){
        int infoSize = sizeof(_PCR_RUN_CTRL_INFO);
        _PCR_RUN_CTRL_INFO pcrInfo;
        memset((void*)&pcrInfo, '\0', infoSize);
        memcpy((void*)&pcrInfo, (const void*)(dat.data()), infoSize);

        bool isOk = false;      ///< 等待操作已完成
        switch(this->currentId()){
        case 0:
            if(!pcrInfo.State.DrvLockMechanical){
                GWizardPage *page = (GWizardPage*)(this->page(0));
                if(page){
                    page->setPause(true);
                    this->next();
                    this->back();
                }
                isOk = true;
            }
            break;
        case 1:
            if(!pcrInfo.State.DrvLockSoft){
                GWizardPage *page = (GWizardPage*)(this->page(1));
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
