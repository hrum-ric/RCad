#ifndef __PROGRAM_TREE__
#define __PROGRAM_TREE__
#include <QString>
#include <vector>
#include <memory>
#include "Value.h"
#include "ProgramError.h"
class Expression;
class Variable;



class Expression
{
public:
};

class UnaryExpression : public Expression
{
public:
	UnaryExpression( Expression* op ) : m_op( op )	{}
protected:
	std::unique_ptr<Expression> m_op;
};

class BinaryExpression : public Expression
{
public:
	BinaryExpression( Expression* op1, Expression* op2 ) : m_op1( op1 ), m_op2( op2 )	{}
protected:
	std::unique_ptr<Expression> m_op1;
	std::unique_ptr<Expression> m_op2;
};


#define DECLARE_UNARY(Name)							\
class Name : public UnaryExpression					\
{													\
public:												\
	Name(Expression* op) : UnaryExpression(op) {}	\
};

#define DECLARE_BINARY(Name)												\
class Name : public BinaryExpression										\
{																			\
public:																		\
	Name(Expression* op1, Expression* op2) : BinaryExpression(op1, op2) {}	\
};

DECLARE_UNARY( MinusExpression );
DECLARE_UNARY( NotExpression );

DECLARE_BINARY( OrExpression );
DECLARE_BINARY( AndExpression );
DECLARE_BINARY( AddExpression );
DECLARE_BINARY( SubstractExpression );
DECLARE_BINARY( MultiplyExpression );
DECLARE_BINARY( DivideExpression );
DECLARE_BINARY( ModuloExpression );

DECLARE_BINARY( EqualCompare );
DECLARE_BINARY( NotEqualCompare );
DECLARE_BINARY( GreaterCompare );
DECLARE_BINARY( GreaterEqualCompare );
DECLARE_BINARY( LessCompare );
DECLARE_BINARY( LessEqualCompare );

class IfExpression : public Expression
{
public:
	IfExpression( Expression* condition, Expression* trueExp, Expression* falseExp ) : m_condition(condition), m_trueExp( trueExp ), m_falseExp( falseExp ) {}
protected:
	std::unique_ptr<Expression>	m_condition;
	std::unique_ptr<Expression>	m_trueExp;
	std::unique_ptr<Expression>	m_falseExp;
};

class Literal : public Expression
{
public:
	Literal( qint64 integer ) : m_value(integer) {}
	Literal(double number) : m_value(number) {}
	Literal( QString string ) : m_value(string) {}
	Literal( bool boolean ) : m_value(boolean) {}
private:
	Value	m_value;
};

class ExpressionMember : public Expression
{
public:
	ExpressionMember(Expression* exp, QString name) : m_expression(exp), m_name(name) {}
protected:
	std::unique_ptr<Expression>	m_expression;
	QString						m_name;
};

class ExpressionSubElement : public Expression
{
public:
	ExpressionSubElement(Expression* exp, Expression* indice) : m_expression(exp), m_indice(indice) {}
protected:
	std::unique_ptr<Expression>	m_expression;
	std::unique_ptr<Expression>	m_indice;
};

// a variable can be evaluated, so is an expression
class Variable : public Expression
{};

class NamedVariable : public Variable
{
public:
	NamedVariable( QString name ) : m_name( name ) {}
protected:
	QString						m_name;
};

// access member of object
class VariableMember : public Variable
{
public:
	VariableMember(Variable* var, QString memberName) : m_variable(var), m_memberName(memberName) {}
protected:
	std::unique_ptr<Variable>	m_variable;
	QString						m_memberName;
};

// element in a array variable
class VariableSubElement : public Variable
{
public:
	VariableSubElement(Variable* var, Expression* indice) : m_variable(var), m_indice(indice) {}
protected:
	std::unique_ptr<Variable>	m_variable;
	std::unique_ptr<Expression>	m_indice;
};


class Statement
{
public:

};

typedef QList<Statement*> StatementList;

class DeclarationStatement : public Statement
{
public:
	DeclarationStatement( QString name, SimpleType type, Expression* val = nullptr ) : m_name( name ), m_type( type ), m_initialValue( val ) {}
protected:
	QString						m_name;
	SimpleType					m_type;
	std::unique_ptr<Expression>	m_initialValue;
};

class ForCounterStatement : public Statement
{
public:
	ForCounterStatement( QString var, Expression* from, Expression* to, Expression* by, StatementList* body ) : m_varName( var ), m_from( from ), m_to( to ), m_by( by ), m_body( body ) {}
protected:
	QString							m_varName;
	std::unique_ptr<Expression>		m_from;
	std::unique_ptr<Expression>		m_to;
	std::unique_ptr<Expression>		m_by;
	std::unique_ptr<StatementList>  m_body;
};

