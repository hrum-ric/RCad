%name BaseParse
%extra_argument			{ RCADParser* pParser }
%token_prefix			TOKEN_
%nonterminal_prefix		RULES_
%token_type				{ TokenBase }

%include 
{
#include <assert.h>
#include "Parser.h"
#include "ProgramTree.h"
}

%syntax_error 
{
	yymajor;
	int expected[YYNOCODE]; 
	int* cur = expected;
    for (int i = 0; i < YYNOCODE; ++i) 
	{
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) { *cur=i; cur++; }
	}
	pParser->OnError(TOKEN, expected, cur-expected);
}
%stack_overflow 
{
	yypMinor;
    pParser->OnStackOverflow();
}

%left  KW_AND.
%left  KW_OR.
%left  EQ NE GT GE LT LE.
%left  PLUS MINUS.
%left  TIMES DIVIDE MODULO.
%right KW_NOT.
%left  PERIOD LEFT_BRACKET.

program ::= function_list .

newline ::= NEWLINE.
newline ::= ENDFILE.

function_list ::= function_list function_decl(F).														{ pParser->addFunction(F); }
function_list ::= function_list NEWLINE.
function_list ::= .

%type function_decl0 { FunctionDecl* }
%type function_decl1 { FunctionDecl* }
%type function_declN { FunctionDecl* }
%type function_decl  { FunctionDecl* }
%destructor function_decl0 { delete($$); }
%destructor function_decl1 { delete($$); }
%destructor function_declN { delete($$); }
%destructor function_decl { delete($$); }
function_decl0(F) ::= KW_FUNCTION IDENTIFIER(I) LEFT_PARENTHESIS.										{ F = new FunctionDecl(I.string()); }
function_decl1(F) ::= function_decl0(F0) IDENTIFIER(I) opt_type(T).										{ F=F0; F0->AddParameter(I.string(),T,false); }
function_decl1(F) ::= function_decl0(F0) IDENTIFIER(I) VARIADIC opt_type(T).							{ F=F0; F0->AddParameter(I.string(),T,true); }
function_declN(F) ::= function_decl1(F1).																{ F = F1; }
function_declN(F) ::= function_declN(FN) COMMA IDENTIFIER(I) opt_type(T).								{ F=FN; FN->AddParameter(I.string(),T,true); }
function_declN(F) ::= function_declN(FN) COMMA IDENTIFIER(I) VARIADIC opt_type(T).						{ F=FN; FN->AddParameter(I.string(),T,true); }
function_decl(F)  ::= function_decl0(F0) RIGHT_PARENTHESIS newline statement_list(S) KW_END newline.	{ F = F0; F0->SetStatement(S); }
function_decl(F)  ::= function_declN(FN) RIGHT_PARENTHESIS newline statement_list(S) KW_END newline.	{ F = FN; FN->SetStatement(S); }

%type statement_list { StatementList* }
%destructor statement_list { delete($$); }
statement_list(L) ::= statement_list(SL) statement(S) newline.				{ SL->append(S); L = SL; }
statement_list(L) ::= statement_list(SL) newline.							{ L = SL; }
statement_list(L) ::= statement_list(SL) error newline.						{ L = SL; }
statement_list(L) ::= .														{ L = new StatementList(); }

%type statement { Statement* }
%destructor statement { delete($$); }
statement(S)	::= var_declaration(DS) .									{ S = DS; }
statement(S)	::= statement_if(IS) .										{ S = IS; }
statement(S)	::= statement_for(FS) .										{ S = FS; }
statement(S)	::= statement_then(TS) .									{ S = TS; }

%type statement_then { Statement* }
%destructor statement_then { delete($$); }
statement_then(S)	::= assignment(A) .										{ S = A; }
statement_then(S)	::= call(C) .											{ S = C; }

%type var_declaration { Statement* }
%destructor var_declaration { delete($$); }
var_declaration(D) ::= KW_VAR IDENTIFIER(I) opt_type(T) EQ expression(E) .	{ D = new DeclarationStatement(I.string(),T,E); }
var_declaration(D) ::= KW_VAR IDENTIFIER(I) opt_type(T) .					{ D = new DeclarationStatement(I.string(),T); }

