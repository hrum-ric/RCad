#ifndef __RCAD_SCANNER__
#define __RCAD_SCANNER__
#include <QString>

#define TOKEN_ERROR -1

class Scanner
{
public:
	class Token
	{
	public:
		Token() : m_tokenID() {}
		Token( int tokenID ) : m_tokenID( tokenID ) {}
		Token( int tokenID, long long integer ) : m_tokenID( tokenID ), m_integer(integer) {}
		Token( int tokenID, double number) : m_tokenID(tokenID), m_number(number) {}
		Token(int tokenID, const unsigned char* start, const unsigned char* end) : m_tokenID(tokenID), m_stIdentifier({ start, end }) {}

		QString toString() { return QString::fromUtf8( (const char*)m_stIdentifier.start, m_stIdentifier.end - m_stIdentifier.start ); }

		int				m_tokenID;
		long long		m_integer;
		double			m_number;
		typedef struct { const unsigned char* start; const unsigned char* end; } stIDENTIFIER;
		stIDENTIFIER	m_stIdentifier;
	};
public:
	Scanner();
	void  reset( const unsigned char* start, const unsigned char* end );
	Token nextToken();

private:
	Token	_scanNumber();
	Token	_scanString();
	void	_skipNewLigne();
	Token	_scanKeywordOrIdentifier();
	Token	_scanIdentifier();
	void	_skipComment();
	void	_skipMultilineComment();
	bool	_getChar( unsigned long& c32, int& length );
	bool	_is7bitLetter( unsigned char c );
	bool	_isValidIdentifier(bool firstChar=false);

private:
	const unsigned char* m_start;
	const unsigned char* m_end;
	const unsigned char* m_cursor;
	const unsigned char* m_tokenStart;
	unsigned int		 m_line;


};

#endif // __RCAD_SCANNER__