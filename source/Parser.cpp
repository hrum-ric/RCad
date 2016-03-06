#include "Parser.h"
#include "ParserBase.h"
#include <malloc.h>
#include <QLoggingCategory>
#include <QCoreApplication>

void* BaseParseAlloc( void *(*mallocProc)(size_t) );
void  BaseParseFree( void *p, void( *freeProc )(void*) );
void  BaseParse( void *p, int yymajor, TokenBase pToken, RCADParser* );
void  BaseParseTrace( const QLoggingCategory * pclTraceCategory, const char *zTracePrompt );

static const QLoggingCategory LOG_PARSER( "parser" ); 

void RCADParser::Parse( QString module, const unsigned char* start, const unsigned char* end )
{
	m_scanner.reset( start, end );
	m_module.reset( new ProgramModule );
	m_errorList.reset(new ModuleErrorList(module) );

#ifndef NDEBUG
	BaseParseTrace( &LOG_PARSER, "parser" );
#endif // #ifndef NDEBUG

	void* pParser = BaseParseAlloc( malloc );

	Token token;
	do
	{
		token = m_scanner.nextToken();
		BaseParse( pParser, token.tokenID(), token, this );
	} while (token.tokenID() != TOKEN_ENDFILE);

	BaseParse( pParser, 0, token, this );
	BaseParseFree( pParser, free );
}

void RCADParser::addFunction(FunctionDecl* function)
{
	m_module->DeclareFunction(function);
}

void RCADParser::OnError( const TokenBase& tok, const int* expected, unsigned int nExpected )
{
	qCDebug( LOG_PARSER, "error" );

	QString message = QCoreApplication::translate("Error", "syntaxe error.");
	QString expectedlist;

	for( unsigned int i = 0; i < nExpected; i++)
	{
		if (!expectedlist.isEmpty()) expectedlist.append(", ");
		expectedlist.append(_getExpected(expected[i]));
	}
	if (!expectedlist.isEmpty())
	{
		message.append('\n');
		message.append(QCoreApplication::translate("Error", "found : %1 \nExpected : %2").arg(_getExpected(tok.tokenID())).arg(expectedlist));
	}

	m_errorList->addError(tok.position(), message);
}

void RCADParser::OnStackOverflow()
{
	m_errorList->addError(TokenPosition(), QCoreApplication::translate("Error", "Parser stack overflow") );
}

QString RCADParser::_getExpected(int expected)
{
	switch (expected)
	{
	case TOKEN_KW_AND:					return QString("and");
	case TOKEN_KW_OR:					return QString("or");
	case TOKEN_EQ:						return QString("=");
	case TOKEN_NE:						return QString("!=");
	case TOKEN_GT:						return QString(">");
	case TOKEN_GE:						return QString(">=");
	case TOKEN_LT:						return QString("<");
	case TOKEN_LE:						return QString("<=");
	case TOKEN_PLUS:					return QString("+");
	case TOKEN_MINUS:					return QString("-");
	case TOKEN_TIMES:					return QString("*");
	case TOKEN_DIVIDE:					return QString("/");
	case TOKEN_MODULO:					return QString("%");
	case TOKEN_KW_NOT:					return QString("!");
	case TOKEN_PERIOD:					return QString(".");
	case TOKEN_LEFT_BRACKET:			return QString("[");
	case TOKEN_NEWLINE:
	case RULES_NEWLINE:					return QCoreApplication::translate("Error", "new line");
	case TOKEN_ENDFILE:					return QCoreApplication::translate("Error", "end of function");
	case TOKEN_KW_FUNCTION:				return QString("function");
	case TOKEN_IDENTIFIER:				return QCoreApplication::translate("Error", "identifier");
	case TOKEN_LEFT_PARENTHESIS:		QString("(");
	case TOKEN_VARIADIC:				QString("...");
	case TOKEN_COMMA:					return QCoreApplication::translate("Error", "comma");
	case TOKEN_RIGHT_PARENTHESIS:		QString(")");
	case TOKEN_KW_END:					QString("end");
	case TOKEN_KW_VAR:					QString("var");
	case TOKEN_RIGHT_BRACKET:			QString("]");
	case TOKEN_KW_IF:					QString("if");
	case TOKEN_KW_THEN:					QString("then");
	case TOKEN_KW_ELSE:					QString("else");
	case TOKEN_KW_FOR:					QString("for");
	case TOKEN_KW_FROM:					QString("from");
	case TOKEN_KW_TO:					QString("to");
	case TOKEN_KW_DO:					QString("do");
	case TOKEN_KW_IN:					QString("in");
	case TOKEN_KW_BY:					QString("by");
	case TOKEN_COLON:					QString(":");
	case TOKEN_INTEGER:					return QCoreApplication::translate("Error", "an integer");
	case TOKEN_REAL:					return QCoreApplication::translate("Error", "a number");
	case TOKEN_STRING:					return QCoreApplication::translate("Error", "a string");
	case TOKEN_KW_TRUE:					return QCoreApplication::translate("Error", "true");
	case TOKEN_KW_FALSE:				return QCoreApplication::translate("Error", "false");
	case TOKEN_KW_INTEGER:				return QCoreApplication::translate("Error", "integer");
	case TOKEN_KW_NUMBER:				return QCoreApplication::translate("Error", "number");
	case TOKEN_KW_STRING:				return QCoreApplication::translate("Error", "string");
	case RULES_FUNCTION_LIST:			return QCoreApplication::translate("Error", "a list of function declaration");
	case RULES_STATEMENT:				return QCoreApplication::translate("Error", "a statement");
	case RULES_EXPRESSION:				return QCoreApplication::translate("Error", "an expression");
	case RULES_VARIABLE:				return QCoreApplication::translate("Error", "a variable");
	case RULES_PARAMETERS_CALL:			
	case RULES_PARAMETERS_CALL_LIST:	return QCoreApplication::translate("Error", "a list of parameters");
	case RULES_PARAMETER_CALL:			return QCoreApplication::translate("Error", "a parameter");
	case RULES_LITERAL:					return QCoreApplication::translate("Error", "a literal");
	case RULES_TYPE:					return QCoreApplication::translate("Error", "a type");
	default:							return QString();
	};
}