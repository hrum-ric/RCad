#include "Compiler.h"
#include "project.h"
#include "Parser.h"
#include <QFile>

Program* Compiler::compile(QList<SourceInfo> sourceList)
{
	RCADParser parser;
	Program* program=new Program;

	for (SourceInfo info : sourceList)
	{
		if( info.m_content.isEmpty() )
		{
			QFile file(info.m_file);
			file.open(QFile::ReadOnly);
			info.m_content = file.readAll();
		}
		parser.Parse(info.m_file, (const unsigned char*)info.m_content.constData(), (const unsigned char*)info.m_content.constEnd());

		program->AddErrorModule( parser.ErrorList() );
		program->AddProgramModule( parser.takeProgram() );
	}

	return program;
}
