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
#line 48 "fts5parse.y"

#include "fts5Int.h"
#include "fts5parse.h"

/*
** Disable all error recovery processing in the parser push-down
** automaton.
*/
#define YYNOERRORRECOVERY 1

/*
** Make yytestcase() the same as testcase()
*/
#define yytestcase(X) testcase(X)

/*
** Indicate that sqlite3ParserFree() will never be called with a null
** pointer.
*/
#define YYPARSEFREENOTNULL 1

/*
** Alternative datatype for the argument to the malloc() routine passed
** into sqlite3ParserAlloc().  The default is size_t.
*/
#define YYMALLOCARGTYPE  u64

#line 56 "fts5parse.c"
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
**    sqlite3Fts5ParserTOKENTYPE     is the data type used for minor type for terminal
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
**                       which is sqlite3Fts5ParserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    sqlite3Fts5ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3Fts5ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3Fts5ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3Fts5ParserARG_FETCH     Code to extract %extra_argument from yypParser
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
#define YYNOCODE 27
#define YYACTIONTYPE unsigned char
#define sqlite3Fts5ParserTOKENTYPE Fts5Token
typedef union {
  int yyinit;
  sqlite3Fts5ParserTOKENTYPE yy0;
  Fts5Colset* yy3;
  Fts5ExprPhrase* yy11;
  Fts5ExprNode* yy18;
  int yy20;
  Fts5ExprNearset* yy26;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define sqlite3Fts5ParserARG_SDECL Fts5Parse *pParse;
#define sqlite3Fts5ParserARG_PDECL ,Fts5Parse *pParse
#define sqlite3Fts5ParserARG_FETCH Fts5Parse *pParse = yypParser->pParse
#define sqlite3Fts5ParserARG_STORE yypParser->pParse = pParse
#define YYNSTATE             26
#define YYNRULE              24
#define YY_MAX_SHIFT         25
#define YY_MIN_SHIFTREDUCE   40
#define YY_MAX_SHIFTREDUCE   63
#define YY_MIN_REDUCE        64
#define YY_MAX_REDUCE        87
#define YY_ERROR_ACTION      88
#define YY_ACCEPT_ACTION     89
#define YY_NO_ACTION         90
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
#define YY_ACTTAB_COUNT (78)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    89,   15,   46,    5,   48,   24,   12,   19,   23,   14,
 /*    10 */    46,    5,   48,   24,   20,   21,   23,   43,   46,    5,
 /*    20 */    48,   24,    6,   18,   23,   17,   46,    5,   48,   24,
 /*    30 */    75,    7,   23,   25,   46,    5,   48,   24,   62,   47,
 /*    40 */    23,   48,   24,    7,   11,   23,    9,    3,    4,    2,
 /*    50 */    62,   50,   52,   44,   64,    3,    4,    2,   49,    4,
 /*    60 */     2,    1,   23,   11,   16,    9,   12,    2,   10,   61,
 /*    70 */    53,   59,   62,   60,   22,   13,   55,    8,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    15,   16,   17,   18,   19,   20,   10,   11,   23,   16,
 /*    10 */    17,   18,   19,   20,   23,   24,   23,   16,   17,   18,
 /*    20 */    19,   20,   22,   23,   23,   16,   17,   18,   19,   20,
 /*    30 */     5,    6,   23,   16,   17,   18,   19,   20,   13,   17,
 /*    40 */    23,   19,   20,    6,    8,   23,   10,    1,    2,    3,
 /*    50 */    13,    9,   10,    7,    0,    1,    2,    3,   19,    2,
 /*    60 */     3,    6,   23,    8,   21,   10,   10,    3,   10,   25,
 /*    70 */    10,   10,   13,   25,   12,   10,    7,    5,
};
#define YY_SHIFT_USE_DFLT (-5)
#define YY_SHIFT_COUNT (25)
#define YY_SHIFT_MIN   (-4)
#define YY_SHIFT_MAX   (72)
static const signed char yy_shift_ofst[] = {
 /*     0 */    55,   55,   55,   55,   55,   36,   -4,   56,   58,   25,
 /*    10 */    37,   60,   59,   59,   46,   54,   42,   57,   62,   61,
 /*    20 */    62,   69,   65,   62,   72,   64,
};
#define YY_REDUCE_USE_DFLT (-16)
#define YY_REDUCE_COUNT (13)
#define YY_REDUCE_MIN   (-15)
#define YY_REDUCE_MAX   (48)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -15,   -7,    1,    9,   17,   22,   -9,    0,   39,   44,
 /*    10 */    44,   43,   44,   48,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    88,   88,   88,   88,   88,   69,   82,   88,   88,   87,
 /*    10 */    87,   88,   87,   87,   88,   88,   88,   66,   80,   88,
 /*    20 */    81,   88,   88,   78,   88,   65,
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
  sqlite3Fts5ParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
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
void sqlite3Fts5ParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "OR",            "AND",           "NOT",         
  "TERM",          "COLON",         "LP",            "RP",          
  "LCP",           "RCP",           "STRING",        "COMMA",       
  "PLUS",          "STAR",          "error",         "input",       
  "expr",          "cnearset",      "exprlist",      "nearset",     
  "colset",        "colsetlist",    "nearphrases",   "phrase",      
  "neardist_opt",  "star_opt",    
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= expr",
 /*   1 */ "expr ::= expr AND expr",
 /*   2 */ "expr ::= expr OR expr",
 /*   3 */ "expr ::= expr NOT expr",
 /*   4 */ "expr ::= LP expr RP",
 /*   5 */ "expr ::= exprlist",
 /*   6 */ "exprlist ::= cnearset",
 /*   7 */ "exprlist ::= exprlist cnearset",
 /*   8 */ "cnearset ::= nearset",
 /*   9 */ "cnearset ::= colset COLON nearset",
 /*  10 */ "colset ::= LCP colsetlist RCP",
 /*  11 */ "colset ::= STRING",
 /*  12 */ "colsetlist ::= colsetlist STRING",
 /*  13 */ "colsetlist ::= STRING",
 /*  14 */ "nearset ::= phrase",
 /*  15 */ "nearset ::= STRING LP nearphrases neardist_opt RP",
 /*  16 */ "nearphrases ::= phrase",
 /*  17 */ "nearphrases ::= nearphrases phrase",
 /*  18 */ "neardist_opt ::=",
 /*  19 */ "neardist_opt ::= COMMA STRING",
 /*  20 */ "phrase ::= phrase PLUS STRING star_opt",
 /*  21 */ "phrase ::= STRING star_opt",
 /*  22 */ "star_opt ::= STAR",
 /*  23 */ "star_opt ::=",
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
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to sqlite3Fts5ParserAlloc() below.  This can be changed by
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
** to sqlite3Fts5Parser and sqlite3Fts5ParserFree.
*/
void *sqlite3Fts5ParserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
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
  sqlite3Fts5ParserARG_FETCH;
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
    case 15: /* input */
{
#line 84 "fts5parse.y"
 (void)pParse; 
#line 491 "fts5parse.c"
}
      break;
    case 16: /* expr */
    case 17: /* cnearset */
    case 18: /* exprlist */
{
#line 90 "fts5parse.y"
 sqlite3Fts5ParseNodeFree((yypminor->yy18)); 
#line 500 "fts5parse.c"
}
      break;
    case 19: /* nearset */
    case 22: /* nearphrases */
{
#line 138 "fts5parse.y"
 sqlite3Fts5ParseNearsetFree((yypminor->yy26)); 
#line 508 "fts5parse.c"
}
      break;
    case 20: /* colset */
    case 21: /* colsetlist */
{
#line 120 "fts5parse.y"
 sqlite3_free((yypminor->yy3)); 
#line 516 "fts5parse.c"
}
      break;
    case 23: /* phrase */
{
#line 169 "fts5parse.y"
 sqlite3Fts5ParsePhraseFree((yypminor->yy11)); 
#line 523 "fts5parse.c"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
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
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
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
void sqlite3Fts5ParserFree(
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
int sqlite3Fts5ParserStackPeak(void *p){
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
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
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
            if( yyTraceFILE ){
              fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
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
   sqlite3Fts5ParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
#line 36 "fts5parse.y"

  UNUSED_PARAM(yypMinor); /* Silence a compiler warning */
  sqlite3Fts5ParseError(pParse, "fts5: parser stack overflow");
#line 700 "fts5parse.c"
/******** End %stack_overflow code ********************************************/
   sqlite3Fts5ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
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
  { 15, 1 },
  { 16, 3 },
  { 16, 3 },
  { 16, 3 },
  { 16, 3 },
  { 16, 1 },
  { 18, 1 },
  { 18, 2 },
  { 17, 1 },
  { 17, 3 },
  { 20, 3 },
  { 20, 1 },
  { 21, 2 },
  { 21, 1 },
  { 19, 1 },
  { 19, 5 },
  { 22, 1 },
  { 22, 2 },
  { 24, 0 },
  { 24, 2 },
  { 23, 4 },
  { 23, 2 },
  { 25, 1 },
  { 25, 0 },
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
  sqlite3Fts5ParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
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
      case 0: /* input ::= expr */
#line 83 "fts5parse.y"
{ sqlite3Fts5ParseFinished(pParse, yymsp[0].minor.yy18); }
#line 835 "fts5parse.c"
        break;
      case 1: /* expr ::= expr AND expr */
#line 93 "fts5parse.y"
{
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_AND, yymsp[-2].minor.yy18, yymsp[0].minor.yy18, 0);
}
#line 842 "fts5parse.c"
        break;
      case 2: /* expr ::= expr OR expr */
#line 96 "fts5parse.y"
{
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_OR, yymsp[-2].minor.yy18, yymsp[0].minor.yy18, 0);
}
#line 849 "fts5parse.c"
        break;
      case 3: /* expr ::= expr NOT expr */
#line 99 "fts5parse.y"
{
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_NOT, yymsp[-2].minor.yy18, yymsp[0].minor.yy18, 0);
}
#line 856 "fts5parse.c"
        break;
      case 4: /* expr ::= LP expr RP */
#line 103 "fts5parse.y"
{yygotominor.yy18 = yymsp[-1].minor.yy18;}
#line 861 "fts5parse.c"
        break;
      case 5: /* expr ::= exprlist */
      case 6: /* exprlist ::= cnearset */ yytestcase(yyruleno==6);
#line 104 "fts5parse.y"
{yygotominor.yy18 = yymsp[0].minor.yy18;}
#line 867 "fts5parse.c"
        break;
      case 7: /* exprlist ::= exprlist cnearset */
#line 107 "fts5parse.y"
{
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_AND, yymsp[-1].minor.yy18, yymsp[0].minor.yy18, 0);
}
#line 874 "fts5parse.c"
        break;
      case 8: /* cnearset ::= nearset */
