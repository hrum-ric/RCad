#ifndef __COLOR_PICKER__
#define __COLOR_PICKER__
#include <QPushButton>
#include <QColor>
class ColorPickerPopup;

class ColorPicker : public QPushButton
{
	Q_OBJECT
public:
	ColorPicker(QWidget *parent = nullptr);

	QColor	color();
	void	setColor(QColor);

signals:
	void colorSelected(QColor color);

private:
	friend class ColorPickerPopup;
	void	__ignoreNextClick();
	void	__selectColor(QColor color);
private:
	QColor				m_color;
	ColorPickerPopup*	m_popup;
	bool				m_bIgnoreNextClick;
};


#endif // __COLOR_PICKER__