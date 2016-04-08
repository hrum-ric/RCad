#include "scanner.h"
#include "ParserBase.h"

Scanner::Scanner() : m_start(nullptr), m_end(nullptr), m_cursor(nullptr), m_lineStart(nullptr), m_lineNumber(1)
{}

void Scanner::reset( const unsigned char* start, const unsigned char* end )
{
	m_lineStart = m_start = m_cursor = start;
	m_end = end;
	m_lineNumber = 1;
}

Token Scanner::nextToken()
{
	int firstLine, firstColumn;
	Token token = _nextToken(firstLine, firstColumn);
	token.setPosition(firstLine, firstColumn, m_lineNumber, m_cursor - m_lineStart, m_tokenStart, m_cursor);
	return token;
}

Token Scanner::_nextToken(int& firstLine, int& firstColumn)
{
start:
	// end of string, nothing left to scan
	if( m_cursor >= m_end ) return Token(TOKEN_ENDFILE);
	m_tokenStart = m_cursor;
	firstLine = m_lineNumber;
	firstColumn = (m_cursor - m_lineStart);

	char c = *m_cursor;
	switch( c )
	{
	// number
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	return _scanNumber();
	// string
	case '"':	return _scanString();
	// symbol
	case '(':	m_cursor++; return Token(TOKEN_LEFT_PARENTHESIS);
	case ')':	m_cursor++; return Token(TOKEN_RIGHT_PARENTHESIS);
	case '[':	m_cursor++; return Token(TOKEN_LEFT_BRACKET);
	case ']':	m_cursor++; return Token(TOKEN_RIGHT_BRACKET);
	case ',':	m_cursor++; return Token(TOKEN_COMMA);
	case ':':	m_cursor++; return Token(TOKEN_COLON);
	case '+':	m_cursor++; return Token(TOKEN_PLUS);
	case '-':	m_cursor++; return Token(TOKEN_MINUS);
	case '*':	m_cursor++; return Token(TOKEN_TIMES);
	case '/':	
		m_cursor++; 
		if( m_cursor < m_end && *m_cursor == '/' )		{ _skipComment(); goto start; }
		else if( m_cursor < m_end && *m_cursor == '*' ) { _skipMultilineComment(); goto start; }
		else return Token(TOKEN_DIVIDE);
	case '=':	m_cursor++; return Token(TOKEN_EQ);
	case '%':	m_cursor++; return Token(TOKEN_MODULO);
	case '<':	m_cursor++; if( m_cursor < m_end && *m_cursor == '=' ) { m_cursor++; return Token( TOKEN_LE ); } return Token( TOKEN_LT );
	case '>':	m_cursor++; if( m_cursor < m_end && *m_cursor == '=' ) { m_cursor++; return Token( TOKEN_GE ); } return Token( TOKEN_GT );
	case '!':	m_cursor++; if( m_cursor < m_end && *m_cursor == '=' ) { m_cursor++; return Token( TOKEN_NE ); } return Token( TOKEN_KW_NOT );
	case '.':
		m_cursor++;
		if( m_cursor + 1 < m_end && m_cursor[0] == '.' && m_cursor[1] == '.' )
		{
			m_cursor += 2;
			return Token( TOKEN_VARIADIC );
		}
		return Token( TOKEN_PERIOD );
	// new line
	case '\n':
	case '\r':	_skipNewLigne(); return Token(TOKEN_NEWLINE);
	// white space
	case ' ':
	case '\t':	m_cursor++; goto start;
	}

	// unicode symbols
	unsigned long c32;
	int length;
	if( _getChar( c32, length ) )
	{
		switch( c32 )
		{
		case 0x2260: m_cursor += length; return TOKEN_NE;
		}
	}

	// keyword or identifier
	if( _is7bitLetter( c ) )
	{
		Token tok = _scanKeywordOrIdentifier();
		if( tok.tokenID()== TOKEN_IDENTIFIER ) return Token(TOKEN_IDENTIFIER, m_tokenStart, m_cursor);
		return tok;
	}
	// identifier
	if( _isValidIdentifier( true ) )
	{
		return _scanIdentifier();
	}
	return Token(TOKEN_ERROR);
}

