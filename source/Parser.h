#ifndef __PARSER__
#define __PARSER__
#include "scanner.h"
#include "ProgramError.h"
#include "ProgramTree.h"
#include <memory>

class RCADParser
{
public:

	// parse a source file
	void Parse(QString module, const unsigned char* start, const unsigned char* end );
	// get the program from the parsed source file
	ProgramModule* takeProgram()		{ return m_module.release(); }
	// get the errors produced by compilation of the source file
	const ModuleErrorList& ErrorList()	{ return m_errorList; }

public:
	void addFunction( FunctionDecl* function );
	void OnError( const TokenBase& tok, const int* expected, unsigned int nExpected );
	void OnStackOverflow();
private:
	QString _getExpected(int expected);
private:
	Scanner								m_scanner;
	std::unique_ptr<ProgramModule>		m_module;
	ModuleErrorList						m_errorList;
};

#endif // __PARSER__