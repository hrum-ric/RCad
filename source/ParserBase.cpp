/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
#line 8 "..\\source\\ParserBase.y"

#include <assert.h>
#include "Parser.h"
#include "ProgramTree.h"
#line 33 "..\\source\\ParserBase.cpp"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    BaseParseTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is BaseParseTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    BaseParseARG_SDECL     A static variable declaration for the %extra_argument
**    BaseParseARG_PDECL     A parameter declaration for the %extra_argument
**    BaseParseARG_STORE     Code to store %extra_argument into yypParser
**    BaseParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 75
#define YYACTIONTYPE unsigned short int
#define BaseParseTOKENTYPE  TokenBase 
typedef union {
  int yyinit;
  BaseParseTOKENTYPE yy0;
  Expression* yy7;
  Statement* yy56;
  Variable* yy57;
  SimpleType yy81;
  StatementList* yy92;
  Call* yy95;
  IfBlocStatement* yy111;
  FunctionDecl* yy137;
  int yy149;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define BaseParseARG_SDECL  RCADParser* pParser ;
#define BaseParseARG_PDECL , RCADParser* pParser 
#define BaseParseARG_FETCH  RCADParser* pParser  = yypParser->pParser 
#define BaseParseARG_STORE yypParser->pParser  = pParser 
#define YYERRORSYMBOL 46
#define YYERRSYMDT yy149
#define YYNSTATE             123
#define YYNRULE              85
#define YY_MAX_SHIFT         122
#define YY_MIN_SHIFTREDUCE   178
#define YY_MAX_SHIFTREDUCE   262
#define YY_MIN_REDUCE        263
#define YY_MAX_REDUCE        347
#define YY_ERROR_ACTION      348
#define YY_ACCEPT_ACTION     349
#define YY_NO_ACTION         350
/************* End control #defines *******************************************/

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE

**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (583)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    40,   41,   34,   33,   32,   31,   30,   29,   39,   38,
 /*    10 */    37,   36,   35,    8,  118,   42,   40,   41,   34,   33,
 /*    20 */    32,   31,   30,   29,   39,   38,   37,   36,   35,    9,
 /*    30 */   118,   42,   67,  179,  180,   18,   67,  181,  112,  187,
 /*    40 */   103,   48,  120,  114,   22,  179,  180,  115,   67,   54,
 /*    50 */   261,   40,   41,   34,   33,   32,   31,   30,   29,   39,
 /*    60 */    38,   37,   36,   35,    8,  118,   42,   40,   41,   34,
 /*    70 */    33,   32,   31,   30,   29,   39,   38,   37,   36,   35,
 /*    80 */    11,  118,   42,   19,   39,   38,   37,   36,   35,   72,
 /*    90 */   118,   42,   58,  200,  201,   57,  102,   40,   41,   34,
 /*   100 */    33,   32,   31,   30,   29,   39,   38,   37,   36,   35,
 /*   110 */    20,  118,   42,   17,  233,   90,  210,  228,  349,   15,
 /*   120 */   179,  180,  106,  226,  229,   12,  191,   40,   41,   34,
 /*   130 */    33,   32,   31,   30,   29,   39,   38,   37,   36,   35,
 /*   140 */    71,  118,   42,   37,   36,   35,  188,  118,   42,   51,
 /*   150 */   215,  113,   49,  208,   46,   40,   41,   34,   33,   32,
 /*   160 */    31,   30,   29,   39,   38,   37,   36,   35,  214,  118,
 /*   170 */    42,  217,  233,   88,   73,  205,   10,  221,  118,   42,
 /*   180 */   216,  232,  229,   40,   41,   34,   33,   32,   31,   30,
 /*   190 */    29,   39,   38,   37,   36,   35,   74,  118,   42,   40,
 /*   200 */    41,   34,   33,   32,   31,   30,   29,   39,   38,   37,
 /*   210 */    36,   35,   24,  118,   42,   75,   65,  209,  233,   86,
 /*   220 */    64,  117,  194,  192,  220,  190,   77,   25,  229,   40,
 /*   230 */    41,   34,   33,   32,   31,   30,   29,   39,   38,   37,
 /*   240 */    36,   35,  185,  118,   42,   45,  189,    8,  119,  186,
 /*   250 */    50,   56,  234,  104,   40,   41,   34,   33,   32,   31,
 /*   260 */    30,   29,   39,   38,   37,   36,   35,    1,  118,   42,
 /*   270 */     2,   41,   34,   33,   32,   31,   30,   29,   39,   38,
 /*   280 */    37,   36,   35,  207,  118,   42,   34,   33,   32,   31,
 /*   290 */    30,   29,   39,   38,   37,   36,   35,    3,  118,   42,
 /*   300 */     4,   61,    6,    7,  193,  179,  180,  260,  235,  179,
 /*   310 */   180,   62,  196,  197,  198,  199,  200,  201,   16,  102,
 /*   320 */    21,  212,  105,   27,  257,  258,  259,   28,   27,   66,
 /*   330 */    13,    5,   28,   60,   14,  233,   78,  222,   68,   14,
 /*   340 */   231,   26,  116,   23,   45,  229,   26,   43,   70,   69,
 /*   350 */   121,  252,  253,  254,  255,  256,  252,  253,  254,  255,
 /*   360 */   256,  265,   27,   45,  233,   87,   28,   27,  263,  184,
 /*   370 */   265,   28,   59,   14,  229,  265,  265,   68,   14,  265,
 /*   380 */    26,  265,  265,  265,  265,  182,  265,  122,  265,   76,
 /*   390 */   252,  253,  254,  255,  256,  252,  253,  254,  255,  256,
 /*   400 */    47,  200,  201,  265,  102,  265,  265,  265,  179,  180,
 /*   410 */   265,   67,  179,  180,  265,   67,   52,  120,  265,   22,
 /*   420 */    53,  120,  115,   22,  265,  265,  115,  179,  180,  265,
 /*   430 */    67,  179,  180,  265,   67,   55,  120,  265,   22,   63,
 /*   440 */   120,  115,   22,  233,   44,  115,  179,  180,  265,   67,
 /*   450 */   265,  265,  265,  229,  265,  120,  265,   22,  233,   90,
 /*   460 */   115,  228,  233,   90,  265,  228,  265,  223,  229,  265,
 /*   470 */   265,  224,  229,  265,  233,   90,  265,  228,  233,   79,
 /*   480 */   233,   80,  265,  225,  229,  233,   81,  265,  229,  265,
 /*   490 */   229,  265,  265,  233,   82,  229,  265,  233,   89,  233,
 /*   500 */    84,  233,   85,  229,  233,  107,  265,  229,  265,  229,
 /*   510 */   265,  229,  233,  108,  229,  265,  233,   94,  233,   95,
 /*   520 */   233,   96,  229,  233,   97,  265,  229,  265,  229,  265,
 /*   530 */   229,  233,   98,  229,  233,   99,  265,  233,  109,  265,
 /*   540 */   265,  229,  233,  110,  229,  265,  265,  229,  233,  111,
 /*   550 */   265,  265,  229,  233,  100,  265,  233,  101,  229,  265,
 /*   560 */   233,   92,  265,  229,  233,   93,  229,  233,   83,  265,
 /*   570 */   229,  265,  233,   91,  229,  265,  265,  229,  265,  265,
 /*   580 */   265,  265,  229,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*    10 */    11,   12,   13,   21,   15,   16,    1,    2,    3,    4,
 /*    20 */     5,    6,    7,    8,    9,   10,   11,   12,   13,   37,
 /*    30 */    15,   16,   20,   17,   18,   36,   20,   50,   51,   52,
 /*    40 */    53,   25,   26,   68,   28,   17,   18,   31,   20,   34,
 /*    50 */    73,    1,    2,    3,    4,    5,    6,    7,    8,    9,
 /*    60 */    10,   11,   12,   13,   21,   15,   16,    1,    2,    3,
 /*    70 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*    80 */    37,   15,   16,   33,    9,   10,   11,   12,   13,   49,
 /*    90 */    15,   16,   60,   61,   62,   29,   64,    1,    2,    3,
 /*   100 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   110 */    32,   15,   16,   35,   62,   63,   49,   65,   47,   48,
 /*   120 */    17,   18,   70,   71,   72,   29,   49,    1,    2,    3,
 /*   130 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   140 */    49,   15,   16,   11,   12,   13,   54,   15,   16,   25,
 /*   150 */    49,   23,   24,   27,   30,    1,    2,    3,    4,    5,
 /*   160 */     6,    7,    8,    9,   10,   11,   12,   13,   49,   15,
 /*   170 */    16,   49,   62,   63,   49,   65,   23,   24,   15,   16,
 /*   180 */    49,   27,   72,    1,    2,    3,    4,    5,    6,    7,
 /*   190 */     8,    9,   10,   11,   12,   13,   49,   15,   16,    1,
 /*   200 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   210 */    12,   13,   30,   15,   16,   49,   20,   49,   62,   63,
 /*   220 */    24,   65,   49,   49,   69,   49,   49,   29,   72,    1,
 /*   230 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   240 */    12,   13,   54,   15,   16,   37,   54,   21,   54,   54,
 /*   250 */    20,   34,   24,   20,    1,    2,    3,    4,    5,    6,
 /*   260 */     7,    8,    9,   10,   11,   12,   13,   55,   15,   16,
 /*   270 */    55,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   280 */    11,   12,   13,   20,   15,   16,    3,    4,    5,    6,
 /*   290 */     7,    8,    9,   10,   11,   12,   13,   55,   15,   16,
 /*   300 */    55,   46,   55,   55,   49,   17,   18,   26,   24,   17,
 /*   310 */    18,   56,   57,   58,   59,   60,   61,   62,   30,   64,
 /*   320 */    28,   66,   67,   10,   43,   44,   45,   14,   10,   22,
 /*   330 */     3,   55,   14,   20,   21,   62,   63,   24,   20,   21,
 /*   340 */    20,   28,   15,   16,   37,   72,   28,    3,   22,   20,
 /*   350 */    20,   38,   39,   40,   41,   42,   38,   39,   40,   41,
 /*   360 */    42,   74,   10,   37,   62,   63,   14,   10,    0,   21,
 /*   370 */    74,   14,   20,   21,   72,   74,   74,   20,   21,   74,
 /*   380 */    28,   74,   74,   74,   74,   17,   74,   19,   74,   49,
 /*   390 */    38,   39,   40,   41,   42,   38,   39,   40,   41,   42,
 /*   400 */    60,   61,   62,   74,   64,   74,   74,   74,   17,   18,
 /*   410 */    74,   20,   17,   18,   74,   20,   25,   26,   74,   28,
 /*   420 */    25,   26,   31,   28,   74,   74,   31,   17,   18,   74,
 /*   430 */    20,   17,   18,   74,   20,   25,   26,   74,   28,   25,
 /*   440 */    26,   31,   28,   62,   63,   31,   17,   18,   74,   20,
 /*   450 */    74,   74,   74,   72,   74,   26,   74,   28,   62,   63,
 /*   460 */    31,   65,   62,   63,   74,   65,   74,   71,   72,   74,
 /*   470 */    74,   71,   72,   74,   62,   63,   74,   65,   62,   63,
 /*   480 */    62,   63,   74,   71,   72,   62,   63,   74,   72,   74,
 /*   490 */    72,   74,   74,   62,   63,   72,   74,   62,   63,   62,
 /*   500 */    63,   62,   63,   72,   62,   63,   74,   72,   74,   72,
 /*   510 */    74,   72,   62,   63,   72,   74,   62,   63,   62,   63,
 /*   520 */    62,   63,   72,   62,   63,   74,   72,   74,   72,   74,
 /*   530 */    72,   62,   63,   72,   62,   63,   74,   62,   63,   74,
 /*   540 */    74,   72,   62,   63,   72,   74,   74,   72,   62,   63,
 /*   550 */    74,   74,   72,   62,   63,   74,   62,   63,   72,   74,
 /*   560 */    62,   63,   74,   72,   62,   63,   72,   62,   63,   74,
 /*   570 */    72,   74,   62,   63,   72,   74,   74,   72,   74,   74,
 /*   580 */    74,   74,   72,
};
#define YY_SHIFT_USE_DFLT (-9)
#define YY_SHIFT_COUNT (122)
#define YY_SHIFT_MIN   (-8)
#define YY_SHIFT_MAX   (429)
static const short yy_shift_ofst[] = {
 /*     0 */    -9,   16,  391,  395,  410,  414,  429,  429,  313,  318,
 /*    10 */   352,  318,   28,  318,  318,  368,   12,  357,  357,  357,
 /*    20 */   357,  357,  357,  357,  357,  357,  357,  357,  357,  357,
 /*    30 */   357,  357,  357,  357,  357,  357,  357,  357,  357,  357,
 /*    40 */   357,  357,  357,  357,   -1,  281,  292,  288,  103,  103,
 /*    50 */   307,  103,  103,  103,  103,  103,  103,  103,  103,   -8,
 /*    60 */    43,  103,  103,  103,  103,  326,  208,  226,  226,  208,
 /*    70 */   208,   -9,   -9,   -9,   -9,   -9,   -9,   -9,   15,   50,
 /*    80 */    66,   96,  126,  154,  182,  198,  228,  253,  253,  253,
 /*    90 */   253,  253,  269,  283,   75,   75,   75,   75,   75,   75,
 /*   100 */   132,  132,  327,  128,   78,  124,  153,  163,  163,  163,
 /*   110 */   163,  163,  196,  230,  217,  233,  263,  284,  320,  344,
 /*   120 */   329,  348,  330,
};
#define YY_REDUCE_USE_DFLT (-26)
#define YY_REDUCE_COUNT (77)
#define YY_REDUCE_MIN   (-25)
#define YY_REDUCE_MAX   (510)
static const short yy_reduce_ofst[] = {
 /*     0 */    71,  255,  255,  255,  255,  255,  255,  255,   52,  396,
 /*    10 */   400,  412,  340,  110,  156,  -13,   32,  273,  302,  381,
 /*    20 */   416,  418,  423,  431,  435,  437,  439,  442,  450,  454,
 /*    30 */   456,  458,  461,  469,  472,  475,  480,  486,  491,  494,
 /*    40 */   498,  502,  505,  510,  -25,  -23,   40,   67,   77,   91,
 /*    50 */    92,  101,  119,  122,  125,  131,  147,  166,  168,  155,
 /*    60 */   155,  173,  174,  176,  177,  188,  192,  155,  155,  194,
 /*    70 */   195,  212,  215,  242,  245,  247,  248,  276,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   268,  348,  348,  348,  348,  348,  298,  296,  348,  348,
 /*    10 */   348,  348,  348,  348,  348,  348,  348,  348,  348,  348,
 /*    20 */   348,  348,  348,  348,  348,  348,  348,  348,  348,  348,
 /*    30 */   348,  348,  348,  348,  348,  348,  348,  348,  348,  348,
 /*    40 */   348,  348,  348,  348,  304,  348,  348,  348,  348,  348,
 /*    50 */   347,  348,  348,  348,  348,  348,  348,  348,  348,  315,
 /*    60 */   315,  348,  348,  348,  348,  347,  347,  291,  315,  347,
 /*    70 */   347,  280,  280,  280,  280,  280,  280,  280,  348,  348,
 /*    80 */   348,  348,  348,  348,  348,  348,  348,  303,  289,  336,
 /*    90 */   312,  287,  324,  323,  335,  334,  333,  332,  331,  330,
 /*   100 */   326,  325,  348,  348,  348,  348,  348,  322,  321,  329,
 /*   110 */   328,  327,  348,  348,  348,  348,  348,  348,  348,  288,
 /*   120 */   348,  348,  348,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  BaseParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <QLoggingCategory>
static const char *yyTracePrompt = 0;
static const QLoggingCategory* yyTraceCategory = nullptr;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void BaseParseTrace( const QLoggingCategory * pclTraceCategory, const char *zTracePrompt )
{
	yyTraceCategory = pclTraceCategory;
	yyTracePrompt = zTracePrompt;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "KW_AND",        "KW_OR",         "EQ",          
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "PLUS",          "MINUS",         "TIMES",       
  "DIVIDE",        "MODULO",        "KW_NOT",        "PERIOD",      
  "LEFT_BRACKET",  "NEWLINE",       "ENDFILE",       "KW_FUNCTION", 
  "IDENTIFIER",    "LEFT_PARENTHESIS",  "VARIADIC",      "COMMA",       
  "RIGHT_PARENTHESIS",  "KW_END",        "KW_VAR",        "RIGHT_BRACKET",
  "KW_IF",         "KW_THEN",       "KW_ELSE",       "KW_FOR",      
  "KW_FROM",       "KW_TO",         "KW_DO",         "KW_IN",       
  "KW_BY",         "COLON",         "INTEGER",       "REAL",        
  "STRING",        "KW_TRUE",       "KW_FALSE",      "KW_INTEGER",  
  "KW_NUMBER",     "KW_STRING",     "error",         "program",     
  "function_list",  "newline",       "function_decl",  "function_decl0",
  "function_decl1",  "function_declN",  "opt_type",      "statement_list",
  "statement",     "var_declaration",  "statement_if",  "statement_for",
  "statement_then",  "assignment",    "call",          "expression",  
  "variable",      "expression_if",  "statement_if_then",  "statement_if_then_elsif",
  "for_by",        "parameters_call",  "parameters_call_list",  "parameter_call",
  "literal",       "type",        
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= function_list",
 /*   1 */ "newline ::= NEWLINE",
 /*   2 */ "newline ::= ENDFILE",
 /*   3 */ "function_list ::= function_list function_decl",
 /*   4 */ "function_list ::= function_list NEWLINE",
 /*   5 */ "function_list ::=",
 /*   6 */ "function_decl0 ::= KW_FUNCTION IDENTIFIER LEFT_PARENTHESIS",
 /*   7 */ "function_decl1 ::= function_decl0 IDENTIFIER opt_type",
 /*   8 */ "function_decl1 ::= function_decl0 IDENTIFIER VARIADIC opt_type",
 /*   9 */ "function_declN ::= function_decl1",
 /*  10 */ "function_declN ::= function_declN COMMA IDENTIFIER opt_type",
 /*  11 */ "function_declN ::= function_declN COMMA IDENTIFIER VARIADIC opt_type",
 /*  12 */ "function_decl ::= function_decl0 RIGHT_PARENTHESIS newline statement_list KW_END newline",
 /*  13 */ "function_decl ::= function_declN RIGHT_PARENTHESIS newline statement_list KW_END newline",
 /*  14 */ "statement_list ::= statement_list statement newline",
 /*  15 */ "statement_list ::= statement_list newline",
 /*  16 */ "statement_list ::= statement_list error newline",
 /*  17 */ "statement_list ::=",
 /*  18 */ "statement ::= var_declaration",
 /*  19 */ "statement ::= statement_if",
 /*  20 */ "statement ::= statement_for",
 /*  21 */ "statement ::= statement_then",
 /*  22 */ "statement_then ::= assignment",
 /*  23 */ "statement_then ::= call",
 /*  24 */ "var_declaration ::= KW_VAR IDENTIFIER opt_type EQ expression",
 /*  25 */ "var_declaration ::= KW_VAR IDENTIFIER opt_type",
 /*  26 */ "assignment ::= variable EQ expression",
 /*  27 */ "assignment ::= variable EQ expression_if",
 /*  28 */ "variable ::= IDENTIFIER",
 /*  29 */ "variable ::= variable PERIOD IDENTIFIER",
 /*  30 */ "variable ::= variable LEFT_BRACKET expression RIGHT_BRACKET",
 /*  31 */ "statement_if ::= KW_IF expression KW_THEN statement_then KW_ELSE statement_then newline",
 /*  32 */ "statement_if ::= KW_IF expression KW_THEN statement_then newline",
 /*  33 */ "statement_if_then ::= KW_IF expression KW_THEN newline statement_list",
 /*  34 */ "statement_if_then_elsif ::= statement_if_then",
 /*  35 */ "statement_if_then_elsif ::= statement_if_then_elsif KW_ELSE KW_IF expression KW_THEN newline statement_list",
 /*  36 */ "statement_if ::= statement_if_then_elsif KW_ELSE newline statement_list KW_END newline",
 /*  37 */ "statement_if ::= statement_if_then_elsif KW_END newline",
 /*  38 */ "statement_for ::= KW_FOR IDENTIFIER KW_FROM expression KW_TO expression for_by KW_DO newline statement_list KW_END newline",
 /*  39 */ "statement_for ::= KW_FOR IDENTIFIER KW_IN expression KW_DO newline statement_list KW_END newline",
 /*  40 */ "for_by ::= KW_BY expression",
 /*  41 */ "for_by ::=",
 /*  42 */ "call ::= IDENTIFIER parameters_call",
 /*  43 */ "parameters_call ::= LEFT_PARENTHESIS parameters_call_list RIGHT_PARENTHESIS",
 /*  44 */ "parameters_call ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS",
 /*  45 */ "parameters_call_list ::= parameters_call_list COMMA IDENTIFIER COLON parameter_call",
 /*  46 */ "parameters_call_list ::= parameters_call_list COMMA parameter_call",
 /*  47 */ "parameters_call_list ::= IDENTIFIER COLON parameter_call",
 /*  48 */ "parameters_call_list ::= parameter_call",
 /*  49 */ "parameter_call ::= expression",
 /*  50 */ "parameter_call ::= expression_if",
 /*  51 */ "expression ::= literal",
 /*  52 */ "expression ::= IDENTIFIER",
 /*  53 */ "expression ::= expression PERIOD IDENTIFIER",
 /*  54 */ "expression ::= expression LEFT_BRACKET expression RIGHT_BRACKET",
 /*  55 */ "expression ::= call",
 /*  56 */ "expression ::= LEFT_PARENTHESIS expression RIGHT_PARENTHESIS",
 /*  57 */ "expression ::= LEFT_PARENTHESIS expression_if RIGHT_PARENTHESIS",
 /*  58 */ "expression ::= KW_NOT expression",
 /*  59 */ "expression ::= MINUS expression",
 /*  60 */ "expression ::= expression KW_OR expression",
 /*  61 */ "expression ::= expression KW_AND expression",
 /*  62 */ "expression ::= expression PLUS expression",
 /*  63 */ "expression ::= expression MINUS expression",
 /*  64 */ "expression ::= expression TIMES expression",
 /*  65 */ "expression ::= expression DIVIDE expression",
 /*  66 */ "expression ::= expression MODULO expression",
 /*  67 */ "expression ::= expression EQ expression",
 /*  68 */ "expression ::= expression NE expression",
 /*  69 */ "expression ::= expression GT expression",
 /*  70 */ "expression ::= expression GE expression",
 /*  71 */ "expression ::= expression LT expression",
 /*  72 */ "expression ::= expression LE expression",
 /*  73 */ "expression_if ::= KW_IF expression KW_THEN expression KW_ELSE expression",
 /*  74 */ "literal ::= INTEGER",
 /*  75 */ "literal ::= REAL",
 /*  76 */ "literal ::= STRING",
 /*  77 */ "literal ::= KW_TRUE",
 /*  78 */ "literal ::= KW_FALSE",
 /*  79 */ "type ::= KW_INTEGER",
 /*  80 */ "type ::= KW_NUMBER",
 /*  81 */ "type ::= KW_STRING",
 /*  82 */ "type ::= KW_VAR",
 /*  83 */ "opt_type ::= COLON type",
 /*  84 */ "opt_type ::=",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTracePrompt ){
      qCDebug( (*yyTraceCategory),"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to BaseParseAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to BaseParse and BaseParseFree.
*/
void *BaseParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  BaseParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
    case 50: /* function_decl */
    case 51: /* function_decl0 */
    case 52: /* function_decl1 */
    case 53: /* function_declN */
{
#line 56 "..\\source\\ParserBase.y"
 delete((yypminor->yy137)); 
#line 677 "..\\source\\ParserBase.cpp"
}
      break;
    case 55: /* statement_list */
{
#line 67 "..\\source\\ParserBase.y"
 delete((yypminor->yy92)); 
#line 684 "..\\source\\ParserBase.cpp"
}
      break;
    case 56: /* statement */
    case 57: /* var_declaration */
    case 58: /* statement_if */
    case 59: /* statement_for */
    case 60: /* statement_then */
    case 61: /* assignment */
{
#line 74 "..\\source\\ParserBase.y"
 delete((yypminor->yy56)); 
#line 696 "..\\source\\ParserBase.cpp"
}
      break;
    case 62: /* call */
    case 69: /* parameters_call */
    case 70: /* parameters_call_list */
{
#line 126 "..\\source\\ParserBase.y"
 delete((yypminor->yy95)); 
#line 705 "..\\source\\ParserBase.cpp"
}
      break;
    case 63: /* expression */
    case 65: /* expression_if */
    case 68: /* for_by */
    case 71: /* parameter_call */
    case 72: /* literal */
{
#line 145 "..\\source\\ParserBase.y"
 delete((yypminor->yy7)); 
#line 716 "..\\source\\ParserBase.cpp"
}
      break;
    case 64: /* variable */
{
#line 96 "..\\source\\ParserBase.y"
 delete((yypminor->yy57)); 
#line 723 "..\\source\\ParserBase.cpp"
}
      break;
    case 66: /* statement_if_then */
    case 67: /* statement_if_then_elsif */
{
#line 107 "..\\source\\ParserBase.y"
 delete((yypminor->yy111)); 
#line 731 "..\\source\\ParserBase.cpp"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
  BaseParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yyidx>=0 );
  yytos = &pParser->yystack[pParser->yyidx--];
#ifndef NDEBUG
  if( yyTracePrompt ){
    qCDebug( (*yyTraceCategory),"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void BaseParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int BaseParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    if( i==YY_SHIFT_USE_DFLT ) return yy_default[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
      if( iLookAhead>0 ){
#ifdef YYFALLBACK
        YYCODETYPE iFallback;            /* Fallback token */
        if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
               && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
          if( yyTracePrompt ){
            qCDebug( (*yyTraceCategory), "%sFALLBACK %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
          }
#endif
          assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
          iLookAhead = iFallback;
          continue;
        }
#endif
#ifdef YYWILDCARD
        {
          int j = i - iLookAhead + YYWILDCARD;
          if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
            j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
            j<YY_ACTTAB_COUNT &&
#endif
            yy_lookahead[j]==YYWILDCARD
          ){
#ifndef NDEBUG
            if( yyTracePrompt ){
              qCDebug( (*yyTraceCategory), "%sWILDCARD %s => %s\n",
                 yyTracePrompt, yyTokenName[iLookAhead],
                 yyTokenName[YYWILDCARD]);
            }
#endif /* NDEBUG */
            return yy_action[j];
          }
        }
#endif /* YYWILDCARD */
      }
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   BaseParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTracePrompt ){
     qCDebug( (*yyTraceCategory),"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
#line 27 "..\\source\\ParserBase.y"

	yypMinor;
    pParser->OnStackOverflow();
#line 909 "..\\source\\ParserBase.cpp"
/******** End %stack_overflow code ********************************************/
   BaseParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTracePrompt ){
    if( yyNewState<YYNSTATE ){
      qCDebug( (*yyTraceCategory),"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major],
         yyNewState);
    }else{
      qCDebug( (*yyTraceCategory),"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 47, 1 },
  { 49, 1 },
  { 49, 1 },
  { 48, 2 },
  { 48, 2 },
  { 48, 0 },
  { 51, 3 },
  { 52, 3 },
  { 52, 4 },
  { 53, 1 },
  { 53, 4 },
  { 53, 5 },
  { 50, 6 },
  { 50, 6 },
  { 55, 3 },
  { 55, 2 },
  { 55, 3 },
  { 55, 0 },
  { 56, 1 },
  { 56, 1 },
  { 56, 1 },
  { 56, 1 },
  { 60, 1 },
  { 60, 1 },
  { 57, 5 },
  { 57, 3 },
  { 61, 3 },
  { 61, 3 },
  { 64, 1 },
  { 64, 3 },
  { 64, 4 },
  { 58, 7 },
  { 58, 5 },
  { 66, 5 },
  { 67, 1 },
  { 67, 7 },
  { 58, 6 },
  { 58, 3 },
  { 59, 12 },
  { 59, 9 },
  { 68, 2 },
  { 68, 0 },
  { 62, 2 },
  { 69, 3 },
  { 69, 2 },
  { 70, 5 },
  { 70, 3 },
  { 70, 3 },
  { 70, 1 },
  { 71, 1 },
  { 71, 1 },
  { 63, 1 },
  { 63, 1 },
  { 63, 3 },
  { 63, 4 },
  { 63, 1 },
  { 63, 3 },
  { 63, 3 },
  { 63, 2 },
  { 63, 2 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 65, 6 },
  { 72, 1 },
  { 72, 1 },
  { 72, 1 },
  { 72, 1 },
  { 72, 1 },
  { 73, 1 },
  { 73, 1 },
  { 73, 1 },
  { 73, 1 },
  { 54, 2 },
  { 54, 0 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  BaseParseARG_FETCH;
  BaseParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTracePrompt && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    qCDebug( (*yyTraceCategory), "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */
  yygotominor = yyzerominor;

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
      case 3: /* function_list ::= function_list function_decl */
#line 45 "..\\source\\ParserBase.y"
{ pParser->addFunction(yymsp[0].minor.yy137); }
#line 1106 "..\\source\\ParserBase.cpp"
        break;
      case 6: /* function_decl0 ::= KW_FUNCTION IDENTIFIER LEFT_PARENTHESIS */
#line 57 "..\\source\\ParserBase.y"
{ yygotominor.yy137 = new FunctionDecl(yymsp[-1].minor.yy0.string()); }
#line 1111 "..\\source\\ParserBase.cpp"
        break;
      case 7: /* function_decl1 ::= function_decl0 IDENTIFIER opt_type */
#line 58 "..\\source\\ParserBase.y"
{ yygotominor.yy137=yymsp[-2].minor.yy137; yymsp[-2].minor.yy137->AddParameter(yymsp[-1].minor.yy0.string(),yymsp[0].minor.yy81,false); }
#line 1116 "..\\source\\ParserBase.cpp"
        break;
      case 8: /* function_decl1 ::= function_decl0 IDENTIFIER VARIADIC opt_type */
#line 59 "..\\source\\ParserBase.y"
{ yygotominor.yy137=yymsp[-3].minor.yy137; yymsp[-3].minor.yy137->AddParameter(yymsp[-2].minor.yy0.string(),yymsp[0].minor.yy81,true); }
#line 1121 "..\\source\\ParserBase.cpp"
        break;
      case 9: /* function_declN ::= function_decl1 */
#line 60 "..\\source\\ParserBase.y"
{ yygotominor.yy137 = yymsp[0].minor.yy137; }
#line 1126 "..\\source\\ParserBase.cpp"
        break;
      case 10: /* function_declN ::= function_declN COMMA IDENTIFIER opt_type */
#line 61 "..\\source\\ParserBase.y"
{ yygotominor.yy137=yymsp[-3].minor.yy137; yymsp[-3].minor.yy137->AddParameter(yymsp[-1].minor.yy0.string(),yymsp[0].minor.yy81,true); }
#line 1131 "..\\source\\ParserBase.cpp"
        break;
      case 11: /* function_declN ::= function_declN COMMA IDENTIFIER VARIADIC opt_type */
#line 62 "..\\source\\ParserBase.y"
{ yygotominor.yy137=yymsp[-4].minor.yy137; yymsp[-4].minor.yy137->AddParameter(yymsp[-2].minor.yy0.string(),yymsp[0].minor.yy81,true); }
#line 1136 "..\\source\\ParserBase.cpp"
        break;
      case 12: /* function_decl ::= function_decl0 RIGHT_PARENTHESIS newline statement_list KW_END newline */
      case 13: /* function_decl ::= function_declN RIGHT_PARENTHESIS newline statement_list KW_END newline */ yytestcase(yyruleno==13);
#line 63 "..\\source\\ParserBase.y"
{ yygotominor.yy137 = yymsp[-5].minor.yy137; yymsp[-5].minor.yy137->SetStatement(yymsp[-2].minor.yy92); }
#line 1142 "..\\source\\ParserBase.cpp"
        break;
      case 14: /* statement_list ::= statement_list statement newline */
#line 68 "..\\source\\ParserBase.y"
{ yymsp[-2].minor.yy92->append(yymsp[-1].minor.yy56); yygotominor.yy92 = yymsp[-2].minor.yy92; }
#line 1147 "..\\source\\ParserBase.cpp"
        break;
      case 15: /* statement_list ::= statement_list newline */
#line 69 "..\\source\\ParserBase.y"
{ yygotominor.yy92 = yymsp[-1].minor.yy92; }
#line 1152 "..\\source\\ParserBase.cpp"
        break;
      case 16: /* statement_list ::= statement_list error newline */
#line 70 "..\\source\\ParserBase.y"
{ yygotominor.yy92 = yymsp[-2].minor.yy92; }
#line 1157 "..\\source\\ParserBase.cpp"
        break;
      case 17: /* statement_list ::= */
#line 71 "..\\source\\ParserBase.y"
{ yygotominor.yy92 = new StatementList(); }
#line 1162 "..\\source\\ParserBase.cpp"
        break;
      case 18: /* statement ::= var_declaration */
      case 19: /* statement ::= statement_if */ yytestcase(yyruleno==19);
      case 20: /* statement ::= statement_for */ yytestcase(yyruleno==20);
      case 21: /* statement ::= statement_then */ yytestcase(yyruleno==21);
      case 22: /* statement_then ::= assignment */ yytestcase(yyruleno==22);
#line 75 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = yymsp[0].minor.yy56; }
#line 1171 "..\\source\\ParserBase.cpp"
        break;
      case 23: /* statement_then ::= call */
#line 83 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = yymsp[0].minor.yy95; }
#line 1176 "..\\source\\ParserBase.cpp"
        break;
      case 24: /* var_declaration ::= KW_VAR IDENTIFIER opt_type EQ expression */
#line 87 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new DeclarationStatement(yymsp[-3].minor.yy0.string(),yymsp[-2].minor.yy81,yymsp[0].minor.yy7); }
#line 1181 "..\\source\\ParserBase.cpp"
        break;
      case 25: /* var_declaration ::= KW_VAR IDENTIFIER opt_type */
#line 88 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new DeclarationStatement(yymsp[-1].minor.yy0.string(),yymsp[0].minor.yy81); }
#line 1186 "..\\source\\ParserBase.cpp"
        break;
      case 26: /* assignment ::= variable EQ expression */
      case 27: /* assignment ::= variable EQ expression_if */ yytestcase(yyruleno==27);
#line 92 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new AssignStatement(yymsp[-2].minor.yy57,yymsp[0].minor.yy7); }
#line 1192 "..\\source\\ParserBase.cpp"
        break;
      case 28: /* variable ::= IDENTIFIER */
#line 97 "..\\source\\ParserBase.y"
{ yygotominor.yy57 = new NamedVariable(yymsp[0].minor.yy0.string()); }
#line 1197 "..\\source\\ParserBase.cpp"
        break;
      case 29: /* variable ::= variable PERIOD IDENTIFIER */
#line 98 "..\\source\\ParserBase.y"
{ yygotominor.yy57 = new VariableMember(yymsp[-2].minor.yy57,yymsp[0].minor.yy0.string()); }
#line 1202 "..\\source\\ParserBase.cpp"
        break;
      case 30: /* variable ::= variable LEFT_BRACKET expression RIGHT_BRACKET */
#line 99 "..\\source\\ParserBase.y"
{ yygotominor.yy57 = new VariableSubElement(yymsp[-3].minor.yy57,yymsp[-1].minor.yy7); }
#line 1207 "..\\source\\ParserBase.cpp"
        break;
      case 31: /* statement_if ::= KW_IF expression KW_THEN statement_then KW_ELSE statement_then newline */
#line 103 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new IfStatement(yymsp[-5].minor.yy7,yymsp[-3].minor.yy56,yymsp[-1].minor.yy56); }
#line 1212 "..\\source\\ParserBase.cpp"
        break;
      case 32: /* statement_if ::= KW_IF expression KW_THEN statement_then newline */
#line 104 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new IfStatement(yymsp[-3].minor.yy7,yymsp[-1].minor.yy56); }
#line 1217 "..\\source\\ParserBase.cpp"
        break;
      case 33: /* statement_if_then ::= KW_IF expression KW_THEN newline statement_list */
#line 108 "..\\source\\ParserBase.y"
{ yygotominor.yy111 = new IfBlocStatement(yymsp[-3].minor.yy7,yymsp[0].minor.yy92); }
#line 1222 "..\\source\\ParserBase.cpp"
        break;
      case 34: /* statement_if_then_elsif ::= statement_if_then */
#line 111 "..\\source\\ParserBase.y"
{ yygotominor.yy111 = yymsp[0].minor.yy111; }
#line 1227 "..\\source\\ParserBase.cpp"
        break;
      case 35: /* statement_if_then_elsif ::= statement_if_then_elsif KW_ELSE KW_IF expression KW_THEN newline statement_list */
#line 112 "..\\source\\ParserBase.y"
{ yygotominor.yy111 = yymsp[-6].minor.yy111; yymsp[-6].minor.yy111->AddElseIf(yymsp[-3].minor.yy7,yymsp[0].minor.yy92); }
#line 1232 "..\\source\\ParserBase.cpp"
        break;
      case 36: /* statement_if ::= statement_if_then_elsif KW_ELSE newline statement_list KW_END newline */
#line 113 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = yymsp[-5].minor.yy111; yymsp[-5].minor.yy111->AddElse(yymsp[-2].minor.yy92); }
#line 1237 "..\\source\\ParserBase.cpp"
        break;
      case 37: /* statement_if ::= statement_if_then_elsif KW_END newline */
#line 114 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = yymsp[-2].minor.yy111; }
#line 1242 "..\\source\\ParserBase.cpp"
        break;
      case 38: /* statement_for ::= KW_FOR IDENTIFIER KW_FROM expression KW_TO expression for_by KW_DO newline statement_list KW_END newline */
#line 118 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new ForCounterStatement(yymsp[-10].minor.yy0.string(), yymsp[-8].minor.yy7, yymsp[-6].minor.yy7, yymsp[-5].minor.yy7, yymsp[-2].minor.yy92); }
#line 1247 "..\\source\\ParserBase.cpp"
        break;
      case 39: /* statement_for ::= KW_FOR IDENTIFIER KW_IN expression KW_DO newline statement_list KW_END newline */
#line 119 "..\\source\\ParserBase.y"
{ yygotominor.yy56 = new ForInStatement(yymsp[-7].minor.yy0.string(), yymsp[-5].minor.yy7, yymsp[-2].minor.yy92); }
#line 1252 "..\\source\\ParserBase.cpp"
        break;
      case 40: /* for_by ::= KW_BY expression */
      case 49: /* parameter_call ::= expression */ yytestcase(yyruleno==49);
      case 50: /* parameter_call ::= expression_if */ yytestcase(yyruleno==50);
      case 51: /* expression ::= literal */ yytestcase(yyruleno==51);
#line 122 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = yymsp[0].minor.yy7; }
#line 1260 "..\\source\\ParserBase.cpp"
        break;
      case 41: /* for_by ::= */
#line 123 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = nullptr; }
#line 1265 "..\\source\\ParserBase.cpp"
        break;
      case 42: /* call ::= IDENTIFIER parameters_call */
#line 127 "..\\source\\ParserBase.y"
{ yymsp[0].minor.yy95->SetName(yymsp[-1].minor.yy0.string()); yygotominor.yy95 = yymsp[0].minor.yy95; }
#line 1270 "..\\source\\ParserBase.cpp"
        break;
      case 43: /* parameters_call ::= LEFT_PARENTHESIS parameters_call_list RIGHT_PARENTHESIS */
#line 130 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = yymsp[-1].minor.yy95; }
#line 1275 "..\\source\\ParserBase.cpp"
        break;
      case 44: /* parameters_call ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS */
#line 131 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = new Call(); }
#line 1280 "..\\source\\ParserBase.cpp"
        break;
      case 45: /* parameters_call_list ::= parameters_call_list COMMA IDENTIFIER COLON parameter_call */
#line 134 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = yymsp[-4].minor.yy95;			yygotominor.yy95->AddParameter(yymsp[-2].minor.yy0.string(), yymsp[0].minor.yy7); }
#line 1285 "..\\source\\ParserBase.cpp"
        break;
      case 46: /* parameters_call_list ::= parameters_call_list COMMA parameter_call */
#line 135 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = yymsp[-2].minor.yy95;			yygotominor.yy95->AddParameter(yymsp[0].minor.yy7); }
#line 1290 "..\\source\\ParserBase.cpp"
        break;
      case 47: /* parameters_call_list ::= IDENTIFIER COLON parameter_call */
#line 136 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = new Call();	yygotominor.yy95->AddParameter(yymsp[-2].minor.yy0.string(), yymsp[0].minor.yy7); }
#line 1295 "..\\source\\ParserBase.cpp"
        break;
      case 48: /* parameters_call_list ::= parameter_call */
#line 137 "..\\source\\ParserBase.y"
{ yygotominor.yy95 = new Call();	yygotominor.yy95->AddParameter(yymsp[0].minor.yy7); }
#line 1300 "..\\source\\ParserBase.cpp"
        break;
      case 52: /* expression ::= IDENTIFIER */
#line 147 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new NamedVariable(yymsp[0].minor.yy0.string()); }
#line 1305 "..\\source\\ParserBase.cpp"
        break;
      case 53: /* expression ::= expression PERIOD IDENTIFIER */
#line 148 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new ExpressionMember(yymsp[-2].minor.yy7,yymsp[0].minor.yy0.string()); }
#line 1310 "..\\source\\ParserBase.cpp"
        break;
      case 54: /* expression ::= expression LEFT_BRACKET expression RIGHT_BRACKET */
#line 149 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new ExpressionSubElement(yymsp[-3].minor.yy7,yymsp[-1].minor.yy7); }
#line 1315 "..\\source\\ParserBase.cpp"
        break;
      case 55: /* expression ::= call */
#line 150 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = yymsp[0].minor.yy95; }
#line 1320 "..\\source\\ParserBase.cpp"
        break;
      case 56: /* expression ::= LEFT_PARENTHESIS expression RIGHT_PARENTHESIS */
      case 57: /* expression ::= LEFT_PARENTHESIS expression_if RIGHT_PARENTHESIS */ yytestcase(yyruleno==57);
#line 151 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = yymsp[-1].minor.yy7; }
#line 1326 "..\\source\\ParserBase.cpp"
        break;
      case 58: /* expression ::= KW_NOT expression */
#line 153 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new NotExpression(yymsp[0].minor.yy7); }
#line 1331 "..\\source\\ParserBase.cpp"
        break;
      case 59: /* expression ::= MINUS expression */
#line 154 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new MinusExpression(yymsp[0].minor.yy7); }
#line 1336 "..\\source\\ParserBase.cpp"
        break;
      case 60: /* expression ::= expression KW_OR expression */
#line 155 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new OrExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1341 "..\\source\\ParserBase.cpp"
        break;
      case 61: /* expression ::= expression KW_AND expression */
#line 156 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new AndExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1346 "..\\source\\ParserBase.cpp"
        break;
      case 62: /* expression ::= expression PLUS expression */
#line 157 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new AddExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1351 "..\\source\\ParserBase.cpp"
        break;
      case 63: /* expression ::= expression MINUS expression */
#line 158 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new SubstractExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1356 "..\\source\\ParserBase.cpp"
        break;
      case 64: /* expression ::= expression TIMES expression */
#line 159 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new MultiplyExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1361 "..\\source\\ParserBase.cpp"
        break;
      case 65: /* expression ::= expression DIVIDE expression */
#line 160 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new DivideExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1366 "..\\source\\ParserBase.cpp"
        break;
      case 66: /* expression ::= expression MODULO expression */
#line 161 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new ModuloExpression(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1371 "..\\source\\ParserBase.cpp"
        break;
      case 67: /* expression ::= expression EQ expression */
#line 162 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new EqualCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1376 "..\\source\\ParserBase.cpp"
        break;
      case 68: /* expression ::= expression NE expression */
#line 163 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new NotEqualCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1381 "..\\source\\ParserBase.cpp"
        break;
      case 69: /* expression ::= expression GT expression */
#line 164 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new GreaterCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1386 "..\\source\\ParserBase.cpp"
        break;
      case 70: /* expression ::= expression GE expression */
#line 165 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new GreaterEqualCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1391 "..\\source\\ParserBase.cpp"
        break;
      case 71: /* expression ::= expression LT expression */
#line 166 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new LessCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1396 "..\\source\\ParserBase.cpp"
        break;
      case 72: /* expression ::= expression LE expression */
#line 167 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new LessEqualCompare(yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1401 "..\\source\\ParserBase.cpp"
        break;
      case 73: /* expression_if ::= KW_IF expression KW_THEN expression KW_ELSE expression */
#line 171 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new IfExpression(yymsp[-4].minor.yy7,yymsp[-2].minor.yy7,yymsp[0].minor.yy7); }
#line 1406 "..\\source\\ParserBase.cpp"
        break;
      case 74: /* literal ::= INTEGER */
#line 175 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new Literal(yymsp[0].minor.yy0.integer()); }
#line 1411 "..\\source\\ParserBase.cpp"
        break;
      case 75: /* literal ::= REAL */
#line 176 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new Literal(yymsp[0].minor.yy0.number()); }
#line 1416 "..\\source\\ParserBase.cpp"
        break;
      case 76: /* literal ::= STRING */
#line 177 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new Literal(yymsp[0].minor.yy0.string()); }
#line 1421 "..\\source\\ParserBase.cpp"
        break;
      case 77: /* literal ::= KW_TRUE */
#line 178 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new Literal(true); }
#line 1426 "..\\source\\ParserBase.cpp"
        break;
      case 78: /* literal ::= KW_FALSE */
#line 179 "..\\source\\ParserBase.y"
{ yygotominor.yy7 = new Literal(false); }
#line 1431 "..\\source\\ParserBase.cpp"
        break;
      case 79: /* type ::= KW_INTEGER */
#line 182 "..\\source\\ParserBase.y"
{ yygotominor.yy81=SimpleType::eInteger; }
#line 1436 "..\\source\\ParserBase.cpp"
        break;
      case 80: /* type ::= KW_NUMBER */
#line 183 "..\\source\\ParserBase.y"
{ yygotominor.yy81=SimpleType::eNumber; }
#line 1441 "..\\source\\ParserBase.cpp"
        break;
      case 81: /* type ::= KW_STRING */
#line 184 "..\\source\\ParserBase.y"
{ yygotominor.yy81=SimpleType::eString; }
#line 1446 "..\\source\\ParserBase.cpp"
        break;
      case 82: /* type ::= KW_VAR */
      case 84: /* opt_type ::= */ yytestcase(yyruleno==84);
#line 185 "..\\source\\ParserBase.y"
{ yygotominor.yy81=SimpleType::eUndef; }
#line 1452 "..\\source\\ParserBase.cpp"
        break;
      case 83: /* opt_type ::= COLON type */
#line 188 "..\\source\\ParserBase.y"
{ yygotominor.yy81=yymsp[0].minor.yy81; }
#line 1457 "..\\source\\ParserBase.cpp"
        break;
      default:
      /* (0) program ::= function_list */ yytestcase(yyruleno==0);
      /* (1) newline ::= NEWLINE */ yytestcase(yyruleno==1);
      /* (2) newline ::= ENDFILE */ yytestcase(yyruleno==2);
      /* (4) function_list ::= function_list NEWLINE */ yytestcase(yyruleno==4);
      /* (5) function_list ::= */ yytestcase(yyruleno==5);
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno>=0 && yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    /* If the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
      yyTraceShift(yypParser, yyact);
    }else{
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  BaseParseARG_FETCH;
#ifndef NDEBUG
  if( yyTracePrompt ){
    qCDebug( (*yyTraceCategory),"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  BaseParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  BaseParseARG_FETCH;
#define TOKEN (yyminor.yy0)
/************ Begin %syntax_error code ****************************************/
#line 15 "..\\source\\ParserBase.y"

	yymajor;
	int expected[YYNOCODE]; 
	int* cur = expected;
    for (int i = 0; i < YYNOCODE; ++i) 
	{
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) { *cur=i; cur++; }
	}
	pParser->OnError(TOKEN, expected, cur-expected);
#line 1539 "..\\source\\ParserBase.cpp"
/************ End %syntax_error code ******************************************/
  BaseParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  BaseParseARG_FETCH;
#ifndef NDEBUG
  if( yyTracePrompt ){
    qCDebug( (*yyTraceCategory),"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  BaseParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "BaseParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void BaseParse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  BaseParseTOKENTYPE yyminor       /* The value for the token */
  BaseParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
#ifndef NDEBUG
    if( yyTracePrompt ){
      qCDebug( (*yyTraceCategory),"%sInitialize. Empty stack. State 0\n",
              yyTracePrompt);
    }
#endif
  }
  yyminorunion.yy0 = yyminor;
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  BaseParseARG_STORE;

#ifndef NDEBUG
  if( yyTracePrompt ){
    qCDebug( (*yyTraceCategory),"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      if( yyact > YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTracePrompt ){
        qCDebug( (*yyTraceCategory),"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTracePrompt ){
          qCDebug( (*yyTraceCategory),"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
#ifndef NDEBUG
  if( yyTracePrompt ){
    int i;
    qCDebug( (*yyTraceCategory),"%sReturn. Stack=",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      qCDebug( (*yyTraceCategory),"%c%s", i==1 ? '[' : ' ', 
              yyTokenName[yypParser->yystack[i].major]);
    qCDebug( (*yyTraceCategory),"]\n");
  }
#endif
  return;
}
