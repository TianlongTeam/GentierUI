#include "widgetkeyboard.h"

#include "mymessagebox.h"

#include <QSignalMapper>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSqlQueryModel>
#include <QSqlError>

WidgetKeyboard::WidgetKeyboard(QWidget *parent)
    : QWidget(0)
{
    setupUi(this);
    signalMapperHZ = NULL;

    isEnglish = true;  //默认为英文模式

    m_pParent = parent;
    isCaps = false;

    changeTextCaps(false);

    signalMapper = new QSignalMapper(this);
    allButtons = findChildren<QToolButton *>();

    for (int i=0;i<allButtons.count();i++)
    {
        allButtons.at(i)->setAutoRepeat(false);
        connect(allButtons.at(i), SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(allButtons.at(i), i);
    }
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slt_btn_clicked(int)));
    connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(sig_key_press()));
    hzpanel->setVisible(false);
    HZButtons.clear();
    HZButtons.append(hz1);
    HZButtons.append(hz2);
    HZButtons.append(hz3);
    HZButtons.append(hz4);
    HZButtons.append(hz5);
    HZButtons.append(hz6);
    HZButtons.append(hz7);
    HZButtons.append(hz8);

    init_keyBoard_Language();

    for(int i=0;i<HZButtons.count();i++)
    {
        HZButtons.at(i)->setAutoRepeat(false);
        connect(HZButtons.at(i),SIGNAL(clicked()),this,SIGNAL(sig_key_press()));
    }    

    resize(700,300);

    ZHPY->setFocus();
}

void WidgetKeyboard::Set_Parent(QWidget *parent, QWidget *focus_parent)
{
    if((parent == NULL) || (focus_parent == NULL))
        return;
    setParent(parent);
    m_pParent = focus_parent;
}

WidgetKeyboard::~WidgetKeyboard()
{
    qDebug("--> Destructor WidgetKeyboard ");
    delete signalMapper;
    if(signalMapperHZ!= NULL) delete signalMapperHZ;
}

void WidgetKeyboard::setDatabase(/*QSqlDatabase _db*/)
{
    //    db = _db;
    //    if(!db.isOpen())
    //    {
    //        My_MessageBox::warning(this,tr("ErrorInfo"),tr("An error occurred while opening the database!"));
    //    }
}

void WidgetKeyboard::init_keyBoard_Language()
{
//    if(sys_lan == Lan_ENGLISH)
    if(this->property("Language").toString() == trUtf8("English"))
    {
        Switch_ZH->setText(tr("ABC"));
        Switch_ZH->setVisible(false);
        Set_ChEnable(false);
    }
    else /*if(sys_lan == CHINESE)*/
    {
        Switch_ZH->setText(tr("ABC"));
        Switch_ZH->setVisible(true);
        Set_ChEnable(true);
    }
}

void WidgetKeyboard::setLanguage(const int &lan)
{
    Q_UNUSED(lan)
    //    language = lan;
    //    if(language != CHINESE)
    //    {
    //        Switch_ZH->setText(tr("ABC"));
    //        Switch_ZH->setVisible(false);
    //        Set_ChEnable(false);
    //        // Switch_ZH->setEnabled(false);
    //    }
    //    else
    //    {
    //        Switch_ZH->setText(tr("ABC"));
    //        Switch_ZH->setVisible(true);
    //        Set_ChEnable(true);
    //        // Switch_ZH->setEnabled(false);
    //    }
}

void WidgetKeyboard::setPeriodVisible(bool visible)
{
    //隐藏按键（.）
    btnPeriod->setVisible(visible);
}

bool WidgetKeyboard::Set_ChEnable(bool _ch_enable/*,const QString &dbName*/)
{
    if(_ch_enable) //load chinese
    {
        hzoffset = 0;
        if(signalMapperHZ == NULL)
        {
            signalMapperHZ = new QSignalMapper(this);
            for (int i=0;i<HZButtons.count();i++)
            {
                connect(HZButtons.at(i), SIGNAL(clicked()), signalMapperHZ, SLOT(map()));
                signalMapperHZ->setMapping(HZButtons.at(i), i);
            }
            connect(signalMapperHZ, SIGNAL(mapped(int)), this, SLOT(slt_HZ_clicked(int)));
        }


        Switch_ZH->setEnabled(true);
        return true;
    }
    else
    {
        Switch_ZH->setText(tr("ABC"));
        Switch_ZH->setEnabled(false);
        isEnglish = true;
    }
    return true;
}

