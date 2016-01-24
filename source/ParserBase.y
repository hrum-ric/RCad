%extra_argument  { RCADParser& rclParser }
%token_prefix    TOKEN_

%include 
{
#include <assert.h>
#include "Parser.h"
}

%left AND.
%left OR.
%left EQ NE GT GE LT LE.
%left PLUS MINUS.
%left TIMES DIVIDE MODULO.
%right NOT.

program ::= function_list .

newline ::= NEWLINE.
newline ::= ENDFILE.

function_list ::= function_list function_decl.
function_list ::= function_list NEWLINE.
function_list ::= .

function_decl ::= KW_FUNCTION IDENTIFIER  parameters_decl newline statement_list KW_END newline.

parameters_decl ::= LEFT_PARENTHESIS parameters_decl_list RIGHT_PARENTHESIS .
parameters_decl ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS .
parameters_decl_list ::= parameters_decl_list COMMA parameter_decl.
parameters_decl_list ::= parameter_decl.
parameter_decl ::= IDENTIFIER opt_type.
parameter_decl ::= IDENTIFIER VARIADIC opt_type.

statement_list ::= statement_list statement newline.
statement_list ::= .

statement	::= var_declaration .
statement	::= statement_if .
statement	::= statement_for .
statement	::= statement_then .

statement_then	::= assignment .
statement_then	::= call .

var_declaration ::= KW_VAR IDENTIFIER opt_type EQ expression .
var_declaration ::= KW_VAR IDENTIFIER opt_type .

assignment ::= variable EQ expression .
assignment ::= variable EQ expression_if .

variable ::= IDENTIFIER.
variable ::= variable PERIOD IDENTIFIER.
variable ::= variable LEFT_BRACKET expression RIGHT_BRACKET.

expression_if			::= KW_IF expression KW_THEN expression KW_ELSE expression.

statement_if			::= KW_IF expression KW_THEN statement_then KW_ELSE statement_then newline.
statement_if			::= KW_IF expression KW_THEN statement_then newline.
statement_if			::= KW_IF expression KW_THEN newline statement_list statement_elseif_list statement_else KW_END newline.
statement_elseif_list	::= statement_elseif_list statement_elseif.
statement_elseif_list	::= .
statement_elseif		::= KW_ELSE KW_IF expression KW_THEN newline statement_list.
statement_else			::= KW_ELSE newline statement_list.

statement_for			::= KW_FOR IDENTIFIER KW_FROM expression KW_TO expression for_by KW_DO newline statement_list KW_END newline.
statement_for			::= KW_FOR IDENTIFIER KW_IN expression KW_DO newline statement_list KW_END newline.
for_by					::= KW_BY expression.
for_by					::= .

call ::= IDENTIFIER  parameters_call.
parameters_call ::= LEFT_PARENTHESIS parameters_call_list RIGHT_PARENTHESIS .
parameters_call ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS .
parameters_call_list ::= parameters_call_list COMMA parameter_call.
parameters_call_list ::= parameter_call.
parameter_call ::= expression.
parameter_call ::= expression_if.
parameter_call ::= IDENTIFIER COLON expression.

expression	::= literal.
expression	::= variable.
expression	::= call.
expression	::= LEFT_PARENTHESIS expression RIGHT_PARENTHESIS.
expression	::= LEFT_PARENTHESIS expression_if RIGHT_PARENTHESIS.
expression  ::= MINUS expression. 
expression  ::= expression OR|AND expression.
expression  ::= expression PLUS|MINUS expression.
expression  ::= expression TIMES|DIVIDE|MODULO expression.
expression  ::= expression EQ|NE|GT|GE|LT|LE expression.

literal ::= INTEGER|REAL|STRING|KW_TRUE|KW_FALSE.
type ::= KW_INTEGER|KW_NUMBER|KW_STRING|KW_VAR.

opt_type  ::= COLON type.
opt_type  ::= . 


