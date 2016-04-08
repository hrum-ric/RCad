#include <QDebug>
#include <QColor>
#include <QFont>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscistyle.h>
#include <memory>
#include "rcadscilexer.h"
#include "scanner.h"
#include "ParserBase.h"

RCadSciLexer::RCadSciLexer( QObject *parent ): QsciLexerCustom( parent )
{
}

RCadSciLexer::~RCadSciLexer()
{
}

const char* RCadSciLexer::language() const
{
	return "rcad";
}

void RCadSciLexer::styleText( int start, int end )
{
	QString source;

	if( !editor() )
		return;

	auto buffer = std::make_unique<char[]>(end - start + 1);
	editor()->SendScintilla( QsciScintilla::SCI_GETTEXTRANGE, start, end, buffer.get() );

	startStyling( start, 0x1f );

	Scanner scanner;
	auto string_start = (const unsigned char*)(buffer.get());
	auto string_end = string_start+strlen(buffer.get());
	scanner.reset( string_start, string_end );

	for( Token token = scanner.nextToken(); token.tokenID() != TOKEN_ENDFILE; token = scanner.nextToken() )
	{
		int style = Default;

		switch (token.tokenID())
		{
		case TOKEN_INTEGER:
		case TOKEN_REAL:
			style = Number;
			break;
		case TOKEN_STRING:
			style = String;
			break;
		case TOKEN_EQ:
		case TOKEN_NE:
		case TOKEN_GT:
		case TOKEN_GE:
		case TOKEN_LT:
		case TOKEN_LE:
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_TIMES:
		case TOKEN_DIVIDE:
		case TOKEN_MODULO:
			style = Operator;
			break;
		case TOKEN_KW_INTEGER:
		case TOKEN_KW_NUMBER:
		case TOKEN_KW_STRING:
			style = Type;
			break;
		case TOKEN_KW_FUNCTION:
		case TOKEN_KW_IF:
		case TOKEN_KW_THEN:
		case TOKEN_KW_ELSE:
		case TOKEN_KW_FOR:
		case TOKEN_KW_FROM:
		case TOKEN_KW_TO:
		case TOKEN_KW_DO:
		case TOKEN_KW_IN:
		case TOKEN_KW_BY:
		case TOKEN_KW_END:
		case TOKEN_KW_VAR:
		case TOKEN_KW_AND:
		case TOKEN_KW_OR:
		case TOKEN_KW_NOT:
		case TOKEN_KW_TRUE:
		case TOKEN_KW_FALSE:
			style = Keywords;
			break;
		case TOKEN_PERIOD:
			style = Period;
			break;
		case TOKEN_LEFT_BRACKET:
		case TOKEN_RIGHT_BRACKET:
			style = Bracket;
			break;
		case TOKEN_LEFT_PARENTHESIS:
		case TOKEN_RIGHT_PARENTHESIS:
			style = Parenthesis;
			break;
		case TOKEN_IDENTIFIER:
			style = Identifier;
			break;
		}
		
		int blankLen = token.start()-string_start;
		setStyling(blankLen, Default);
		setStyling(token.len(), style);
		string_start = token.end();
	} 
}

QString RCadSciLexer::description(int style) const
{
	switch (style)
	{
	case Default:		return tr("Default");
	case Number:		return tr("Number");
	case String:		return tr("String");
	case Operator:		return tr("Operator");
	case Type:			return tr("Type");
	case Keywords:		return tr("Keywords");
	case Period:		return tr("Period");
	case Bracket:		return tr("Bracket");
	case Parenthesis:	return tr("Parenthesis");
	case Identifier:	return tr("Identifier");
	}

	return QString();
}

QColor RCadSciLexer::defaultColor( int style ) const
{
	switch( style )
	{
	case Default:		return Qt::black;
	case Number:		return Qt::darkRed;
	case String:		return Qt::darkGreen;
	case Operator:		return Qt::black;
	case Type:			return Qt::darkBlue;
	case Keywords:		return Qt::blue;
	case Period:		return Qt::black;
	case Bracket:		return Qt::black;
	case Parenthesis:	return Qt::black;
	case Identifier:	return Qt::black;
	default:
		return QColor( 0x0, 0x00, 0xFF );
	}
}