// check if character is an US-AINSI letter
bool Scanner::_is7bitLetter( unsigned char c )
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// check if character is an US-AINSI letter
unsigned char to7bitUpper( unsigned char c )
{
	if( 'a' <= c && c <= 'z' ) return c + 'A' - 'a';
	return c;
}

// scan an integer or a real
Token Scanner::_scanNumber()
{
	long long integer = 0;

	// integer
	while( '0' <= *m_cursor && *m_cursor <= '9' )
	{
		integer = integer * 10 + ((*m_cursor) - '0');
		m_cursor++;
	}
	if( *m_cursor != '.' && *m_cursor != 'e' && *m_cursor != 'E' )
	{
		return Token(TOKEN_INTEGER, integer);
	}
	// decimal 
	double value = (double)integer;
	double x = 1.0;
	if( *m_cursor == '.' )
	{
		m_cursor++;
		while( '0' <= *m_cursor && *m_cursor <= '9' )
		{
			value += (x /= 10) * ((*m_cursor) - '0');
			m_cursor++;
		}
	}
	// exponant
	if( *m_cursor == 'e' || *m_cursor == 'E' )
	{
		m_cursor++;
		// sign
		if( *m_cursor == '+' )		{ x = 10;  m_cursor++; }
		else if( *m_cursor == '-' ) { x = 0.0; m_cursor++; }
		else						{ x = 10; }
		// exponant
		int exponant = 0;
		while( '0' <= *m_cursor && *m_cursor <= '9' )
		{
			exponant = exponant * 10 + ((*m_cursor) - '0');
			m_cursor++;
		}
		// update value
		for( ; exponant > 0; exponant-- ) value *= x;
	}
	return Token(TOKEN_REAL, value);
}

// scan a string
Token Scanner::_scanString()
{
	while( true )
	{
		m_cursor++;

		switch( *m_cursor )
		{
		case '"':
			m_cursor++;
			return Token(TOKEN_STRING);
		case '\r':
		case '\n':  _skipNewLigne();
		case '\\':
			m_cursor++;
			switch( *m_cursor )
			{
			case '\\': break;
			case 'n': break;
			case 'r': break;
			case '"': break;
			default: return Token(TOKEN_ERROR);
			}
			break;
		default:
			break;
		}
	}
}

// skip a new line and count line number
void Scanner::_skipNewLigne()
{
	if( m_cursor[0] == '\n' && m_cursor[1] == '\r' ||
		m_cursor[0] == '\r' && m_cursor[1] == '\n' ) m_cursor++;
	m_cursor++;

	m_lineStart = m_cursor;
	m_lineNumber++;
}

// get the current utf8 character, and the length in byte of the letter
bool Scanner::_getChar( unsigned long& c32, int& length )
{
	unsigned long c = m_cursor[0];
	if( c <= 0x7F )
	{
		/* 1 octet*/
		length = 1;
	}
	else if( c <= 0xDF )
	{
		/* 2 octet*/
		if( m_cursor >= m_end ) return false;
		length = 2;
		c = ((c & 0x1F) << 6);
		c += ((m_cursor[1]) & 0x3F);
	}
	else if( c <= 0xEF )
	{
		/* 3 octet*/
		if( m_cursor + 1 >= m_end ) return false;
		length = 3;
		c = ((c & 0x0F) << 12);
		c += ((m_cursor[1]) & 0x3F) << 6;
		c += ((m_cursor[2]) & 0x3F);
	}
	else if( c <= 0xF4 )
	{
		/* 4 octet*/
		if( m_cursor + 2 >= m_end ) return false;
		length = 4;
		c = ((c & 0x07) << 18);
		c += ((m_cursor[1]) & 0x3F) << 12;
		c += ((m_cursor[2]) & 0x3F) << 6;
		c += ((m_cursor[3]) & 0x3F);
	}
	else
	{
		// invalid caracter;
		return false;
	}

	c32 = c;
	return true;
}