#line 111 "fts5parse.y"
{ 
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_STRING, 0, 0, yymsp[0].minor.yy26); 
}
#line 881 "fts5parse.c"
        break;
      case 9: /* cnearset ::= colset COLON nearset */
#line 114 "fts5parse.y"
{ 
  sqlite3Fts5ParseSetColset(pParse, yymsp[0].minor.yy26, yymsp[-2].minor.yy3);
  yygotominor.yy18 = sqlite3Fts5ParseNode(pParse, FTS5_STRING, 0, 0, yymsp[0].minor.yy26); 
}
#line 889 "fts5parse.c"
        break;
      case 10: /* colset ::= LCP colsetlist RCP */
#line 124 "fts5parse.y"
{ yygotominor.yy3 = yymsp[-1].minor.yy3; }
#line 894 "fts5parse.c"
        break;
      case 11: /* colset ::= STRING */
#line 125 "fts5parse.y"
{
  yygotominor.yy3 = sqlite3Fts5ParseColset(pParse, 0, &yymsp[0].minor.yy0);
}
#line 901 "fts5parse.c"
        break;
      case 12: /* colsetlist ::= colsetlist STRING */
#line 129 "fts5parse.y"
{ 
  yygotominor.yy3 = sqlite3Fts5ParseColset(pParse, yymsp[-1].minor.yy3, &yymsp[0].minor.yy0); }
