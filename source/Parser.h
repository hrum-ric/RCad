#ifndef __PARSER__
#define __PARSER__
#include "scanner.h"

class RCADParser
{
public:

	void Parse( const unsigned char* start, const unsigned char* end );

public:
	void OnError( Scanner::Token* tok, const int* expected, unsigned int nExpected );
private:
	Scanner	m_scanner;
};

#endif // __PARSER__