#include "PreferenceDialog.h"
#include <QSettings>
#include <QPainter>
#include "QPalette"

class OptionDelegate : public QAbstractItemDelegate {
public:
	OptionDelegate(QObject *parent = 0) : QAbstractItemDelegate(parent) {}

	void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const 
	{
		QStyleOptionViewItem opt = option;
		if (opt.state & QStyle::State_Selected) {
			painter->fillRect(opt.rect, opt.palette.highlight());
		}

		QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
		QSize ds = opt.decorationSize;
		QRect decorationRect(opt.rect.x() + 4, opt.rect.top() + 4, opt.rect.width() - 8, ds.height());

		icon.paint(painter, decorationRect, Qt::AlignHCenter | Qt::AlignTop, opt.state & QStyle::State_Enabled ? ((opt.state & QStyle::State_Selected) && opt.showDecorationSelected ? QIcon::Selected : QIcon::Normal) : QIcon::Disabled);

		QString displayText = index.data(Qt::DisplayRole).toString();
		painter->setPen(opt.palette.color(option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
		QRect displayRect = opt.rect.adjusted(2, ds.height() + 2, -2, -2);
		painter->drawText(displayRect, Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap, opt.fontMetrics.elidedText(displayText, opt.textElideMode, displayRect.width()));
	}
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const 
	{
		QSize ds = option.decorationSize;
		int totalwidth = option.fontMetrics.width(index.data(Qt::DisplayRole).toString());
		QSize ts = QSize(totalwidth, option.fontMetrics.height());
		return QSize(qBound(ds.width(), ts.width(), option.rect.width()), ds.height() + 6 + ts.height());
	}
};

PreferenceDialog::PreferenceDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	QSettings settings;
	lexer.readSettings(settings);

	__fillOptionList();
	__fillStyleList();
}

void PreferenceDialog::__fillOptionList()
{
	optionList->setItemDelegate(new OptionDelegate(optionList));
	optionList->addItem(new QListWidgetItem(QIcon(":/images/syntax_highlighting.png"), tr("syntax highlighting")));
	optionList->addItem(new QListWidgetItem(QIcon(":/images/syntax_highlighting.png"), tr("autres")));

	changePage(0);
}

void PreferenceDialog::__fillStyleList()
{
	for (int i = 0; i < 128; ++i)
	{
		if ( !lexer.description(i).isEmpty() )
		{
			styleList->addItem(lexer.description(i));
		}
	}
	styleList->setCurrentRow(0);
}

void PreferenceDialog::changePage(int)
{
	optionPages->setCurrentIndex(optionList->currentRow());
}

void PreferenceDialog::changeFontSize()
{
	int nIndice = styleList->currentRow();

	QFont font = lexer.font(nIndice);
	font.setPointSize(fontSize->currentData().toInt());
	lexer.setFont(font, nIndice);

	__updateStyleSample();
}

void PreferenceDialog::changeFont(QFont)
{
	int nIndice = styleList->currentRow();

	QFont font = lexer.font(nIndice);
	font.setFamily(fontFamilly->currentFont().family());
	lexer.setFont(font, nIndice);

	__updateFontSize(font);

	__updateStyleSample();
}

void PreferenceDialog::toggleFontBold(bool bold)
{
	int nIndice = styleList->currentRow();

	QFont font = lexer.font(nIndice);
	font.setBold(bold);
	lexer.setFont(font, nIndice);

	__updateStyleSample();
}

void PreferenceDialog::toggleFontItalic(bool italic)
{
	int nIndice = styleList->currentRow();

	QFont font = lexer.font(nIndice);
	font.setItalic(italic);
	lexer.setFont(font, nIndice);

	__updateStyleSample();
}

void PreferenceDialog::toggleFontUnderline(bool underline)
{
	int nIndice = styleList->currentRow();

	QFont font = lexer.font(nIndice);
	font.setUnderline(underline);
	lexer.setFont(font, nIndice);

	__updateStyleSample();
}

void PreferenceDialog::__updateFontSize(const QFont& font)
{
	QFontDatabase fontdatabase;
	QList<int> sizeList = fontdatabase.smoothSizes(font.family(), "");
	if (sizeList.empty()) sizeList = fontdatabase.pointSizes(font.family(), "");
	if (sizeList.empty()) sizeList = fontdatabase.standardSizes();
	if (!sizeList.contains(font.pointSize())) sizeList.append(font.pointSize());

	fontSize->clear();
	for (int size : sizeList) fontSize->addItem(QString::number(size), size);
	fontSize->setCurrentIndex(sizeList.indexOf(fontFamilly->currentFont().pointSize()));
}

void PreferenceDialog::changeFontForegroudColor(QColor c)
{
	int nIndice = styleList->currentRow();
	lexer.setColor(c, nIndice);
	__updateStyleSample();
}
void PreferenceDialog::changeFontBackgroudColor(QColor c)
{
	int nIndice = styleList->currentRow();
	lexer.setPaper(c, nIndice);
	__updateStyleSample();
}

void PreferenceDialog::changeStyle(int nIndice)
{
	QColor foreground = lexer.color(nIndice);
	QColor background = lexer.paper(nIndice);
	QFont font = lexer.font(nIndice);

	fontBold->setChecked( font.bold() );
	fontItalic->setChecked( font.italic() );
	fontUnderline->setChecked( font.underline() );

	fontFamilly->setCurrentFont(font);

	__updateFontSize(font);

	backgroundColorButton->setColor(background);
	foregroundColorButton->setColor(foreground);

	__updateStyleSample();
}

void PreferenceDialog::__updateStyleSample()
{
	int nIndice = styleList->currentRow();

	syntaxExemple->setText(lexer.sample(nIndice));
	syntaxExemple->setFont(lexer.font(nIndice));

	QPalette pal = syntaxExemple->palette();
	pal.setColor(QPalette::Window, lexer.paper(nIndice));
	pal.setColor(QPalette::WindowText, lexer.color(nIndice));
	syntaxExemple->setPalette(pal);
}

void PreferenceDialog::resetStyle()
{
	int nIndice = styleList->currentRow();
	lexer.resetStyle(nIndice);
	__updateStyleSample();
}
void PreferenceDialog::resetAllStyle()
{
	lexer.resetAllStyle();
	__updateStyleSample();
}
void PreferenceDialog::accept()
{
	QSettings settings;
	lexer.writeSettings(settings);

	QDialog::accept();
}