#line 907 "fts5parse.c"
        break;
      case 13: /* colsetlist ::= STRING */
#line 131 "fts5parse.y"
{ 
  yygotominor.yy3 = sqlite3Fts5ParseColset(pParse, 0, &yymsp[0].minor.yy0); 
}
#line 914 "fts5parse.c"
        break;
      case 14: /* nearset ::= phrase */
#line 141 "fts5parse.y"
{ yygotominor.yy26 = sqlite3Fts5ParseNearset(pParse, 0, yymsp[0].minor.yy11); }
#line 919 "fts5parse.c"
        break;
      case 15: /* nearset ::= STRING LP nearphrases neardist_opt RP */
#line 142 "fts5parse.y"
{
  sqlite3Fts5ParseNear(pParse, &yymsp[-4].minor.yy0);
  sqlite3Fts5ParseSetDistance(pParse, yymsp[-2].minor.yy26, &yymsp[-1].minor.yy0);
  yygotominor.yy26 = yymsp[-2].minor.yy26;
}
#line 928 "fts5parse.c"
        break;
      case 16: /* nearphrases ::= phrase */
#line 148 "fts5parse.y"
{ 
  yygotominor.yy26 = sqlite3Fts5ParseNearset(pParse, 0, yymsp[0].minor.yy11); 
}
#line 935 "fts5parse.c"
        break;
      case 17: /* nearphrases ::= nearphrases phrase */
