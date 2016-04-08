#ifndef __VALUE__
#define __VALUE__
#include <QString>

enum class SimpleType
{
	eUndef,
	eInteger,
	eNumber,
	eString,
	eBoolean,
};

class Value
{
public:
	Value(qint64 integer) : m_type(SimpleType::eInteger), m_integer(integer) {}
	Value(double number)  : m_type(SimpleType::eNumber), m_number(number) {}
	Value(QString string) : m_type(SimpleType::eString) { new(&m_string)QString(string);  }
	Value(bool boolean)   : m_type(SimpleType::eBoolean), m_boolean(boolean) {}
	~Value();

	

private:
	SimpleType m_type;

	union 
	{
		int			m_integer;
		double		m_number;
		QString		m_string;
		bool		m_boolean;
	};
};

#endif __VALUE__