%type assignment { Statement* }
%destructor assignment { delete($$); }
assignment(A) ::= variable(V) EQ expression(E) .							{ A = new AssignStatement(V,E); }
assignment(A) ::= variable(V) EQ expression_if(E) .							{ A = new AssignStatement(V,E); }

%type variable { Variable* }
%destructor variable { delete($$); }
variable(V) ::= IDENTIFIER(I).												{ V = new NamedVariable(I.string()); }
variable(V) ::= variable(O) PERIOD IDENTIFIER(M).							{ V = new VariableMember(O,M.string()); }
variable(V) ::= variable(A) LEFT_BRACKET expression(I) RIGHT_BRACKET.		{ V = new VariableSubElement(A,I); }

%type statement_if { Statement* }
%destructor statement_if { delete($$); }
statement_if(S)			::= KW_IF expression(C) KW_THEN statement_then(T) KW_ELSE statement_then(E) newline.						{ S = new IfStatement(C,T,E); }
statement_if(S)			::= KW_IF expression(C) KW_THEN statement_then(T) newline.													{ S = new IfStatement(C,T); }

%type statement_if_then { IfBlocStatement* }
%destructor statement_if_then { delete($$); }
statement_if_then(S)		::= KW_IF expression(C) KW_THEN newline statement_list(T).													{ S = new IfBlocStatement(C,T); }
%type statement_if_then_elsif { IfBlocStatement* }
%destructor statement_if_then_elsif { delete($$); }
statement_if_then_elsif(S)	::= statement_if_then(S1).																					{ S = S1; }
statement_if_then_elsif(S)	::= statement_if_then_elsif(S1)	KW_ELSE KW_IF expression(C) KW_THEN newline statement_list(SE).				{ S = S1; S1->AddElseIf(C,SE); }
statement_if(S)				::= statement_if_then_elsif(S1) KW_ELSE newline statement_list(SE) KW_END newline.							{ S = S1; S1->AddElse(SE); }
statement_if(S)				::= statement_if_then_elsif(S1) KW_END newline.																{ S = S1; }

%type statement_for { Statement* }
%destructor statement_for { delete($$); }
statement_for(S)			::= KW_FOR IDENTIFIER(I) KW_FROM expression(F) KW_TO expression(T) for_by(B) KW_DO newline statement_list(SL) KW_END newline.	{ S = new ForCounterStatement(I.string(), F, T, B, SL); }
statement_for(S)			::= KW_FOR IDENTIFIER(I) KW_IN expression(E) KW_DO newline statement_list(SL) KW_END newline.									{ S = new ForInStatement(I.string(), E, SL); }
%type for_by { Expression* }
%destructor for_by { delete($$); }
for_by(B)					::= KW_BY expression(E).																						{ B = E; }
for_by(B)					::= .																											{ B = nullptr; }

%type call { Call* }
%destructor call { delete($$); }
call(C) ::= IDENTIFIER(F)  parameters_call(P).														{ P->SetName(F.string()); C = P; }
%type parameters_call { Call* }
%destructor parameters_call { delete($$); }
parameters_call(C) ::= LEFT_PARENTHESIS parameters_call_list(P) RIGHT_PARENTHESIS .					{ C = P; }
parameters_call(C) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS .											{ C = new Call(); }
%type parameters_call_list { Call* }
%destructor parameters_call_list { delete($$); }
parameters_call_list(L) ::= parameters_call_list(OL) COMMA IDENTIFIER(N) COLON parameter_call(P).	{ L = OL;			L->AddParameter(N.string(), P); }
parameters_call_list(L) ::= parameters_call_list(OL) COMMA parameter_call(P).						{ L = OL;			L->AddParameter(P); }
parameters_call_list(L) ::= IDENTIFIER(N) COLON parameter_call(P).									{ L = new Call();	L->AddParameter(N.string(), P); }
parameters_call_list(L) ::= parameter_call(P).														{ L = new Call();	L->AddParameter(P); }
%type parameter_call { Expression* }
%destructor parameter_call { delete($$); }
parameter_call(P) ::= expression(E).																{ P = E; }
parameter_call(P) ::= expression_if(E).																{ P = E; }