#line 151 "fts5parse.y"
{
  yygotominor.yy26 = sqlite3Fts5ParseNearset(pParse, yymsp[-1].minor.yy26, yymsp[0].minor.yy11);
}
#line 942 "fts5parse.c"
        break;
      case 18: /* neardist_opt ::= */
#line 158 "fts5parse.y"
{ yygotominor.yy0.p = 0; yygotominor.yy0.n = 0; }
#line 947 "fts5parse.c"
        break;
      case 19: /* neardist_opt ::= COMMA STRING */
#line 159 "fts5parse.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 952 "fts5parse.c"
        break;
      case 20: /* phrase ::= phrase PLUS STRING star_opt */
#line 171 "fts5parse.y"
{ 
  yygotominor.yy11 = sqlite3Fts5ParseTerm(pParse, yymsp[-3].minor.yy11, &yymsp[-1].minor.yy0, yymsp[0].minor.yy20);
}
#line 959 "fts5parse.c"
        break;
      case 21: /* phrase ::= STRING star_opt */
#line 174 "fts5parse.y"
{ 
  yygotominor.yy11 = sqlite3Fts5ParseTerm(pParse, 0, &yymsp[-1].minor.yy0, yymsp[0].minor.yy20);
}
#line 966 "fts5parse.c"
        break;
      case 22: /* star_opt ::= STAR */
#line 183 "fts5parse.y"
{ yygotominor.yy20 = 1; }
#line 971 "fts5parse.c"
        break;
      case 23: /* star_opt ::= */
#line 184 "fts5parse.y"
{ yygotominor.yy20 = 0; }
#line 976 "fts5parse.c"
        break;
      default:
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
  sqlite3Fts5ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  sqlite3Fts5ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
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
  sqlite3Fts5ParserARG_FETCH;
#define TOKEN (yyminor.yy0)
/************ Begin %syntax_error code ****************************************/
#line 30 "fts5parse.y"

  UNUSED_PARAM(yymajor); /* Silence a compiler warning */
  sqlite3Fts5ParseError(
    pParse, "fts5: syntax error near \"%.*s\"",TOKEN.n,TOKEN.p
  );
#line 1048 "fts5parse.c"
/************ End %syntax_error code ******************************************/
  sqlite3Fts5ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3Fts5ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  sqlite3Fts5ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3Fts5ParserAlloc" which describes the current state of the parser.
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
void sqlite3Fts5Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3Fts5ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3Fts5ParserARG_PDECL               /* Optional %extra_argument parameter */
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
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sInitialize. Empty stack. State 0\n",
              yyTracePrompt);
    }
#endif
  }
  yyminorunion.yy0 = yyminor;
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  sqlite3Fts5ParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
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
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
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
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
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
  if( yyTraceFILE ){
    int i;
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE,"%c%s", i==1 ? '[' : ' ', 
              yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
