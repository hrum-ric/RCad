#ifndef __RCAD_SCI_LEXER__
#define __RCAD_SCI_LEXER__

#include <QObject>
#include <Qsci/qscilexercustom.h>

class QsciStyle;

class RCadSciLexer : public QsciLexerCustom
{
	Q_OBJECT

public:
	enum
	{
		Default= 0,
		Number,
		String,
		Operator,
		Type,
		Keywords,
		Period,
		Bracket,
		Parenthesis,
		Identifier,
		MaxStyle
	};

	RCadSciLexer( QObject *parent = 0 );
	~RCadSciLexer();

	virtual const char *language() const Q_DECL_OVERRIDE;
	virtual QString description( int ) const Q_DECL_OVERRIDE;
	virtual void styleText( int start, int end ) Q_DECL_OVERRIDE;
	virtual QColor defaultColor( int ) const Q_DECL_OVERRIDE;

};

#endif // __RCAD_SCI_LEXER__