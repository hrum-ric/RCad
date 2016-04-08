#ifndef __PROGRAM_ERROR__
#define __PROGRAM_ERROR__
#include "Token.h"
#include <QString>
#include <QList>

class Error
{
public:
	Error() {}
	Error(TokenPositionBase position, QString message ) : m_position(position), m_message(message) {}
	
	int							startLine() const	{ return m_position.m_startLine; }
	const TokenPositionBase&	position() const	{ return m_position; }
	QString						message() const		{ return m_message; }
private:
	TokenPosition		m_position;
	QString				m_message;
};

class ModuleErrorList
{
public:
	void Clear(QString module) { m_module=module; m_errorList.clear(); }
	void addError(TokenPositionBase position, QString message) { m_errorList.append(Error(position, message)); }

	QString			m_module;
	QList<Error>	m_errorList;
};

class ErrorList
{
public:
	void AddModule(const ModuleErrorList& module) { m_list.append(module); }

	QList<ModuleErrorList>	m_list;
};

#endif // __PROGRAM_ERROR__
