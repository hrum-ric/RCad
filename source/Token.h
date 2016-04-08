#ifndef __TOKEN__
#define __TOKEN__
#include <QString>

#define TOKEN_ERROR -1

class TokenPositionBase
{
public:
	int	m_startLine;
	int	m_startColumn;
	int	m_endLine;
	int	m_endColumn;
};

class TokenPosition : public TokenPositionBase
{
public:
	TokenPosition() { m_startLine = -1; m_startColumn = -1; m_endLine = -1; m_endColumn = -1; }
	TokenPosition(int startLine, int startColumn, int endLine, int endColumn) { m_startLine = startLine; m_startColumn = startColumn; m_endLine = endLine; m_endColumn = endColumn; }
	TokenPosition(const TokenPositionBase& src) { m_startLine = src.m_startLine; m_startColumn = src.m_startColumn; m_endLine = src.m_endLine; m_endColumn = src.m_endColumn; }
};

struct TokenBase
{
public:
	int					tokenID() const { return m_tokenID; }
	long long			integer() const { return m_integer; }
	double				number() const { return m_number; }
	QString				string() const { return QString::fromUtf8((const char*)m_stIdentifier.start, m_stIdentifier.end - m_stIdentifier.start); }
	TokenPositionBase	position() const { return m_position; }

protected:
	typedef struct { const unsigned char* start; const unsigned char* end; } stIDENTIFIER;
	int					m_tokenID;
	long long			m_integer;
	double				m_number;
	stIDENTIFIER		m_stIdentifier;
	TokenPositionBase	m_position;

};


class Token : public TokenBase
{
public:
	Token() { m_tokenID = TOKEN_ERROR; }
	Token(int tokenID) { m_tokenID = tokenID; }
	Token(int tokenID, long long integer) { m_tokenID = tokenID; m_integer = integer; }
	Token(int tokenID, double number) { m_tokenID = tokenID; m_number = number; }
	Token(int tokenID, const unsigned char* start, const unsigned char* end) { m_tokenID = tokenID; m_stIdentifier = { start, end }; }

	Token setPosition(int startLine, int startColumn, int endLine, int endColumn, const unsigned char* start, const unsigned char* end) { m_position = TokenPosition(startLine, startColumn, endLine, endColumn); m_stIdentifier.start=start; m_stIdentifier.end=end; return *this; }
	
	const unsigned char* start() { return m_stIdentifier.start; }
	const unsigned char* end()   { return m_stIdentifier.end; }
	int len() { return end()-start(); }
};


#endif // __TOKEN__
