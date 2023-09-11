#ifndef QLIGHTBOXWIDGET_H
#define QLIGHTBOXWIDGET_H

#include <QWidget>


/**
 * @brief Класс перекрытия
 */
class GDataPool;
class QPushButton;
class QLightBoxWidget : public QWidget
{
	Q_OBJECT

public:
    explicit QLightBoxWidget(GDataPool *dat_ptr, const QString &text, QWidget* _parent, bool _folowToHeadWidget = false);
    ~QLightBoxWidget();

signals:
    void signal_ok();
    void signal_cancel();
protected:
	/**
	 * @brief Переопределяется для отслеживания собитий родительского виджета
	 */
	bool eventFilter(QObject* _object, QEvent* _event);

	/**
	 * @brief Переопределяется для того, чтобы эмитировать эффект перекрытия
	 */
	void paintEvent(QPaintEvent* _event);

	/**
	 * @brief Переопределяется для того, чтобы перед отображением настроить внешний вид
	 */
	void showEvent(QShowEvent* _event);

private:
	/**
	 * @brief Обновить размер и картинку фона
	 */
	void updateSelf();

	/**
	 * @brief Разрешает конфликт рекурсивного обновления
	 */
	bool m_isInUpdateSelf;

	/**
	 * @brief Обновить картинку фона
	 */
	QPixmap grabParentWidgetPixmap() const;

	/**
	 * @brief Картинка фона
	 */
	QPixmap m_parentWidgetPixmap;

    // guozf add
    GDataPool *m_dp_ptr;
    QPushButton *m_btn_ok, *m_btn_cancel;

};

#endif // QLIGHTBOXWIDGET_H
