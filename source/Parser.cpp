#include "Parser.h"
#include "ParserBase.h"
#include <malloc.h>
#include <QLoggingCategory>

void* BaseParseAlloc( void *(*mallocProc)(size_t) );
void  BaseParseFree( void *p, void( *freeProc )(void*) );
void  BaseParse( void *p, int yymajor, Scanner::Token* pToken, RCADParser* );
void  BaseParseTrace( const QLoggingCategory * pclTraceCategory, const char *zTracePrompt );

static const QLoggingCategory LOG_PARSER( "parser" ); 

void RCADParser::Parse( const unsigned char* start, const unsigned char* end )
{
	m_scanner.reset( start, end );

#ifndef NDEBUG
	BaseParseTrace( &LOG_PARSER, "parser" );
#endif // #ifndef NDEBUG

	void* pParser = BaseParseAlloc( malloc );

	Scanner::Token token;
	do
	{
		token = m_scanner.nextToken();
		BaseParse( pParser, token.m_tokenID, &token, this );
	} while( token.m_tokenID != TOKEN_ENDFILE );

	BaseParse( pParser, 0, &token, this );
	BaseParseFree( pParser, free );
}

void RCADParser::OnError( Scanner::Token* tok, const int* expected, unsigned int nExpected )
{
	qCDebug( LOG_PARSER, "error" );
	//for( unsigned int i = 0; i < nExpected; i++ ) qCDebug( LOG_PARSER, QString(i) );
}