// check if the current character is valid inside an identifier
bool Scanner::_isValidIdentifier( bool firstChar /*= false*/ )
{
	if( *m_cursor <= 127 )
	{
		if( 'a' <= *m_cursor && *m_cursor <= 'z' ||
			'A' <= *m_cursor && *m_cursor <= 'Z' ||
			(!firstChar && ('0' <= *m_cursor && *m_cursor <= '9')) ) 
		{
			return true;
		}
	}
	else
	{
		unsigned long c32;
		int length;
		if( _getChar( c32, length ) )
		{
			// todo check letter
			return true;
		}
	}
	return false;
}

void Scanner::_skipComment()
{
	while( m_cursor < m_end && *m_cursor != '\r' && *m_cursor != '\n' )
		m_cursor++;
}
void Scanner::_skipMultilineComment()
{
	m_cursor++;
	while( m_cursor < m_end )
	{
		if( *m_cursor == '*' && (m_cursor + 1) < m_end && *(m_cursor + 1) == '/' )
		{
			m_cursor += 2;
			return;
		}
		m_cursor++;
	}
}

// parse an identifier
Token Scanner::_scanIdentifier()
{
	while( m_cursor < m_end )
	{
		// ascii 7
		if( *m_cursor <= 127 )
		{
			if( !('a' <= *m_cursor && *m_cursor <= 'z' ||
				'A' <= *m_cursor && *m_cursor <= 'Z' ||
				'0' <= *m_cursor && *m_cursor <= '9') )
			{
				break;
			}
			m_cursor++;
		}
		else
		{
			unsigned long c32;
			int length;
			if( !_getChar( c32, length ) ) break;
			// todo check letter
			m_cursor += length;
		}
	}
	return Token(TOKEN_IDENTIFIER,m_tokenStart,m_cursor);
}


#define SCAN_LETTER(start,token)				id_##start:									\
													m_cursor++;								\
													if( m_cursor >= m_end ) return token;	\
													switch( to7bitUpper(*m_cursor) )		\
													{
#define NEXT_LETTER(start,letter)					case letter : goto id_##start;
#define END_KEYWORD(letter,token)					case letter :														\
														m_cursor++;														\
														if( m_cursor >= m_end || !_isValidIdentifier() ) return token;	\
														return _scanIdentifier();
#define END_SCAN_LETTER(token)						default:												\
														if( _isValidIdentifier() ) 							\
														{													\
															return _scanIdentifier();						\
														}													\
														return token;										\
													};
#define CONTINUE_KEYWORD(start,letter,next)		id_##start:																		\
													m_cursor++;																	\
													if( m_cursor >= m_end ) return Token(TOKEN_IDENTIFIER,m_tokenStart,m_end);	\
													if( to7bitUpper(*m_cursor) == letter )										\
														goto id_##next;															\
													return _scanIdentifier();
#define TERMINATE_KEYWORD(start,letter,token)	id_##start:																		\
													m_cursor++;																	\
													if( m_cursor >= m_end ) return Token(TOKEN_IDENTIFIER,m_tokenStart,m_end);	\
													if( to7bitUpper(*m_cursor) == letter )										\
													{																			\
														m_cursor++;																\
														if( m_cursor >= m_end ) return token;									\
														if( _isValidIdentifier() )												\
														{																		\
															return _scanIdentifier();											\
														}																		\
														return token;															\
													}																			\
													return _scanIdentifier();

