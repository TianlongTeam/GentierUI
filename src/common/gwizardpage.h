#ifndef GWIZARDPAGE_H
#define GWIZARDPAGE_H

#include <QWizard>
#include <QWizardPage>

class GWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit GWizardPage(QWidget * parent = 0) : \
        QWizardPage(parent),
        m_pause(false)
    {}
    virtual bool isComplete() const{
        return m_pause;
    }
    void setPause(bool pause){
        m_pause = pause;
    }
private:
    bool m_pause;
};

#endif // GWIZARDPAGE_H