%type expression { Expression* }
%destructor expression { delete($$); }
expression(E) ::= literal(L).																		{ E = L; }
expression(E) ::= IDENTIFIER(I).																	{ E = new NamedVariable(I.string()); }												
expression(E) ::= expression(O) PERIOD IDENTIFIER(M).												{ E = new ExpressionMember(O,M.string()); }	
expression(E) ::= expression(A) LEFT_BRACKET expression(I) RIGHT_BRACKET.							{ E = new ExpressionSubElement(A,I); }
expression(E) ::= call(C).																			{ E = C; }
expression(E) ::= LEFT_PARENTHESIS expression(S) RIGHT_PARENTHESIS.									{ E = S; }
expression(E) ::= LEFT_PARENTHESIS expression_if(S) RIGHT_PARENTHESIS.								{ E = S; }
expression(E) ::= KW_NOT expression(A).																{ E = new NotExpression(A); } 
expression(E) ::= MINUS expression(A). [KW_NOT] 													{ E = new MinusExpression(A); }
expression(E) ::= expression(A) KW_OR expression(B).												{ E = new OrExpression(A,B); }
expression(E) ::= expression(A) KW_AND expression(B).												{ E = new AndExpression(A,B); }
expression(E) ::= expression(A) PLUS expression(B).													{ E = new AddExpression(A,B); }
expression(E) ::= expression(A) MINUS expression(B).												{ E = new SubstractExpression(A,B); }	
expression(E) ::= expression(A) TIMES expression(B).												{ E = new MultiplyExpression(A,B); }
expression(E) ::= expression(A) DIVIDE expression(B).												{ E = new DivideExpression(A,B); }
expression(E) ::= expression(A) MODULO expression(B).												{ E = new ModuloExpression(A,B); }
expression(E) ::= expression(A) EQ expression(B).													{ E = new EqualCompare(A,B); }
expression(E) ::= expression(A) NE expression(B).													{ E = new NotEqualCompare(A,B); }
expression(E) ::= expression(A) GT expression(B).													{ E = new GreaterCompare(A,B); }
expression(E) ::= expression(A) GE expression(B).													{ E = new GreaterEqualCompare(A,B); }
expression(E) ::= expression(A) LT expression(B).													{ E = new LessCompare(A,B); }
expression(E) ::= expression(A) LE expression(B).													{ E = new LessEqualCompare(A,B); }

%type expression_if { Expression* }
%destructor expression_if { delete($$); }
expression_if(E)	::= KW_IF expression(C) KW_THEN expression(T) KW_ELSE expression(F).			{ E = new IfExpression(C,T,F); }

%type literal { Expression* }
%destructor literal { delete($$); }
literal(E) ::= INTEGER(L).																			{ E = new Literal(L.integer()); }
literal(E) ::= REAL(L).																				{ E = new Literal(L.number()); }
literal(E) ::= STRING(L).																			{ E = new Literal(L.string()); }
literal(E) ::= KW_TRUE.																				{ E = new Literal(true); }
literal(E) ::= KW_FALSE.																			{ E = new Literal(false); }

%type type { SimpleType }
type(T) ::= KW_INTEGER.																				{ T=SimpleType::eInteger; }
type(T) ::= KW_NUMBER.																				{ T=SimpleType::eNumber; }
type(T) ::= KW_STRING.																				{ T=SimpleType::eString; }
type(T) ::= KW_VAR.																					{ T=SimpleType::eUndef; }

%type opt_type { SimpleType }
opt_type(T)  ::= COLON type(TS).																	{ T=TS; }
opt_type(T)  ::= .																					{ T=SimpleType::eUndef; }


