#ifndef __PREFERENCEDIALOG__
#define __PREFERENCEDIALOG__
#include "ui_preference.h"
#include "rcadscilexer.h"
#include <QDialog>
class QFont;

class PreferenceDialog : public QDialog, public Ui::PreferenceDialog
{
	Q_OBJECT

public:
	PreferenceDialog(QWidget *parent = 0);

public:
	virtual void accept() Q_DECL_OVERRIDE;

private slots:
	void changePage(int);
	void changeFontSize();
	void changeFont(QFont);
	void toggleFontBold(bool);
	void toggleFontItalic(bool);
	void toggleFontUnderline(bool);
	void changeFontForegroudColor(QColor);
	void changeFontBackgroudColor(QColor);
	void changeStyle(int);
	void resetStyle();
	void resetAllStyle();

private:
	void __fillOptionList();
	void __fillStyleList();
	void __updateFontSize(const QFont& font);
	void __updateStyleSample();

private:
	RCadSciLexer lexer;
};


#endif // __PREFERENCEDIALOG__