// 键盘区
void WidgetKeyboard::slt_btn_clicked(int btn)
{
    if(m_pParent ==NULL) return;
    if(m_pParent->focusWidget() == NULL) return;
    QString strKeyId = allButtons.at(btn)->accessibleName();
    bool isOk = false;
    int keyId = strKeyId.toInt(&isOk, 16);
    if (strKeyId.isEmpty() || !isOk) {
        My_MessageBox mb;
        mb.ginformation(NULL, NULL,tr("Key"),tr("Key Not Found"));
        return;
    }

    QString ch = allButtons.at(btn)->text().trimmed();

    bool sendkey = true;
    if(!isEnglish)  // is chinese.
    {
        sendkey = false;
        QString ttext =  ZHPY->text();
        if((keyId < 0x5b) &&(keyId > 0x40))  //now 'a' to 'z'
        {
            ttext +=ch;
        }
        else if((keyId == 0x01000003) &&(ttext.length()>0)) //backspace.
        {
            ttext = ttext.left(ttext.length()-1);
        }
        else  //send key to the focused .
            sendkey = true;

        if(!sendkey)
        {
            ZHPY->setText(ttext);
            hzoffset = 0;
            QueryHZ(ttext);
            return;
        }
    }

    if(sendkey)
    {
        int involvedKeys = 1;
        Qt::KeyboardModifiers Modifier = Qt::NoModifier;

//        bool isTextKey = false;

        if (keyId==Qt::Key_Space)
            ch = " ";
        else if (checkNotTextKey(keyId))
            ch = QString();
//        else
//            isTextKey = true;

        QKeyEvent keyEvent(QEvent::KeyPress, keyId, Modifier, ch, false, involvedKeys);
        QApplication::sendEvent(m_pParent->focusWidget(), &keyEvent);

        if(keyId == Qt::Key_Enter)
            emit sig_press_over(true);
        else if(keyId == Qt::Key_Close)
            emit sig_press_over(false);
    }
}
//汉字选择
void WidgetKeyboard::slt_HZ_clicked(int btn)
{
    //  if(!currentDatabase.isOpen()) return;  //if hz database closed, do nothing.

    if(m_pParent == NULL) return;
    if(m_pParent->focusWidget() == NULL) return;

    QString hz = HZButtons.at(btn)->text();

    if(hz.isEmpty()) return;


    //    if(m_pParent->hasFocus())
    if(m_pParent->focusWidget() != NULL)
    {
        QLineEdit *cobj = /*static_cast*/dynamic_cast<QLineEdit*>(m_pParent->focusWidget());
        if(cobj != NULL)
        {
            //we have the proper object, fill the character into the right place.
            QString abc = cobj->text();
            int cur_pos = cobj->cursorPosition();
            abc.insert(cur_pos,hz);
            cobj->setText(abc);
            cobj->setCursorPosition(cur_pos+1);
            ZHPY->setText("");
        }
    }
}



void WidgetKeyboard::on_btnCaps_toggled(bool checked)
{
    changeTextCaps(checked);
    isCaps = checked;
}

void WidgetKeyboard::changeTextCaps(bool isCaps)
{
    if (isCaps) {
        btnQ->setText(QChar('Q'));
        btnW->setText(QChar('W'));
        btnE->setText(QChar('E'));
        btnR->setText(QChar('R'));
        btnT->setText(QChar('T'));
        btnY->setText(QChar('Y'));
        btnU->setText(QChar('U'));
        btnI->setText(QChar('I'));
        btnO->setText(QChar('O'));
        btnP->setText(QChar('P'));

        btnA->setText(QChar('A'));
        btnS->setText(QChar('S'));
        btnD->setText(QChar('D'));
        btnF->setText(QChar('F'));
        btnG->setText(QChar('G'));
        btnH->setText(QChar('H'));
        btnJ->setText(QChar('J'));
        btnK->setText(QChar('K'));
        btnL->setText(QChar('L'));

        btnZ->setText(QChar('Z'));
        btnX->setText(QChar('X'));
        btnC->setText(QChar('C'));
        btnV->setText(QChar('V'));
        btnB->setText(QChar('B'));
        btnN->setText(QChar('N'));
        btnM->setText(QChar('M'));
    }
    else {
        btnQ->setText(QChar('q'));
        btnW->setText(QChar('w'));
        btnE->setText(QChar('e'));
        btnR->setText(QChar('r'));
        btnT->setText(QChar('t'));
        btnY->setText(QChar('y'));
        btnU->setText(QChar('u'));
        btnI->setText(QChar('i'));
        btnO->setText(QChar('o'));
        btnP->setText(QChar('p'));

        btnA->setText(QChar('a'));
        btnS->setText(QChar('s'));
        btnD->setText(QChar('d'));
        btnF->setText(QChar('f'));
        btnG->setText(QChar('g'));
        btnH->setText(QChar('h'));
        btnJ->setText(QChar('j'));
        btnK->setText(QChar('k'));
        btnL->setText(QChar('l'));

        btnZ->setText(QChar('z'));
        btnX->setText(QChar('x'));
        btnC->setText(QChar('c'));
        btnV->setText(QChar('v'));
        btnB->setText(QChar('b'));
        btnN->setText(QChar('n'));
        btnM->setText(QChar('m'));
    }
}