class ForInStatement : public Statement
{
public:
	ForInStatement(QString var, Expression* enumerator, StatementList* body) : m_varName(var), m_enumerator(enumerator), m_body(body) {}
protected:
	QString							m_varName;
	std::unique_ptr<Expression>		m_enumerator;
	std::unique_ptr<StatementList>  m_body;
};

class IfStatement : public Statement
{
public:
	IfStatement( Expression* condition, Statement* stmThen, Statement* stmElse = nullptr ) : m_condition(condition), m_stmThen( stmThen ), m_stmElse( stmElse ) {}
protected:
	std::unique_ptr<Expression> m_condition;
	std::unique_ptr<Statement>  m_stmThen;
	std::unique_ptr<Statement>  m_stmElse;
};

class IfBlocStatement : public Statement
{
public:
	IfBlocStatement(Expression* condition, StatementList* stmThen) : m_condition(condition), m_stmThen(stmThen) {}
	void AddElseIf( Expression* condition, StatementList* stmElse ) { m_elseIf.emplace_back( condition, stmElse ); }
	void AddElse( StatementList* stmElse ) { m_stmElse.reset(stmElse);  }
protected:
	struct stELSEIF 
	{ 
		stELSEIF( Expression* condition, StatementList* stmElse ) : m_condition( condition ), m_stmElse( stmElse ) {} 
		stELSEIF( stELSEIF && src ) { m_condition = std::move( src.m_condition ); m_stmElse = std::move( src.m_stmElse ); }
		std::unique_ptr<Expression>		m_condition; 
		std::unique_ptr<StatementList>  m_stmElse; 
	};
	std::unique_ptr<Expression>		m_condition;
	std::unique_ptr<StatementList>  m_stmThen;
	std::vector<stELSEIF>			m_elseIf;
	std::unique_ptr<StatementList>  m_stmElse;
};

class AssignStatement : public Statement
{
public:
	AssignStatement(Variable* var, Expression* val) : m_variable(var), m_value(val) {}
protected:
	std::unique_ptr<Variable>	m_variable;
	std::unique_ptr<Expression>	m_value;
};

class CallParameter
{
public:
	CallParameter( QString name, Expression* value) : m_name(name), m_value(value) {}
	CallParameter( Expression* value) : m_value(value) {}
	CallParameter( CallParameter && src ) { m_name = src.m_name; m_value = std::move( src.m_value ); }
protected:
	QString						m_name;
	std::unique_ptr<Expression>	m_value;
};

// call a function
class Call : public Expression, public Statement
{
public:
	Call() {}

	void SetName( QString name ) { m_functionName = name; }
	void AddParameter( QString name, Expression* value ) { m_parameters.emplace_back( name, value ); }
	void AddParameter( Expression* value ) { m_parameters.emplace_back( value ); }
protected:
	QString						m_functionName;
	std::vector<CallParameter>	m_parameters;
};

class DeclParameter
{
public:
	DeclParameter( QString name, SimpleType type, bool bVariadic ) : m_name( name ), m_type( type ), m_bVariadic( bVariadic ) {}
protected:
	QString						m_name;
	SimpleType					m_type;
	bool						m_bVariadic;
};

class FunctionDecl
{
public:
	FunctionDecl(QString name) : m_functionName(name) {}
	void AddParameter( QString name, SimpleType type, bool bVariadic ) { m_parameters.emplace_back( name, type, bVariadic );  }
	void SetStatement( StatementList* body ) { m_body.reset( body ); }
protected:
	QString							m_functionName;
	std::vector<DeclParameter>		m_parameters;
	std::unique_ptr<StatementList>  m_body;
};

class ProgramModule
{
public:
	void DeclareFunction(FunctionDecl* function) { m_functionList.emplace_back(function);  }
private:
	std::vector<std::unique_ptr<FunctionDecl>> m_functionList;
};

class Program
{
public:
	void AddProgramModule(ProgramModule* module)		{ m_moduleList.emplace_back(module); }
	void AddErrorModule(const ModuleErrorList& module)	{ m_errorList.AddModule(module); }

	ErrorList getErrorList() const						{ return m_errorList; }

private:
	std::vector<std::unique_ptr<ProgramModule>> m_moduleList;
	ErrorList									m_errorList;
};

#endif // __PROGRAM_TREE__