// scan to find if the word is a keyword or an identifier
Token Scanner::_scanKeywordOrIdentifier()
{
	switch( to7bitUpper(*m_cursor) )
	{
	case 'A': goto id_A;
	case 'B': goto id_B;
	case 'D': goto id_D;
	case 'E': goto id_E;
	case 'F': goto id_F;
	case 'I': goto id_I;
	case 'N': goto id_N;
	case 'O': goto id_O;
	case 'S': goto id_S;
	case 'T': goto id_T;
	case 'V': goto id_V;
	default:
		return _scanIdentifier();
	};
	CONTINUE_KEYWORD( A, 'N', AN )
	TERMINATE_KEYWORD( AN, 'D', TOKEN_KW_AND )
	TERMINATE_KEYWORD( B, 'Y', TOKEN_KW_BY )
	TERMINATE_KEYWORD( D, 'O', TOKEN_KW_DO )
	SCAN_LETTER( E, TOKEN_IDENTIFIER)
		NEXT_LETTER( EL, 'L' )
		NEXT_LETTER( EN, 'N' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	CONTINUE_KEYWORD( EL, 'S', ELS )
	TERMINATE_KEYWORD( ELS, 'E', TOKEN_KW_ELSE )
	TERMINATE_KEYWORD( EN, 'D', TOKEN_KW_END )
	SCAN_LETTER( F, TOKEN_IDENTIFIER)
		NEXT_LETTER( FA, 'A' )
		NEXT_LETTER( FO, 'O' )
		NEXT_LETTER( FR, 'R' )
		NEXT_LETTER( FU, 'U' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	CONTINUE_KEYWORD( FA, 'L', FAL )
	CONTINUE_KEYWORD( FAL, 'S', FALS )
	TERMINATE_KEYWORD( FALS, 'E', TOKEN_KW_FALSE )
	TERMINATE_KEYWORD( FO, 'R', TOKEN_KW_FOR )
	CONTINUE_KEYWORD( FR, 'O', FRO )
	TERMINATE_KEYWORD( FRO, 'M', TOKEN_KW_FROM )
	CONTINUE_KEYWORD( FU, 'N', FUN )
	CONTINUE_KEYWORD( FUN, 'C', FUNC )
	CONTINUE_KEYWORD( FUNC, 'T', FUNCT )
	CONTINUE_KEYWORD( FUNCT, 'I', FUNCTI )
	CONTINUE_KEYWORD( FUNCTI, 'O', FUNCTIO )
	TERMINATE_KEYWORD( FUNCTIO, 'N', TOKEN_KW_FUNCTION )
	SCAN_LETTER( I, TOKEN_IDENTIFIER)
		END_KEYWORD( 'F', TOKEN_KW_IF)
		NEXT_LETTER( IN, 'N' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	SCAN_LETTER( IN, TOKEN_KW_IN)
		NEXT_LETTER( INT, 'T' )
	END_SCAN_LETTER( TOKEN_KW_IN )
	CONTINUE_KEYWORD( INT, 'E', INTE )
	CONTINUE_KEYWORD( INTE, 'G', INTEG )
	CONTINUE_KEYWORD( INTEG, 'E', INTEGE )
	TERMINATE_KEYWORD( INTEGE, 'R', TOKEN_KW_INTEGER )
	SCAN_LETTER( N, TOKEN_IDENTIFIER)
		NEXT_LETTER( NO, 'O' )
		NEXT_LETTER( NU, 'U' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	TERMINATE_KEYWORD( NO, 'T', TOKEN_KW_NOT )
	CONTINUE_KEYWORD( NU, 'M', NUM )
	CONTINUE_KEYWORD( NUM, 'B', NUMB )
	CONTINUE_KEYWORD( NUMB, 'E', NUMBE )
	TERMINATE_KEYWORD( NUMBE, 'R', TOKEN_KW_NUMBER )
	TERMINATE_KEYWORD( O, 'R', TOKEN_KW_OR )
	CONTINUE_KEYWORD( S, 'T', ST )
	CONTINUE_KEYWORD( ST, 'R', STR )
	CONTINUE_KEYWORD( STR, 'I', STRI )
	CONTINUE_KEYWORD( STRI, 'N', STRIN )
	TERMINATE_KEYWORD( STRIN, 'G', TOKEN_KW_STRING )
	SCAN_LETTER( T, TOKEN_IDENTIFIER)
		NEXT_LETTER( TH, 'H' )
		END_KEYWORD( 'O', TOKEN_KW_TO)
		NEXT_LETTER( TR, 'R' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	CONTINUE_KEYWORD( TH, 'E', THE )
	TERMINATE_KEYWORD( THE, 'N', TOKEN_KW_THEN )
	CONTINUE_KEYWORD( TR, 'U', TRU )
	TERMINATE_KEYWORD( TRU, 'E', TOKEN_KW_TRUE )
	CONTINUE_KEYWORD( V, 'A', VA )
	TERMINATE_KEYWORD( VA, 'R', TOKEN_KW_VAR )
}