bool WidgetKeyboard::checkNotTextKey(int keyId)
{
    if (keyId==Qt::Key_Control
            || keyId==Qt::Key_Return
            || keyId==Qt::Key_Close
            || keyId==Qt::Key_Enter
            || keyId==Qt::Key_CapsLock
            || keyId==Qt::Key_Insert
            || keyId==Qt::Key_Delete)
    {
        return true;
    }
    else
        return false;
}


//QSqlError WidgetKeyboard::addConnection(const QString &dbName)
//{
//    QSqlError err;
//    bool opened;
//    if(!currentDatabase.isValid())
//        currentDatabase = QSqlDatabase::addDatabase("QSQLITE", "HZK");
//    currentDatabase.setDatabaseName(dbName); //
//    currentDatabase.setHostName("");
//    currentDatabase.setPort(0);
//
//    opened = currentDatabase.open("","");
//    err = currentDatabase.lastError();
//
//    if (!opened)
//    {
//        currentDatabase = QSqlDatabase();
//        QSqlDatabase::removeDatabase("HZK");
//    }
//    return err;
//}


int WidgetKeyboard::QueryHZ(const QString &str)
{
    Q_UNUSED(str);
    QString sql_str = "select word from zhcn where py like '"+ ZHPY->text() +"%' limit 8 offset " + QString("%1").arg(hzoffset);

    QSqlQueryModel *model = new QSqlQueryModel(this);
    //model->setQuery(QSqlQuery(sql_str, currentDatabase));
    //model->setQuery(QSqlQuery(sql_str,sys_data->curDatabase()));
    model->setQuery(sql_str);

    if(model->lastError().type() == QSqlError::NoError)
    {
        int scount = model->rowCount();

        for(int i =0; i<scount; i++)
        {
            HZButtons.at(i)->setText(model->data(model->index(i,0,QModelIndex()),Qt::DisplayRole).toString());
        }
        if(scount<8)
            for(int i=scount; i<8; i++) HZButtons.at(i)->setText("");
        return scount;
    }
    return 0;
}

void WidgetKeyboard::on_Switch_ZH_clicked(bool checked)
{
    emit sig_key_press();

    isEnglish = !checked;
    hzpanel->setVisible(!isEnglish);

    if(isEnglish)
        Switch_ZH->setText(QStringLiteral("ABC"));
    else
        Switch_ZH->setText(QStringLiteral("中文"));
}

void WidgetKeyboard::on_pushButton_previous_clicked()  //prio 8
{
    emit sig_key_press();

    if(hzoffset <8) return;
    hzoffset -=8;

    QueryHZ(ZHPY->text());
}

void WidgetKeyboard::on_pushButton_next_clicked() //next 8
{
    emit sig_key_press();

    hzoffset +=8;
    if(QueryHZ(ZHPY->text())==0) {hzoffset -=8; QueryHZ(ZHPY->text());}
}

void WidgetKeyboard::clearCtrl()
{
    ZHPY->setText("");
    hz1->setText("");
    hz2->setText("");
    hz3->setText("");
    hz4->setText("");
    hz5->setText("");
    hz6->setText("");
    hz7->setText("");
    hz8->setText("");
    if(Switch_ZH->text().trimmed().compare(tr("中文"),Qt::CaseSensitive)==0)
        Switch_ZH->click();
}
