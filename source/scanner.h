#ifndef __RCAD_SCANNER__
#define __RCAD_SCANNER__
#include "Token.h"
#include "ProgramError.h"
#include <QString>


class Scanner
{
public:
	Scanner();
	void  reset( const unsigned char* start, const unsigned char* end );
	Token nextToken();

private:

	Token	_nextToken(int& firstLine, int& firstColumn);
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
	const unsigned char* m_lineStart;
	unsigned int		 m_lineNumber;


};

#endif // __RCAD_SCANNER__