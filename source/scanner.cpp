#include "scanner.h"
#include "ParserBase.h"

Scanner::Scanner() : m_start(nullptr), m_end(nullptr), m_cursor(nullptr), m_line(1)
{}

void Scanner::reset( const unsigned char* start, const unsigned char* end )
{
	m_start = m_cursor = start;
	m_end = end;
	m_line = 1;
}

Scanner::Token Scanner::nextToken()
{
start:
	// end of string, nothing left to scan
	if( m_cursor >= m_end ) return TOKEN_END;
	m_tokenStart = m_cursor;

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
	case '9':	return scanNumber();
	// string
	case '"':	return scanString();
	// symbol
	case '(':	m_cursor++; return Token(TOKEN_LEFT_PARENTHESIS);
	case ')':	m_cursor++; return Token(TOKEN_RIGHT_PARENTHESIS);
	case '{':	m_cursor++; return Token(TOKEN_LEFT_CURLY_BRACE);
	case '}':	m_cursor++; return Token(TOKEN_RIGHT_CURLY_BRACE);
	case ',':	m_cursor++; return Token(TOKEN_COMMA);
//	case ':':	m_cursor++; return Token(TOKEN_COLON);
//	case '=':	m_cursor++; return Token(TOKEN_EQUAL);
	// new line
	case '\n':
	case '\r':	skipNewLigne(); goto start;
	// white space
	case ' ':
	case '\t':	m_cursor++; goto start;
	}
	// keyword or identifier
	if( is7bitLetter( c ) )
	{
		return scanKeywordOrIdentifier();
	}
	// identifier
	if( isValidIdentifier( true ) )
	{
		return scanIdentifier();
	}
	return Token(TOKEN_ERROR);
}

// check if character is an US-AINSI letter
bool Scanner::is7bitLetter( unsigned char c )
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// scan an integer or a real
Scanner::Token Scanner::scanNumber()
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
Scanner::Token Scanner::scanString()
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
		case '\n':  skipNewLigne();
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
void Scanner::skipNewLigne()
{
	m_line++;

	if( m_cursor[0] == '\n' && m_cursor[1] == '\r' ||
		m_cursor[0] == '\r' && m_cursor[1] == '\n' ) m_cursor++;
	m_cursor++;
}

// get the current utf8 character, and the length in byte of the letter
bool Scanner::GetChar( unsigned long& c32, int& length )
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
bool Scanner::isValidIdentifier( bool firstChar /*= false*/ )
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
		if( GetChar( c32, length ) )
		{
			// todo check letter
			return true;
		}
	}
	return false;
}

// parse an identifier
Scanner::Token Scanner::scanIdentifier()
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
			if( !GetChar( c32, length ) ) break;
			// todo check letter
			m_cursor += length;
		}
	}
	return Token(TOKEN_IDENTIFIER,m_tokenStart,m_cursor);
}


#define SCAN_LETTER(start,token)				id_##start:									\
													m_cursor++;								\
													if( m_cursor >= m_end ) return token;	\
													switch( *m_cursor )						\
													{
#define NEXT_LETTER(start,letter)					case letter : goto id_##start;
#define END_KEYWORD(letter,token)					case letter :														\
														m_cursor++;														\
														if( m_cursor >= m_end || !isValidIdentifier() ) return token;	\
														return scanIdentifier();
#define END_SCAN_LETTER(token)						default:												\
														if( isValidIdentifier() ) 							\
														{													\
															return scanIdentifier();						\
														}													\
														return token;										\
													};
#define CONTINUE_KEYWORD(start,letter,next)		id_##start:																		\
													m_cursor++;																	\
													if( m_cursor >= m_end ) return Token(TOKEN_IDENTIFIER,m_tokenStart,m_end);	\
													if( *m_cursor == letter )													\
														goto id_##next;															\
													return scanIdentifier();
#define TERMINATE_KEYWORD(start,letter,token)	id_##start:																		\
													m_cursor++;																	\
													if( m_cursor >= m_end ) return Token(TOKEN_IDENTIFIER,m_tokenStart,m_end);	\
													if( *m_cursor == letter )													\
													{																			\
														m_cursor++;																\
														if( m_cursor >= m_end ) return token;									\
														if( isValidIdentifier() )												\
														{																		\
															return scanIdentifier();											\
														}																		\
														return token;															\
													}																			\
													return scanIdentifier();

// scan to find if the word is a keyword or an identifier
Scanner::Token Scanner::scanKeywordOrIdentifier()
{
	switch( *m_cursor )
	{
	case 'f': goto id_f;
	case 'i': goto id_i;
	case 't': goto id_t;
	case 'v': goto id_v;
	default:
		return scanIdentifier();
	};
	SCAN_LETTER( f, TOKEN_IDENTIFIER)
		NEXT_LETTER( fa, 'a' )
		NEXT_LETTER( fo, 'o' )
		NEXT_LETTER( fu, 'u' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	CONTINUE_KEYWORD( fa, 'l', fal )
	CONTINUE_KEYWORD( fal, 's', fals )
	TERMINATE_KEYWORD( fals, 'e', TOKEN_KW_FALSE )
	TERMINATE_KEYWORD( fo, 'r', TOKEN_KW_FOR )
	CONTINUE_KEYWORD( fu, 'n', fun )
	CONTINUE_KEYWORD( fun, 'c', func )
	CONTINUE_KEYWORD( func, 't', funct )
	CONTINUE_KEYWORD( funct, 'i', functi )
	CONTINUE_KEYWORD( functi, 'o', functio )
	TERMINATE_KEYWORD( functio, 'n', TOKEN_KW_FUNCTION )
	SCAN_LETTER( i, TOKEN_IDENTIFIER)
		END_KEYWORD( 'f', TOKEN_KW_IF)
		NEXT_LETTER( in, 'n' )
	END_SCAN_LETTER( TOKEN_IDENTIFIER )
	CONTINUE_KEYWORD( in, 't', int )
	CONTINUE_KEYWORD( int, 'e', inte )
	CONTINUE_KEYWORD( inte, 'g', integ )
	CONTINUE_KEYWORD( integ, 'e', intege )
	TERMINATE_KEYWORD( intege, 'r', TOKEN_KW_INTEGER )
	CONTINUE_KEYWORD( t, 'r', tr )
	CONTINUE_KEYWORD( tr, 'u', tru )
	TERMINATE_KEYWORD( tru, 'e', TOKEN_KW_TRUE )
	CONTINUE_KEYWORD( v, 'a', va )
	TERMINATE_KEYWORD( va, 'r', TOKEN_KW_VAR )
}