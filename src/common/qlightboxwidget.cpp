#include "qlightboxwidget.h"

#include <QPixmap>
#include <QEvent>
#include <QPaintEvent>
#include <QChildEvent>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>

#include "gdatapool.h"

QLightBoxWidget::QLightBoxWidget(GDataPool *dat_ptr, const QString &text, QWidget* _parent, bool _folowToHeadWidget) :
	QWidget(_parent),
	m_isInUpdateSelf(false)
{
	//
	// Родительский виджет должен быть обязательно установлен
	//
	Q_ASSERT_X(_parent, "", Q_FUNC_INFO);

	//
	// Если необходимо, делаем родителем самый "старший" виджет
	//
	if (_folowToHeadWidget) {
		while (_parent->parentWidget() != 0) {
			_parent = _parent->parentWidget();
		}
		setParent(_parent);
	}

    // guozf add
    m_dp_ptr = dat_ptr;

    QLabel* label_ = new QLabel(text, this);
    QFont font_ = label_->font();
    font_.setPointSize(20);
    label_->setFont(font_);
    label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    label_->setAlignment(Qt::AlignCenter);
    label_->setStyleSheet("background:transparent;color:white");

    m_btn_ok = new QPushButton(tr("Confirm"), this);
    m_btn_ok->setFixedSize(120,40);
    m_btn_cancel = new QPushButton(tr("Cancel"), this);
    m_btn_cancel->setFixedSize(120,40);

    if(m_dp_ptr){
        connect(m_btn_ok, SIGNAL(clicked()), m_dp_ptr, SLOT(screen_sound()));
        connect(m_btn_cancel, SIGNAL(clicked()), m_dp_ptr, SLOT(screen_sound()));
    }

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addStretch();
    hl->addWidget(m_btn_ok);
    hl->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Preferred));
    hl->addWidget(m_btn_cancel);
    hl->addStretch();

    QVBoxLayout *vl = new QVBoxLayout;
    vl->addWidget(label_);
    vl->addLayout(hl);
    vl->setStretch(0, 3);
    vl->setStretch(1, 1);

    this->setLayout(vl);

    connect(m_btn_ok, SIGNAL(clicked()), this, SIGNAL(signal_ok()));
    connect(m_btn_cancel, SIGNAL(clicked()), this, SIGNAL(signal_cancel()));
    //! guozf

	//
	// Следим за событиями родительского виджета, чтобы
	// иметь возможность перерисовать его, когда изменяется размер и т.п.
	//
	_parent->installEventFilter(this);

	//
	// Скрываем виджет
	//
	setVisible(false);
}

QLightBoxWidget::~QLightBoxWidget()
{
    m_btn_ok->disconnect(m_dp_ptr);
    m_btn_cancel->disconnect(m_dp_ptr);
    m_btn_ok = 0;
    m_btn_cancel = 0;
    m_dp_ptr = 0;
}

bool QLightBoxWidget::eventFilter(QObject* _object, QEvent* _event)
{
	//
	// Виджету необходимо всегда быть последним ребёнком,
	// чтобы перекрывать остальные виджеты при отображении
	//
	if (_event->type() == QEvent::ChildAdded) {
		QChildEvent* childEvent = dynamic_cast<QChildEvent*>(_event);
		if (childEvent->child() != this) {
			QWidget* parent = parentWidget();
			setParent(0);
			setParent(parent);
		}
	}

	//
	// Если изменился размер родительского виджета, необходимо
	// перерисовать себя
	//
	if (isVisible()
		&& _event->type() == QEvent::Resize) {
		updateSelf();
	}
	return QWidget::eventFilter(_object, _event);
}

void QLightBoxWidget::paintEvent(QPaintEvent* _event)
{
	//
	// Рисуем фон
	//
	QPainter p;
	p.begin(this);
	// ... фото родительского виджета
	p.drawPixmap(0, 0, width(), height(), m_parentWidgetPixmap);
	// ... накладываем затемнённую область
	p.setBrush(QBrush(QColor(0, 0, 0, 220)));
	p.drawRect(0, 0, width(), height());
	p.end();

	//
	// Рисуем всё остальное
	//
	QWidget::paintEvent(_event);
}

void QLightBoxWidget::showEvent(QShowEvent* _event)
{
	//
	// Обновим себя
	//
	updateSelf();

	//
	// Показываемся
	//
	QWidget::showEvent(_event);
}

void QLightBoxWidget::updateSelf()
{
	if (!m_isInUpdateSelf) {
		m_isInUpdateSelf = true;

		{
			//
			// Обновляем отображение
			//
			hide();
			resize(parentWidget()->size());
			m_parentWidgetPixmap = grabParentWidgetPixmap();
			show();
		}

		m_isInUpdateSelf = false;
	}
}

QPixmap QLightBoxWidget::grabParentWidgetPixmap() const
{
	QPixmap parentWidgetPixmap;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	parentWidgetPixmap = parentWidget()->grab();
#else
	parentWidgetPixmap = QPixmap::grabWidget(parentWidget());
#endif

	return parentWidgetPixmap;
}
