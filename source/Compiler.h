#ifndef __COMPILER__
#define __COMPILER__
#include <QString>
#include <QByteArray>
class Project;
class Program;

class Compiler
{
public:
	struct SourceInfo { SourceInfo(QString file) : m_file(file) {} QString m_file; QByteArray m_content; };

	Program* compile(QList<SourceInfo> sourceList);

};


#endif // __COMPILER__
