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
#line 49 "parse.y"

#include "sqliteInt.h"

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
#define YYPARSEFREENEVERNULL 1

/*
** Alternative datatype for the argument to the malloc() routine passed
** into sqlite3ParserAlloc().  The default is size_t.
*/
#define YYMALLOCARGTYPE  u64

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int bNot;         /* True if the NOT keyword is present */
};

/*
** An instance of the following structure describes the event of a
** TRIGGER.  "a" is the event type, one of TK_UPDATE, TK_INSERT,
** TK_DELETE, or TK_INSTEAD.  If the event is of the form
**
**      UPDATE ON (a,b,c)
**
** Then the "b" IdList records the list "a,b,c".
*/
struct TrigEvent { int a; IdList * b; };

/*
** An instance of this structure holds the ATTACH key and the key type.
*/
struct AttachKey { int type;  Token key; };

/*
** Disable lookaside memory allocation for objects that might be
** shared across database connections.
*/
static void disableLookaside(Parse *pParse){
  pParse->disableLookaside++;
  pParse->db->lookaside.bDisable++;
}

#line 434 "parse.y"

  /*
  ** For a compound SELECT statement, make sure p->pPrior->pNext==p for
  ** all elements in the list.  And make sure list length does not exceed
  ** SQLITE_LIMIT_COMPOUND_SELECT.
  */
  static void parserDoubleLinkSelect(Parse *pParse, Select *p){
    if( p->pPrior ){
      Select *pNext = 0, *pLoop;
      int mxSelect, cnt = 0;
      for(pLoop=p; pLoop; pNext=pLoop, pLoop=pLoop->pPrior, cnt++){
        pLoop->pNext = pNext;
        pLoop->selFlags |= SF_Compound;
      }
      if( (p->selFlags & SF_MultiValue)==0 && 
        (mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT])>0 &&
        cnt>mxSelect
      ){
        sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
      }
    }
  }
#line 847 "parse.y"

  /* This is a utility routine used to set the ExprSpan.zStart and
  ** ExprSpan.zEnd values of pOut so that the span covers the complete
  ** range of text beginning with pStart and going to the end of pEnd.
  */
  static void spanSet(ExprSpan *pOut, Token *pStart, Token *pEnd){
    pOut->zStart = pStart->z;
    pOut->zEnd = &pEnd->z[pEnd->n];
  }

  /* Construct a new Expr object from a single identifier.  Use the
  ** new Expr to populate pOut.  Set the span of pOut to be the identifier
  ** that created the expression.
  */
  static void spanExpr(ExprSpan *pOut, Parse *pParse, int op, Token *pValue){
    pOut->pExpr = sqlite3PExpr(pParse, op, 0, 0, pValue);
    pOut->zStart = pValue->z;
    pOut->zEnd = &pValue->z[pValue->n];
  }
#line 937 "parse.y"

  /* This routine constructs a binary expression node out of two ExprSpan
  ** objects and uses the result to populate a new ExprSpan object.
  */
  static void spanBinaryExpr(
    ExprSpan *pOut,     /* Write the result here */
    Parse *pParse,      /* The parsing context.  Errors accumulate here */
    int op,             /* The binary operation */
    ExprSpan *pLeft,    /* The left operand */
    ExprSpan *pRight    /* The right operand */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pLeft->pExpr, pRight->pExpr, 0);
    pOut->zStart = pLeft->zStart;
    pOut->zEnd = pRight->zEnd;
  }

  /* If doNot is true, then add a TK_NOT Expr-node wrapper around the
  ** outside of *ppExpr.
  */
  static void exprNot(Parse *pParse, int doNot, Expr **ppExpr){
    if( doNot ) *ppExpr = sqlite3PExpr(pParse, TK_NOT, *ppExpr, 0, 0);
  }
#line 998 "parse.y"

  /* Construct an expression node for a unary postfix operator
  */
  static void spanUnaryPostfix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPostOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pOperand->zStart;
    pOut->zEnd = &pPostOp->z[pPostOp->n];
  }                           
#line 1017 "parse.y"

  /* A routine to convert a binary TK_IS or TK_ISNOT expression into a
  ** unary TK_ISNULL or TK_NOTNULL expression. */
  static void binaryToUnaryIfNull(Parse *pParse, Expr *pY, Expr *pA, int op){
    sqlite3 *db = pParse->db;
    if( pA && pY && pY->op==TK_NULL ){
      pA->op = (u8)op;
      sqlite3ExprDelete(db, pA->pRight);
      pA->pRight = 0;
    }
  }
#line 1045 "parse.y"

  /* Construct an expression node for a unary prefix operator
  */
  static void spanUnaryPrefix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPreOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pPreOp->z;
    pOut->zEnd = pOperand->zEnd;
  }
#line 1274 "parse.y"

  /* Add a single new term to an ExprList that is used to store a
  ** list of identifiers.  Report an error if the ID list contains
  ** a COLLATE clause or an ASC or DESC keyword, except ignore the
  ** error while parsing a legacy schema.
  */
  static ExprList *parserAddExprIdListTerm(
    Parse *pParse,
    ExprList *pPrior,
    Token *pIdToken,
    int hasCollate,
    int sortOrder
  ){
    ExprList *p = sqlite3ExprListAppend(pParse, pPrior, 0);
    if( (hasCollate || sortOrder!=SQLITE_SO_UNDEFINED)
        && pParse->db->init.busy==0
    ){
      sqlite3ErrorMsg(pParse, "syntax error after column name \"%.*s\"",
                         pIdToken->n, pIdToken->z);
    }
    sqlite3ExprListSetName(pParse, p, pIdToken, 1);
    return p;
  }
#line 230 "parse.c"
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
**    sqlite3ParserTOKENTYPE     is the data type used for minor type for terminal
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
**                       which is sqlite3ParserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    sqlite3ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3ParserARG_FETCH     Code to extract %extra_argument from yypParser
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
#define YYNOCODE 253
#define YYACTIONTYPE unsigned short int
#define YYWILDCARD 70
#define sqlite3ParserTOKENTYPE Token
typedef union {
  int yyinit;
  sqlite3ParserTOKENTYPE yy0;
  int yy4;
  struct TrigEvent yy90;
  ExprSpan yy118;
  TriggerStep* yy203;
  struct {int value; int mask;} yy215;
  SrcList* yy259;
  struct LimitVal yy292;
  Expr* yy314;
  ExprList* yy322;
  struct LikeOp yy342;
  IdList* yy384;
  Select* yy387;
  With* yy451;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define sqlite3ParserARG_SDECL Parse *pParse;
#define sqlite3ParserARG_PDECL ,Parse *pParse
#define sqlite3ParserARG_FETCH Parse *pParse = yypParser->pParse
#define sqlite3ParserARG_STORE yypParser->pParse = pParse
#define YYFALLBACK 1
#define YYNSTATE             436
#define YYNRULE              328
#define YY_MAX_SHIFT         435
#define YY_MIN_SHIFTREDUCE   649
#define YY_MAX_SHIFTREDUCE   976
#define YY_MIN_REDUCE        977
#define YY_MAX_REDUCE        1304
#define YY_ERROR_ACTION      1305
#define YY_ACCEPT_ACTION     1306
#define YY_NO_ACTION         1307
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
#define YY_ACTTAB_COUNT (1501)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   311, 1306,  145,  651,    2,  192,  652,  338,  780,   92,
 /*    10 */    92,   92,   92,   85,   90,   90,   90,   90,   89,   89,
 /*    20 */    88,   88,   88,   87,  335,   88,   88,   88,   87,  335,
 /*    30 */   327,  856,  856,   92,   92,   92,   92,  697,   90,   90,
 /*    40 */    90,   90,   89,   89,   88,   88,   88,   87,  335,   76,
 /*    50 */   807,   74,   93,   94,   84,  868,  871,  860,  860,   91,
 /*    60 */    91,   92,   92,   92,   92,  335,   90,   90,   90,   90,
 /*    70 */    89,   89,   88,   88,   88,   87,  335,  311,  780,   90,
 /*    80 */    90,   90,   90,   89,   89,   88,   88,   88,   87,  335,
 /*    90 */   356,  808,  776,  701,  689,  689,   86,   83,  166,  257,
 /*   100 */   809,  715,  430,   86,   83,  166,  324,  697,  856,  856,
 /*   110 */   201,  158,  276,  387,  271,  386,  188,  689,  689,  828,
 /*   120 */    86,   83,  166,  269,  833,   49,  123,   87,  335,   93,
 /*   130 */    94,   84,  868,  871,  860,  860,   91,   91,   92,   92,
 /*   140 */    92,   92,  239,   90,   90,   90,   90,   89,   89,   88,
 /*   150 */    88,   88,   87,  335,  311,  763,  333,  332,  216,  408,
 /*   160 */   394,   69,  231,  393,  690,  691,  396,  910,  251,  354,
 /*   170 */   250,  288,  315,  430,  908,  430,  909,   89,   89,   88,
 /*   180 */    88,   88,   87,  335,  391,  856,  856,  690,  691,  183,
 /*   190 */    95,  123,  384,  381,  380,  833,   31,  833,   49,  912,
 /*   200 */   912,  751,  752,  379,  123,  311,   93,   94,   84,  868,
 /*   210 */   871,  860,  860,   91,   91,   92,   92,   92,   92,  114,
 /*   220 */    90,   90,   90,   90,   89,   89,   88,   88,   88,   87,
 /*   230 */   335,  430,  408,  399,  435,  657,  856,  856,  346,   57,
 /*   240 */   232,  828,  109,  704,  366,  689,  689,  363,  825,  760,
 /*   250 */    97,  749,  752,  833,   49,  708,  708,   93,   94,   84,
 /*   260 */   868,  871,  860,  860,   91,   91,   92,   92,   92,   92,
 /*   270 */   423,   90,   90,   90,   90,   89,   89,   88,   88,   88,
 /*   280 */    87,  335,  311,  114,   22,  361,  688,   58,  408,  390,
 /*   290 */   251,  349,  240,  213,  762,  689,  689,  847,  685,  115,
 /*   300 */   361,  231,  393,  689,  689,  396,  183,  689,  689,  384,
 /*   310 */   381,  380,  361,  856,  856,  690,  691,  160,  159,  223,
 /*   320 */   379,  738,   25,  806,  707,  841,  143,  689,  689,  835,
 /*   330 */   392,  339,  766,  766,   93,   94,   84,  868,  871,  860,
 /*   340 */   860,   91,   91,   92,   92,   92,   92,  914,   90,   90,
 /*   350 */    90,   90,   89,   89,   88,   88,   88,   87,  335,  311,
 /*   360 */   840,  840,  840,  266,  257,  690,  691,  778,  706,   86,
 /*   370 */    83,  166,  219,  690,  691,  737,    1,  690,  691,  689,
 /*   380 */   689,  689,  689,  430,   86,   83,  166,  249,  688,  937,
 /*   390 */   856,  856,  427,  699,  700,  828,  298,  690,  691,  221,
 /*   400 */   686,  115,  123,  944,  795,  833,   48,  342,  305,  970,
 /*   410 */   847,   93,   94,   84,  868,  871,  860,  860,   91,   91,
 /*   420 */    92,   92,   92,   92,  114,   90,   90,   90,   90,   89,
 /*   430 */    89,   88,   88,   88,   87,  335,  311,  940,  841,  679,
 /*   440 */   713,  429,  835,  430,  251,  354,  250,  355,  288,  690,
 /*   450 */   691,  690,  691,  285,  941,  340,  971,  287,  210,   23,
 /*   460 */   174,  793,  832,  430,  353,  833,   10,  856,  856,   24,
 /*   470 */   942,  151,  753,  840,  840,  840,  794,  968, 1290,  321,
 /*   480 */   398, 1290,  356,  352,  754,  833,   49,  935,   93,   94,
 /*   490 */    84,  868,  871,  860,  860,   91,   91,   92,   92,   92,
 /*   500 */    92,  430,   90,   90,   90,   90,   89,   89,   88,   88,
 /*   510 */    88,   87,  335,  311,  376,  114,  907,  705,  430,  907,
 /*   520 */   328,  890,  114,  833,   10,  966,  430,  857,  857,  320,
 /*   530 */   189,  163,  832,  165,  430,  906,  344,  323,  906,  904,
 /*   540 */   833,   10,  965,  306,  856,  856,  187,  419,  833,   10,
 /*   550 */   220,  869,  872,  832,  222,  403,  833,   49, 1219,  793,
 /*   560 */    68,  937,  406,  245,   66,   93,   94,   84,  868,  871,
 /*   570 */   860,  860,   91,   91,   92,   92,   92,   92,  861,   90,
 /*   580 */    90,   90,   90,   89,   89,   88,   88,   88,   87,  335,
 /*   590 */   311,  404,  213,  762,  834,  345,  114,  940,  902,  368,
 /*   600 */   727,    5,  316,  192,  396,  772,  780,  269,  230,  242,
 /*   610 */   771,  244,  397,  164,  941,  385,  123,  347,   55,  355,
 /*   620 */   329,  856,  856,  728,  333,  332,  688,  968, 1291,  724,
 /*   630 */   942, 1291,  413,  214,  833,    9,  362,  286,  955,  115,
 /*   640 */   718,  311,   93,   94,   84,  868,  871,  860,  860,   91,
 /*   650 */    91,   92,   92,   92,   92,  430,   90,   90,   90,   90,
 /*   660 */    89,   89,   88,   88,   88,   87,  335,  912,  912, 1300,
 /*   670 */  1300,  758,  856,  856,  325,  966,  780,  833,   35,  747,
 /*   680 */   720,  334,  699,  700,  977,  652,  338,  243,  745,  920,
 /*   690 */   920,  369,  187,   93,   94,   84,  868,  871,  860,  860,
 /*   700 */    91,   91,   92,   92,   92,   92,  114,   90,   90,   90,
 /*   710 */    90,   89,   89,   88,   88,   88,   87,  335,  311,  430,
 /*   720 */   954,  430,  112,  310,  430,  693,  317,  698,  400,  430,
 /*   730 */   793,  359,  430, 1017,  430,  192,  430,  401,  780,  430,
 /*   740 */   360,  833,   36,  833,   12,  430,  833,   27,  316,  856,
 /*   750 */   856,  833,   37,   20,  833,   38,  833,   39,  833,   28,
 /*   760 */    72,  833,   29,  663,  664,  665,  264,  833,   40,  234,
 /*   770 */    93,   94,   84,  868,  871,  860,  860,   91,   91,   92,
 /*   780 */    92,   92,   92,  430,   90,   90,   90,   90,   89,   89,
 /*   790 */    88,   88,   88,   87,  335,  311,  430,  698,  430,  917,
 /*   800 */   147,  430,  165,  916,  275,  833,   41,  430,  780,  430,
 /*   810 */    21,  430,  259,  430,  262,  274,  430,  367,  833,   42,
 /*   820 */   833,   11,  430,  833,   43,  235,  856,  856,  793,  833,
 /*   830 */    99,  833,   44,  833,   45,  833,   32,   75,  833,   46,
 /*   840 */   305,  967,  257,  257,  833,   47,  311,   93,   94,   84,
 /*   850 */   868,  871,  860,  860,   91,   91,   92,   92,   92,   92,
 /*   860 */   430,   90,   90,   90,   90,   89,   89,   88,   88,   88,
 /*   870 */    87,  335,  430,  186,  185,  184,  238,  856,  856,  650,
 /*   880 */     2, 1064,  833,   33,  739,  217,  218,  257,  971,  257,
 /*   890 */   426,  317,  257,  774,  833,  117,  257,  311,   93,   94,
 /*   900 */    84,  868,  871,  860,  860,   91,   91,   92,   92,   92,
 /*   910 */    92,  430,   90,   90,   90,   90,   89,   89,   88,   88,
 /*   920 */    88,   87,  335,  430,  318,  124,  212,  163,  856,  856,
 /*   930 */   943,  900,  898,  833,  118,  759,  726,  725,  257,  755,
 /*   940 */   289,  289,  733,  734,  961,  833,  119,  682,  311,   93,
 /*   950 */    82,   84,  868,  871,  860,  860,   91,   91,   92,   92,
 /*   960 */    92,   92,  430,   90,   90,   90,   90,   89,   89,   88,
 /*   970 */    88,   88,   87,  335,  430,  716,  246,  322,  331,  856,
 /*   980 */   856,  256,  114,  357,  833,   53,  808,  913,  913,  932,
 /*   990 */   156,  416,  420,  424,  930,  809,  833,   34,  364,  311,
 /*  1000 */   253,   94,   84,  868,  871,  860,  860,   91,   91,   92,
 /*  1010 */    92,   92,   92,  430,   90,   90,   90,   90,   89,   89,
 /*  1020 */    88,   88,   88,   87,  335,  430,  114,  114,  114,  960,
 /*  1030 */   856,  856,  307,  258,  830,  833,  100,  191,  252,  377,
 /*  1040 */   267,   68,  197,   68,  261,  716,  769,  833,   50,   71,
 /*  1050 */   911,  911,  263,   84,  868,  871,  860,  860,   91,   91,
 /*  1060 */    92,   92,   92,   92,  430,   90,   90,   90,   90,   89,
 /*  1070 */    89,   88,   88,   88,   87,  335,   80,  425,  802,    3,
 /*  1080 */  1214,  191,  430,  265,  336,  336,  833,  101,  741,   80,
 /*  1090 */   425,  897,    3,  723,  722,  428,  721,  336,  336,  430,
 /*  1100 */   893,  270,  430,  197,  833,  102,  430,  800,  428,  430,
 /*  1110 */   695,  430,  843,  111,  414,  430,  784,  409,  430,  831,
 /*  1120 */   430,  833,   98,  123,  833,  116,  847,  414,  833,   49,
 /*  1130 */   779,  833,  113,  833,  106,  226,  123,  833,  105,  847,
 /*  1140 */   833,  103,  833,  104,  791,  411,   77,   78,  290,  412,
 /*  1150 */   430,  291,  114,   79,  432,  431,  389,  430,  835,   77,
 /*  1160 */    78,  897,  839,  408,  410,  430,   79,  432,  431,  372,
 /*  1170 */   703,  835,  833,   52,  430,   80,  425,  430,    3,  833,
 /*  1180 */    54,  772,  843,  336,  336,  684,  771,  833,   51,  840,
 /*  1190 */   840,  840,  842,   19,  428,  672,  833,   26,  671,  833,
 /*  1200 */    30,  673,  840,  840,  840,  842,   19,  207,  661,  278,
 /*  1210 */   304,  148,  280,  414,  282,  248,  358,  822,  382,    6,
 /*  1220 */   348,  161,  273,   80,  425,  847,    3,  934,  895,  720,
 /*  1230 */   894,  336,  336,  296,  157,  415,  241,  284,  674,  958,
 /*  1240 */   194,  953,  428,  951,  948,   77,   78,  777,  319,   56,
 /*  1250 */    59,  135,   79,  432,  431,  121,   66,  835,  146,  128,
 /*  1260 */   350,  414,  819,  130,  351,  131,  132,  133,  375,  173,
 /*  1270 */   107,  138,  149,  847,  365,  178,   62,   70,  425,  936,
 /*  1280 */     3,  827,  889,  371,  255,  336,  336,  792,  840,  840,
 /*  1290 */   840,  842,   19,   77,   78,  915,  428,  208,  179,  144,
 /*  1300 */    79,  432,  431,  373,  260,  835,  180,  326,  675,  181,
 /*  1310 */   308,  744,  388,  743,  731,  414,  718,  742,  730,  712,
 /*  1320 */   402,  309,  711,  272,  788,   65,  710,  847,  709,  277,
 /*  1330 */   193,  789,  787,  279,  876,   73,  840,  840,  840,  842,
 /*  1340 */    19,  786,  281,  418,  283,  422,  227,   77,   78,  330,
 /*  1350 */   228,  229,   96,  767,   79,  432,  431,  407,   67,  835,
 /*  1360 */   215,  292,  293,  405,  294,  303,  302,  301,  204,  299,
 /*  1370 */   295,  202,  676,  681,    7,  433,  669,  203,  205,  206,
 /*  1380 */   125,  110,  313,  434,  667,  666,  658,  168,  224,  237,
 /*  1390 */   840,  840,  840,  842,   19,  120,  656,  337,  236,  155,
 /*  1400 */   167,  341,  233,  314,  108,  905,  903,  826,  127,  126,
 /*  1410 */   756,  170,  129,  172,  247,  928,  134,  136,  171,   60,
 /*  1420 */    61,  123,  169,  137,  933,  175,  176,  927,    8,   13,
 /*  1430 */   177,  254,  918,  139,  191,  924,  140,  370,  678,  150,
 /*  1440 */   374,  182,  274,  268,  141,  122,   63,   14,  378,   15,
 /*  1450 */   383,   64,  225,  846,  845,  874,   16,    4,  729,  765,
 /*  1460 */   770,  162,  395,  209,  211,  142,  801,  878,  796,  312,
 /*  1470 */    71,   68,  875,  873,  939,  190,  417,  938,   17,  195,
 /*  1480 */   196,  152,   18,  975,  199,  976,  153,  198,  154,  421,
 /*  1490 */   877,  844,  696,   81,  200,  297,  343, 1019, 1018,  300,
 /*  1500 */   653,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    19,  144,  145,  146,  147,   24,    1,    2,   27,   80,
 /*    10 */    81,   82,   83,   84,   85,   86,   87,   88,   89,   90,
 /*    20 */    91,   92,   93,   94,   95,   91,   92,   93,   94,   95,
 /*    30 */    19,   50,   51,   80,   81,   82,   83,   27,   85,   86,
 /*    40 */    87,   88,   89,   90,   91,   92,   93,   94,   95,  137,
 /*    50 */   177,  139,   71,   72,   73,   74,   75,   76,   77,   78,
 /*    60 */    79,   80,   81,   82,   83,   95,   85,   86,   87,   88,
 /*    70 */    89,   90,   91,   92,   93,   94,   95,   19,   97,   85,
 /*    80 */    86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
 /*    90 */   152,   33,  212,  173,   27,   28,  223,  224,  225,  152,
 /*   100 */    42,  181,  152,  223,  224,  225,   95,   97,   50,   51,
 /*   110 */    99,  100,  101,  102,  103,  104,  105,   27,   28,   59,
 /*   120 */   223,  224,  225,  112,  174,  175,   66,   94,   95,   71,
 /*   130 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   140 */    82,   83,  195,   85,   86,   87,   88,   89,   90,   91,
 /*   150 */    92,   93,   94,   95,   19,  197,   89,   90,  220,  209,
 /*   160 */   210,   26,  119,  120,   97,   98,  208,  100,  108,  109,
 /*   170 */   110,  152,  157,  152,  107,  152,  109,   89,   90,   91,
 /*   180 */    92,   93,   94,   95,  163,   50,   51,   97,   98,   99,
 /*   190 */    55,   66,  102,  103,  104,  174,  175,  174,  175,  132,
 /*   200 */   133,  192,  193,  113,   66,   19,   71,   72,   73,   74,
 /*   210 */    75,   76,   77,   78,   79,   80,   81,   82,   83,  198,
 /*   220 */    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   230 */    95,  152,  209,  210,  148,  149,   50,   51,  100,   53,
 /*   240 */   154,   59,  156,  174,  229,   27,   28,  232,  163,  163,
 /*   250 */    22,  192,  193,  174,  175,   27,   28,   71,   72,   73,
 /*   260 */    74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   270 */   251,   85,   86,   87,   88,   89,   90,   91,   92,   93,
 /*   280 */    94,   95,   19,  198,  198,  152,  152,   24,  209,  210,
 /*   290 */   108,  109,  110,  196,  197,   27,   28,   69,  164,  165,
 /*   300 */   152,  119,  120,   27,   28,  208,   99,   27,   28,  102,
 /*   310 */   103,  104,  152,   50,   51,   97,   98,   89,   90,  185,
 /*   320 */   113,  187,   22,  177,  174,   97,   58,   27,   28,  101,
 /*   330 */   115,  245,  117,  118,   71,   72,   73,   74,   75,   76,
 /*   340 */    77,   78,   79,   80,   81,   82,   83,   11,   85,   86,
 /*   350 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   19,
 /*   360 */   132,  133,  134,   23,  152,   97,   98,   91,  174,  223,
 /*   370 */   224,  225,  239,   97,   98,  187,   22,   97,   98,   27,
 /*   380 */    28,   27,   28,  152,  223,  224,  225,  239,  152,  163,
 /*   390 */    50,   51,  170,  171,  172,   59,  160,   97,   98,  239,
 /*   400 */   164,  165,   66,  242,  124,  174,  175,  195,   22,   23,
 /*   410 */    69,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*   420 */    80,   81,   82,   83,  198,   85,   86,   87,   88,   89,
 /*   430 */    90,   91,   92,   93,   94,   95,   19,   12,   97,   21,
 /*   440 */    23,  152,  101,  152,  108,  109,  110,  221,  152,   97,
 /*   450 */    98,   97,   98,  152,   29,  243,   70,  226,   23,  233,
 /*   460 */    26,   26,  152,  152,  238,  174,  175,   50,   51,   22,
 /*   470 */    45,   24,   47,  132,  133,  134,  124,   22,   23,  188,
 /*   480 */   163,   26,  152,   65,   59,  174,  175,  163,   71,   72,
 /*   490 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   500 */    83,  152,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   510 */    93,   94,   95,   19,   19,  198,  152,   23,  152,  152,
 /*   520 */   209,  103,  198,  174,  175,   70,  152,   50,   51,  219,
 /*   530 */   213,  214,  152,   98,  152,  171,  172,  188,  171,  172,
 /*   540 */   174,  175,  248,  249,   50,   51,   51,  251,  174,  175,
 /*   550 */   220,   74,   75,  152,  188,  152,  174,  175,  140,  124,
 /*   560 */    26,  163,  188,   16,  130,   71,   72,   73,   74,   75,
 /*   570 */    76,   77,   78,   79,   80,   81,   82,   83,  101,   85,
 /*   580 */    86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
 /*   590 */    19,  209,  196,  197,   23,  231,  198,   12,  231,  219,
 /*   600 */    37,   22,  107,   24,  208,  116,   27,  112,  201,   62,
 /*   610 */   121,   64,  152,  152,   29,   52,   66,  221,  211,  221,
 /*   620 */   219,   50,   51,   60,   89,   90,  152,   22,   23,  183,
 /*   630 */    45,   26,   47,   22,  174,  175,  238,  152,  164,  165,
 /*   640 */   106,   19,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   650 */    79,   80,   81,   82,   83,  152,   85,   86,   87,   88,
 /*   660 */    89,   90,   91,   92,   93,   94,   95,  132,  133,  119,
 /*   670 */   120,  163,   50,   51,  111,   70,   97,  174,  175,  181,
 /*   680 */   182,  170,  171,  172,    0,    1,    2,  140,  190,  108,
 /*   690 */   109,  110,   51,   71,   72,   73,   74,   75,   76,   77,
 /*   700 */    78,   79,   80,   81,   82,   83,  198,   85,   86,   87,
 /*   710 */    88,   89,   90,   91,   92,   93,   94,   95,   19,  152,
 /*   720 */   152,  152,   22,  166,  152,  168,  169,   27,   19,  152,
 /*   730 */    26,   19,  152,  122,  152,   24,  152,   28,   27,  152,
 /*   740 */    28,  174,  175,  174,  175,  152,  174,  175,  107,   50,
 /*   750 */    51,  174,  175,   22,  174,  175,  174,  175,  174,  175,
 /*   760 */   138,  174,  175,    7,    8,    9,   16,  174,  175,  152,
 /*   770 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*   780 */    81,   82,   83,  152,   85,   86,   87,   88,   89,   90,
 /*   790 */    91,   92,   93,   94,   95,   19,  152,   97,  152,   31,
 /*   800 */    24,  152,   98,   35,  101,  174,  175,  152,   97,  152,
 /*   810 */    79,  152,   62,  152,   64,  112,  152,   49,  174,  175,
 /*   820 */   174,  175,  152,  174,  175,  152,   50,   51,  124,  174,
 /*   830 */   175,  174,  175,  174,  175,  174,  175,  138,  174,  175,
 /*   840 */    22,   23,  152,  152,  174,  175,   19,   71,   72,   73,
 /*   850 */    74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   860 */   152,   85,   86,   87,   88,   89,   90,   91,   92,   93,
 /*   870 */    94,   95,  152,  108,  109,  110,  152,   50,   51,  146,
 /*   880 */   147,   23,  174,  175,   26,  195,  195,  152,   70,  152,
 /*   890 */   168,  169,  152,   26,  174,  175,  152,   19,   71,   72,
 /*   900 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   910 */    83,  152,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   920 */    93,   94,   95,  152,  246,  247,  213,  214,   50,   51,
 /*   930 */   195,  152,  195,  174,  175,  195,  100,  101,  152,  195,
 /*   940 */   152,  152,    7,    8,  152,  174,  175,  163,   19,   71,
 /*   950 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   960 */    82,   83,  152,   85,   86,   87,   88,   89,   90,   91,
 /*   970 */    92,   93,   94,   95,  152,   27,  152,  189,  189,   50,
 /*   980 */    51,  195,  198,  152,  174,  175,   33,  132,  133,  152,
 /*   990 */   123,  163,  163,  163,  152,   42,  174,  175,  152,   19,
 /*  1000 */   152,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  1010 */    81,   82,   83,  152,   85,   86,   87,   88,   89,   90,
 /*  1020 */    91,   92,   93,   94,   95,  152,  198,  198,  198,   23,
 /*  1030 */    50,   51,   26,  152,   23,  174,  175,   26,   23,   23,
 /*  1040 */    23,   26,   26,   26,  152,   97,   23,  174,  175,   26,
 /*  1050 */   132,  133,  152,   73,   74,   75,   76,   77,   78,   79,
 /*  1060 */    80,   81,   82,   83,  152,   85,   86,   87,   88,   89,
 /*  1070 */    90,   91,   92,   93,   94,   95,   19,   20,   23,   22,
 /*  1080 */    23,   26,  152,  152,   27,   28,  174,  175,  152,   19,
 /*  1090 */    20,   27,   22,  183,  183,   38,  152,   27,   28,  152,
 /*  1100 */    23,  152,  152,   26,  174,  175,  152,  152,   38,  152,
 /*  1110 */    23,  152,   27,   26,   57,  152,  215,  163,  152,  152,
 /*  1120 */   152,  174,  175,   66,  174,  175,   69,   57,  174,  175,
 /*  1130 */   152,  174,  175,  174,  175,  212,   66,  174,  175,   69,
 /*  1140 */   174,  175,  174,  175,  152,  152,   89,   90,  152,  193,
 /*  1150 */   152,  152,  198,   96,   97,   98,   91,  152,  101,   89,
 /*  1160 */    90,   97,  152,  209,  210,  152,   96,   97,   98,  235,
 /*  1170 */   152,  101,  174,  175,  152,   19,   20,  152,   22,  174,
 /*  1180 */   175,  116,   97,   27,   28,  152,  121,  174,  175,  132,
 /*  1190 */   133,  134,  135,  136,   38,  152,  174,  175,  152,  174,
 /*  1200 */   175,  152,  132,  133,  134,  135,  136,  234,  152,  212,
 /*  1210 */   150,  199,  212,   57,  212,  240,  240,  203,  178,  200,
 /*  1220 */   216,  186,  177,   19,   20,   69,   22,  203,  177,  182,
 /*  1230 */   177,   27,   28,  202,  200,  228,  216,  216,  155,   39,
 /*  1240 */   122,  159,   38,  159,   41,   89,   90,   91,  159,  241,
 /*  1250 */   241,   22,   96,   97,   98,   71,  130,  101,  222,  191,
 /*  1260 */    18,   57,  203,  194,  159,  194,  194,  194,   18,  158,
 /*  1270 */   244,  191,  222,   69,  159,  158,  137,   19,   20,  203,
 /*  1280 */    22,  191,  203,   46,  236,   27,   28,  159,  132,  133,
 /*  1290 */   134,  135,  136,   89,   90,  237,   38,  159,  158,   22,
 /*  1300 */    96,   97,   98,  179,  159,  101,  158,   48,  159,  158,
 /*  1310 */   179,  176,  107,  176,  184,   57,  106,  176,  184,  176,
 /*  1320 */   125,  179,  178,  176,  218,  107,  176,   69,  176,  217,
 /*  1330 */   159,  218,  218,  217,  159,  137,  132,  133,  134,  135,
 /*  1340 */   136,  218,  217,  179,  217,  179,  227,   89,   90,   95,
 /*  1350 */   230,  230,  129,  207,   96,   97,   98,  126,  128,  101,
 /*  1360 */     5,  206,  205,  127,  204,   10,   11,   12,   13,   14,
 /*  1370 */   203,   25,   17,  162,   26,  161,   13,  153,  153,    6,
 /*  1380 */   247,  180,  250,  151,  151,  151,  151,   32,  180,   34,
 /*  1390 */   132,  133,  134,  135,  136,  167,    4,    3,   43,   22,
 /*  1400 */    15,   68,  142,  250,   16,   23,   23,  120,  111,  131,
 /*  1410 */    20,   56,  123,  125,   16,    1,  123,  131,   63,   79,
 /*  1420 */    79,   66,   67,  111,   28,   36,  122,    1,    5,   22,
 /*  1430 */   107,  140,   54,   54,   26,   61,  107,   44,   20,   24,
 /*  1440 */    19,  105,  112,   23,   22,   40,   22,   22,   53,   22,
 /*  1450 */    53,   22,   53,   23,   23,   23,   22,   22,   30,  116,
 /*  1460 */    23,  122,   26,   23,   23,   22,   28,   11,  124,  114,
 /*  1470 */    26,   26,   23,   23,   23,   36,   24,   23,   36,   26,
 /*  1480 */    22,   22,   36,   23,  122,   23,   22,   26,   22,   24,
 /*  1490 */    23,   23,   23,   22,  122,   23,  141,  122,  122,   15,
 /*  1500 */     1,
};
#define YY_SHIFT_USE_DFLT (-89)
#define YY_SHIFT_COUNT (435)
#define YY_SHIFT_MIN   (-88)
#define YY_SHIFT_MAX   (1499)
static const short yy_shift_ofst[] = {
 /*     0 */     5, 1057, 1355, 1070, 1204, 1204, 1204,   90,   60,  -19,
 /*    10 */    58,   58,  186, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    20 */    67,   67,  182,  336,  218,  550,  135,  263,  340,  417,
 /*    30 */   494,  571,  622,  699,  776,  827,  827,  827,  827,  827,
 /*    40 */   827,  827,  827,  827,  827,  827,  827,  827,  827,  827,
 /*    50 */   878,  827,  929,  980,  980, 1156, 1204, 1204, 1204, 1204,
 /*    60 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    70 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    80 */  1204, 1204, 1204, 1204, 1258, 1204, 1204, 1204, 1204, 1204,
 /*    90 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,  -71,  -47,
 /*   100 */   -47,  -47,  -47,  -47,   -6,   88,  -66,  218,  218,  418,
 /*   110 */   495,  535,  535,   33,   43,   10,  -30,  -89,  -89,  -89,
 /*   120 */    11,  425,  425,  268,  455,  605,  218,  218,  218,  218,
 /*   130 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*   140 */   218,  218,  218,  218,  218,  684,  138,   10,   43,  125,
 /*   150 */   125,  125,  125,  125,  125,  -89,  -89,  -89,  228,  341,
 /*   160 */   341,  207,  276,  300,  280,  352,  354,  218,  218,  218,
 /*   170 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*   180 */   218,  218,  218,  218,  563,  563,  563,  218,  218,  435,
 /*   190 */   218,  218,  218,  579,  218,  218,  585,  218,  218,  218,
 /*   200 */   218,  218,  218,  218,  218,  218,  218,  581,  768,  711,
 /*   210 */   711,  711,  704,  215, 1065,  756,  434,  709,  709,  712,
 /*   220 */   434,  712,  534,  858,  641,  953,  709,  -88,  953,  953,
 /*   230 */   867,  489,  447, 1200, 1118, 1118, 1203, 1203, 1118, 1229,
 /*   240 */  1184, 1126, 1242, 1242, 1242, 1242, 1118, 1250, 1126, 1229,
 /*   250 */  1184, 1184, 1126, 1118, 1250, 1139, 1237, 1118, 1118, 1250,
 /*   260 */  1277, 1118, 1250, 1118, 1250, 1277, 1205, 1205, 1205, 1259,
 /*   270 */  1277, 1205, 1210, 1205, 1259, 1205, 1205, 1195, 1218, 1195,
 /*   280 */  1218, 1195, 1218, 1195, 1218, 1118, 1118, 1198, 1277, 1254,
 /*   290 */  1254, 1277, 1223, 1231, 1230, 1236, 1126, 1346, 1348, 1363,
 /*   300 */  1363, 1373, 1373, 1373, 1373,  -89,  -89,  -89,  -89,  -89,
 /*   310 */   -89,  477,  547,  386,  818,  750,  765,  700, 1006,  731,
 /*   320 */  1011, 1015, 1016, 1017,  948,  836,  935,  703, 1023, 1055,
 /*   330 */  1064, 1077,  855,  918, 1087, 1085,  611, 1392, 1394, 1377,
 /*   340 */  1260, 1385, 1333, 1388, 1382, 1383, 1287, 1278, 1297, 1289,
 /*   350 */  1390, 1288, 1398, 1414, 1293, 1286, 1340, 1341, 1312, 1396,
 /*   360 */  1389, 1304, 1426, 1423, 1407, 1323, 1291, 1378, 1408, 1379,
 /*   370 */  1374, 1393, 1329, 1415, 1418, 1421, 1330, 1336, 1422, 1395,
 /*   380 */  1424, 1425, 1420, 1427, 1397, 1428, 1429, 1399, 1405, 1430,
 /*   390 */  1431, 1432, 1343, 1434, 1437, 1435, 1436, 1339, 1440, 1441,
 /*   400 */  1438, 1439, 1443, 1344, 1444, 1442, 1445, 1446, 1444, 1449,
 /*   410 */  1450, 1451, 1453, 1454, 1458, 1456, 1460, 1459, 1452, 1461,
 /*   420 */  1462, 1464, 1465, 1461, 1467, 1466, 1468, 1469, 1471, 1362,
 /*   430 */  1372, 1375, 1376, 1472, 1484, 1499,
};
#define YY_REDUCE_USE_DFLT (-144)
#define YY_REDUCE_COUNT (310)
#define YY_REDUCE_MIN   (-143)
#define YY_REDUCE_MAX   (1235)
static const short yy_reduce_ofst[] = {
 /*     0 */  -143,  954,   86,   21,  -50,   23,   79,  134,  226, -120,
 /*    10 */  -127,  146,  161,  291,  349,  366,  311,  382,  374,  231,
 /*    20 */   364,  367,  396,  398,  236,  317, -103, -103, -103, -103,
 /*    30 */  -103, -103, -103, -103, -103, -103, -103, -103, -103, -103,
 /*    40 */  -103, -103, -103, -103, -103, -103, -103, -103, -103, -103,
 /*    50 */  -103, -103, -103, -103, -103,  460,  503,  567,  569,  572,
 /*    60 */   577,  580,  582,  584,  587,  593,  631,  644,  646,  649,
 /*    70 */   655,  657,  659,  661,  664,  670,  708,  720,  759,  771,
 /*    80 */   810,  822,  861,  873,  912,  930,  947,  950,  957,  959,
 /*    90 */   963,  966,  968,  998, 1005, 1013, 1022, 1025, -103, -103,
 /*   100 */  -103, -103, -103, -103, -103, -103, -103,  474,  212,   15,
 /*   110 */   498,  222,  511, -103,   97,  557, -103, -103, -103, -103,
 /*   120 */   -80,    9,   59,   19,  294,  294,  -53,  -62,  690,  691,
 /*   130 */   735,  737,  740,  744,  133,  310,  148,  330,  160,  380,
 /*   140 */   786,  788,  401,  296,  789,  733,   85,  722,  -42,  324,
 /*   150 */   508,  784,  828,  829,  830,  678,  713,  407,   69,  150,
 /*   160 */   194,  188,  289,  301,  403,  461,  485,  568,  617,  673,
 /*   170 */   724,  779,  792,  824,  831,  837,  842,  846,  848,  881,
 /*   180 */   892,  900,  931,  936,  446,  910,  911,  944,  949,  901,
 /*   190 */   955,  967,  978,  923,  992,  993,  956,  996,  999, 1010,
 /*   200 */   289, 1018, 1033, 1043, 1046, 1049, 1056,  934,  973,  997,
 /*   210 */  1000, 1002,  901, 1012, 1019, 1060, 1014, 1004, 1020,  975,
 /*   220 */  1024,  976, 1040, 1035, 1047, 1045, 1021, 1007, 1051, 1053,
 /*   230 */  1031, 1034, 1083, 1026, 1082, 1084, 1008, 1009, 1089, 1036,
 /*   240 */  1068, 1059, 1069, 1071, 1072, 1073, 1105, 1111, 1076, 1050,
 /*   250 */  1080, 1090, 1079, 1115, 1117, 1058, 1048, 1128, 1138, 1140,
 /*   260 */  1124, 1145, 1148, 1149, 1151, 1131, 1135, 1137, 1141, 1130,
 /*   270 */  1142, 1143, 1144, 1147, 1134, 1150, 1152, 1106, 1112, 1113,
 /*   280 */  1116, 1114, 1125, 1123, 1127, 1171, 1175, 1119, 1164, 1120,
 /*   290 */  1121, 1166, 1146, 1155, 1157, 1160, 1167, 1211, 1214, 1224,
 /*   300 */  1225, 1232, 1233, 1234, 1235, 1132, 1153, 1133, 1201, 1208,
 /*   310 */  1228,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   982, 1300, 1300, 1300, 1214, 1214, 1214, 1305, 1300, 1109,
 /*    10 */  1138, 1138, 1274, 1305, 1305, 1305, 1305, 1305, 1305, 1212,
 /*    20 */  1305, 1305, 1305, 1300, 1305, 1113, 1144, 1305, 1305, 1305,
 /*    30 */  1305, 1305, 1305, 1305, 1305, 1273, 1275, 1152, 1151, 1254,
 /*    40 */  1125, 1149, 1142, 1146, 1215, 1208, 1209, 1207, 1211, 1216,
 /*    50 */  1305, 1145, 1177, 1192, 1176, 1305, 1305, 1305, 1305, 1305,
 /*    60 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*    70 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*    80 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*    90 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1186, 1191,
 /*   100 */  1198, 1190, 1187, 1179, 1178, 1180, 1181, 1305, 1305, 1008,
 /*   110 */  1074, 1305, 1305, 1182, 1305, 1020, 1183, 1195, 1194, 1193,
 /*   120 */  1015, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   130 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   140 */  1305, 1305, 1305, 1305, 1305,  982, 1300, 1305, 1305, 1300,
 /*   150 */  1300, 1300, 1300, 1300, 1300, 1292, 1113, 1103, 1305, 1305,
 /*   160 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1280, 1278,
 /*   170 */  1305, 1227, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   180 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   190 */  1305, 1305, 1305, 1109, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   200 */  1305, 1305, 1305, 1305, 1305, 1305,  988, 1305, 1247, 1109,
 /*   210 */  1109, 1109, 1111, 1089, 1101,  990, 1148, 1127, 1127, 1259,
 /*   220 */  1148, 1259, 1045, 1068, 1042, 1138, 1127, 1210, 1138, 1138,
 /*   230 */  1110, 1101, 1305, 1285, 1118, 1118, 1277, 1277, 1118, 1157,
 /*   240 */  1078, 1148, 1085, 1085, 1085, 1085, 1118, 1005, 1148, 1157,
 /*   250 */  1078, 1078, 1148, 1118, 1005, 1253, 1251, 1118, 1118, 1005,
 /*   260 */  1220, 1118, 1005, 1118, 1005, 1220, 1076, 1076, 1076, 1060,
 /*   270 */  1220, 1076, 1045, 1076, 1060, 1076, 1076, 1131, 1126, 1131,
 /*   280 */  1126, 1131, 1126, 1131, 1126, 1118, 1118, 1305, 1220, 1224,
 /*   290 */  1224, 1220, 1143, 1132, 1141, 1139, 1148, 1011, 1063,  998,
 /*   300 */   998,  987,  987,  987,  987, 1297, 1297, 1292, 1047, 1047,
 /*   310 */  1030, 1305, 1305, 1305, 1305, 1305, 1305, 1022, 1305, 1229,
 /*   320 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   330 */  1305, 1305, 1305, 1305, 1305, 1305, 1164, 1305,  983, 1287,
 /*   340 */  1305, 1305, 1284, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   350 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   360 */  1305, 1257, 1305, 1305, 1305, 1305, 1305, 1305, 1250, 1249,
 /*   370 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   380 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305,
 /*   390 */  1305, 1305, 1092, 1305, 1305, 1305, 1096, 1305, 1305, 1305,
 /*   400 */  1305, 1305, 1305, 1305, 1140, 1305, 1133, 1305, 1213, 1305,
 /*   410 */  1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1305, 1302,
 /*   420 */  1305, 1305, 1305, 1301, 1305, 1305, 1305, 1305, 1305, 1166,
 /*   430 */  1305, 1165, 1169, 1305,  996, 1305,
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
    0,  /*          $ => nothing */
    0,  /*       SEMI => nothing */
   27,  /*    EXPLAIN => ID */
   27,  /*      QUERY => ID */
   27,  /*       PLAN => ID */
   27,  /*      BEGIN => ID */
    0,  /* TRANSACTION => nothing */
   27,  /*   DEFERRED => ID */
   27,  /*  IMMEDIATE => ID */
   27,  /*  EXCLUSIVE => ID */
    0,  /*     COMMIT => nothing */
   27,  /*        END => ID */
   27,  /*   ROLLBACK => ID */
   27,  /*  SAVEPOINT => ID */
   27,  /*    RELEASE => ID */
    0,  /*         TO => nothing */
    0,  /*      TABLE => nothing */
    0,  /*     CREATE => nothing */
   27,  /*         IF => ID */
    0,  /*        NOT => nothing */
    0,  /*     EXISTS => nothing */
   27,  /*       TEMP => ID */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*         AS => nothing */
   27,  /*    WITHOUT => ID */
    0,  /*      COMMA => nothing */
    0,  /*         ID => nothing */
    0,  /*    INDEXED => nothing */
   27,  /*      ABORT => ID */
   27,  /*     ACTION => ID */
   27,  /*      AFTER => ID */
   27,  /*    ANALYZE => ID */
   27,  /*        ASC => ID */
   27,  /*     ATTACH => ID */
   27,  /*     BEFORE => ID */
   27,  /*         BY => ID */
   27,  /*    CASCADE => ID */
   27,  /*       CAST => ID */
   27,  /*   COLUMNKW => ID */
   27,  /*   CONFLICT => ID */
   27,  /*   DATABASE => ID */
   27,  /*       DESC => ID */
   27,  /*     DETACH => ID */
   27,  /*       EACH => ID */
   27,  /*       FAIL => ID */
   27,  /*        FOR => ID */
   27,  /*     IGNORE => ID */
   27,  /*  INITIALLY => ID */
   27,  /*    INSTEAD => ID */
   27,  /*    LIKE_KW => ID */
   27,  /*      MATCH => ID */
   27,  /*         NO => ID */
   27,  /*        KEY => ID */
   27,  /*         OF => ID */
   27,  /*     OFFSET => ID */
   27,  /*     PRAGMA => ID */
   27,  /*      RAISE => ID */
   27,  /*  RECURSIVE => ID */
   27,  /*    REPLACE => ID */
   27,  /*   RESTRICT => ID */
   27,  /*        ROW => ID */
   27,  /*    TRIGGER => ID */
   27,  /*     VACUUM => ID */
   27,  /*       VIEW => ID */
   27,  /*    VIRTUAL => ID */
   27,  /*       WITH => ID */
   27,  /*    REINDEX => ID */
   27,  /*     RENAME => ID */
   27,  /*   CTIME_KW => ID */
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
  sqlite3ParserARG_SDECL                /* A place to hold %extra_argument */
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
void sqlite3ParserTrace(FILE *TraceFILE, char *zTracePrompt){
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
  "$",             "SEMI",          "EXPLAIN",       "QUERY",       
  "PLAN",          "BEGIN",         "TRANSACTION",   "DEFERRED",    
  "IMMEDIATE",     "EXCLUSIVE",     "COMMIT",        "END",         
  "ROLLBACK",      "SAVEPOINT",     "RELEASE",       "TO",          
  "TABLE",         "CREATE",        "IF",            "NOT",         
  "EXISTS",        "TEMP",          "LP",            "RP",          
  "AS",            "WITHOUT",       "COMMA",         "ID",          
  "INDEXED",       "ABORT",         "ACTION",        "AFTER",       
  "ANALYZE",       "ASC",           "ATTACH",        "BEFORE",      
  "BY",            "CASCADE",       "CAST",          "COLUMNKW",    
  "CONFLICT",      "DATABASE",      "DESC",          "DETACH",      
  "EACH",          "FAIL",          "FOR",           "IGNORE",      
  "INITIALLY",     "INSTEAD",       "LIKE_KW",       "MATCH",       
  "NO",            "KEY",           "OF",            "OFFSET",      
  "PRAGMA",        "RAISE",         "RECURSIVE",     "REPLACE",     
  "RESTRICT",      "ROW",           "TRIGGER",       "VACUUM",      
  "VIEW",          "VIRTUAL",       "WITH",          "REINDEX",     
  "RENAME",        "CTIME_KW",      "ANY",           "OR",          
  "AND",           "IS",            "BETWEEN",       "IN",          
  "ISNULL",        "NOTNULL",       "NE",            "EQ",          
  "GT",            "LE",            "LT",            "GE",          
  "ESCAPE",        "BITAND",        "BITOR",         "LSHIFT",      
  "RSHIFT",        "PLUS",          "MINUS",         "STAR",        
  "SLASH",         "REM",           "CONCAT",        "COLLATE",     
  "BITNOT",        "STRING",        "JOIN_KW",       "CONSTRAINT",  
  "DEFAULT",       "NULL",          "PRIMARY",       "UNIQUE",      
  "CHECK",         "REFERENCES",    "AUTOINCR",      "ON",          
  "INSERT",        "DELETE",        "UPDATE",        "SET",         
  "DEFERRABLE",    "FOREIGN",       "DROP",          "UNION",       
  "ALL",           "EXCEPT",        "INTERSECT",     "SELECT",      
  "VALUES",        "DISTINCT",      "DOT",           "FROM",        
  "JOIN",          "USING",         "ORDER",         "GROUP",       
  "HAVING",        "LIMIT",         "WHERE",         "INTO",        
  "INTEGER",       "FLOAT",         "BLOB",          "VARIABLE",    
  "CASE",          "WHEN",          "THEN",          "ELSE",        
  "INDEX",         "ALTER",         "ADD",           "error",       
  "input",         "cmdlist",       "ecmd",          "explain",     
  "cmdx",          "cmd",           "transtype",     "trans_opt",   
  "nm",            "savepoint_opt",  "create_table",  "create_table_args",
  "createkw",      "temp",          "ifnotexists",   "dbnm",        
  "columnlist",    "conslist_opt",  "table_options",  "select",      
  "column",        "columnid",      "type",          "carglist",    
  "typetoken",     "typename",      "signed",        "plus_num",    
  "minus_num",     "ccons",         "term",          "expr",        
  "onconf",        "sortorder",     "autoinc",       "eidlist_opt", 
  "refargs",       "defer_subclause",  "refarg",        "refact",      
  "init_deferred_pred_opt",  "conslist",      "tconscomma",    "tcons",       
  "sortlist",      "eidlist",       "defer_subclause_opt",  "orconf",      
  "resolvetype",   "raisetype",     "ifexists",      "fullname",    
  "selectnowith",  "oneselect",     "with",          "multiselect_op",
  "distinct",      "selcollist",    "from",          "where_opt",   
  "groupby_opt",   "having_opt",    "orderby_opt",   "limit_opt",   
  "values",        "nexprlist",     "exprlist",      "sclp",        
  "as",            "seltablist",    "stl_prefix",    "joinop",      
  "indexed_opt",   "on_opt",        "using_opt",     "idlist",      
  "setlist",       "insert_cmd",    "idlist_opt",    "likeop",      
  "between_op",    "in_op",         "case_operand",  "case_exprlist",
  "case_else",     "uniqueflag",    "collate",       "nmnum",       
  "trigger_decl",  "trigger_cmd_list",  "trigger_time",  "trigger_event",
  "foreach_clause",  "when_clause",   "trigger_cmd",   "trnm",        
  "tridxby",       "database_kw_opt",  "key_opt",       "add_column_fullname",
  "kwcolumn_opt",  "create_vtab",   "vtabarglist",   "vtabarg",     
  "vtabargtoken",  "lp",            "anylist",       "wqlist",      
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= cmdlist",
 /*   1 */ "cmdlist ::= cmdlist ecmd",
 /*   2 */ "cmdlist ::= ecmd",
 /*   3 */ "ecmd ::= SEMI",
 /*   4 */ "ecmd ::= explain cmdx SEMI",
 /*   5 */ "explain ::=",
 /*   6 */ "explain ::= EXPLAIN",
 /*   7 */ "explain ::= EXPLAIN QUERY PLAN",
 /*   8 */ "cmdx ::= cmd",
 /*   9 */ "cmd ::= BEGIN transtype trans_opt",
 /*  10 */ "trans_opt ::=",
 /*  11 */ "trans_opt ::= TRANSACTION",
 /*  12 */ "trans_opt ::= TRANSACTION nm",
 /*  13 */ "transtype ::=",
 /*  14 */ "transtype ::= DEFERRED",
 /*  15 */ "transtype ::= IMMEDIATE",
 /*  16 */ "transtype ::= EXCLUSIVE",
 /*  17 */ "cmd ::= COMMIT trans_opt",
 /*  18 */ "cmd ::= END trans_opt",
 /*  19 */ "cmd ::= ROLLBACK trans_opt",
 /*  20 */ "savepoint_opt ::= SAVEPOINT",
 /*  21 */ "savepoint_opt ::=",
 /*  22 */ "cmd ::= SAVEPOINT nm",
 /*  23 */ "cmd ::= RELEASE savepoint_opt nm",
 /*  24 */ "cmd ::= ROLLBACK trans_opt TO savepoint_opt nm",
 /*  25 */ "cmd ::= create_table create_table_args",
 /*  26 */ "create_table ::= createkw temp TABLE ifnotexists nm dbnm",
 /*  27 */ "createkw ::= CREATE",
 /*  28 */ "ifnotexists ::=",
 /*  29 */ "ifnotexists ::= IF NOT EXISTS",
 /*  30 */ "temp ::= TEMP",
 /*  31 */ "temp ::=",
 /*  32 */ "create_table_args ::= LP columnlist conslist_opt RP table_options",
 /*  33 */ "create_table_args ::= AS select",
 /*  34 */ "table_options ::=",
 /*  35 */ "table_options ::= WITHOUT nm",
 /*  36 */ "columnlist ::= columnlist COMMA column",
 /*  37 */ "columnlist ::= column",
 /*  38 */ "column ::= columnid type carglist",
 /*  39 */ "columnid ::= nm",
 /*  40 */ "nm ::= ID|INDEXED",
 /*  41 */ "nm ::= STRING",
 /*  42 */ "nm ::= JOIN_KW",
 /*  43 */ "type ::=",
 /*  44 */ "type ::= typetoken",
 /*  45 */ "typetoken ::= typename",
 /*  46 */ "typetoken ::= typename LP signed RP",
 /*  47 */ "typetoken ::= typename LP signed COMMA signed RP",
 /*  48 */ "typename ::= ID|STRING",
 /*  49 */ "typename ::= typename ID|STRING",
 /*  50 */ "signed ::= plus_num",
 /*  51 */ "signed ::= minus_num",
 /*  52 */ "carglist ::= carglist ccons",
 /*  53 */ "carglist ::=",
 /*  54 */ "ccons ::= CONSTRAINT nm",
 /*  55 */ "ccons ::= DEFAULT term",
 /*  56 */ "ccons ::= DEFAULT LP expr RP",
 /*  57 */ "ccons ::= DEFAULT PLUS term",
 /*  58 */ "ccons ::= DEFAULT MINUS term",
 /*  59 */ "ccons ::= DEFAULT ID|INDEXED",
 /*  60 */ "ccons ::= NULL onconf",
 /*  61 */ "ccons ::= NOT NULL onconf",
 /*  62 */ "ccons ::= PRIMARY KEY sortorder onconf autoinc",
 /*  63 */ "ccons ::= UNIQUE onconf",
 /*  64 */ "ccons ::= CHECK LP expr RP",
 /*  65 */ "ccons ::= REFERENCES nm eidlist_opt refargs",
 /*  66 */ "ccons ::= defer_subclause",
 /*  67 */ "ccons ::= COLLATE ID|STRING",
 /*  68 */ "autoinc ::=",
 /*  69 */ "autoinc ::= AUTOINCR",
 /*  70 */ "refargs ::=",
 /*  71 */ "refargs ::= refargs refarg",
 /*  72 */ "refarg ::= MATCH nm",
 /*  73 */ "refarg ::= ON INSERT refact",
 /*  74 */ "refarg ::= ON DELETE refact",
 /*  75 */ "refarg ::= ON UPDATE refact",
 /*  76 */ "refact ::= SET NULL",
 /*  77 */ "refact ::= SET DEFAULT",
 /*  78 */ "refact ::= CASCADE",
 /*  79 */ "refact ::= RESTRICT",
 /*  80 */ "refact ::= NO ACTION",
 /*  81 */ "defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt",
 /*  82 */ "defer_subclause ::= DEFERRABLE init_deferred_pred_opt",
 /*  83 */ "init_deferred_pred_opt ::=",
 /*  84 */ "init_deferred_pred_opt ::= INITIALLY DEFERRED",
 /*  85 */ "init_deferred_pred_opt ::= INITIALLY IMMEDIATE",
 /*  86 */ "conslist_opt ::=",
 /*  87 */ "conslist_opt ::= COMMA conslist",
 /*  88 */ "conslist ::= conslist tconscomma tcons",
 /*  89 */ "conslist ::= tcons",
 /*  90 */ "tconscomma ::= COMMA",
 /*  91 */ "tconscomma ::=",
 /*  92 */ "tcons ::= CONSTRAINT nm",
 /*  93 */ "tcons ::= PRIMARY KEY LP sortlist autoinc RP onconf",
 /*  94 */ "tcons ::= UNIQUE LP sortlist RP onconf",
 /*  95 */ "tcons ::= CHECK LP expr RP onconf",
 /*  96 */ "tcons ::= FOREIGN KEY LP eidlist RP REFERENCES nm eidlist_opt refargs defer_subclause_opt",
 /*  97 */ "defer_subclause_opt ::=",
 /*  98 */ "defer_subclause_opt ::= defer_subclause",
 /*  99 */ "onconf ::=",
 /* 100 */ "onconf ::= ON CONFLICT resolvetype",
 /* 101 */ "orconf ::=",
 /* 102 */ "orconf ::= OR resolvetype",
 /* 103 */ "resolvetype ::= raisetype",
 /* 104 */ "resolvetype ::= IGNORE",
 /* 105 */ "resolvetype ::= REPLACE",
 /* 106 */ "cmd ::= DROP TABLE ifexists fullname",
 /* 107 */ "ifexists ::= IF EXISTS",
 /* 108 */ "ifexists ::=",
 /* 109 */ "cmd ::= createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select",
 /* 110 */ "cmd ::= DROP VIEW ifexists fullname",
 /* 111 */ "cmd ::= select",
 /* 112 */ "select ::= with selectnowith",
 /* 113 */ "selectnowith ::= oneselect",
 /* 114 */ "selectnowith ::= selectnowith multiselect_op oneselect",
 /* 115 */ "multiselect_op ::= UNION",
 /* 116 */ "multiselect_op ::= UNION ALL",
 /* 117 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /* 118 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /* 119 */ "oneselect ::= values",
 /* 120 */ "values ::= VALUES LP nexprlist RP",
 /* 121 */ "values ::= values COMMA LP exprlist RP",
 /* 122 */ "distinct ::= DISTINCT",
 /* 123 */ "distinct ::= ALL",
 /* 124 */ "distinct ::=",
 /* 125 */ "sclp ::= selcollist COMMA",
 /* 126 */ "sclp ::=",
 /* 127 */ "selcollist ::= sclp expr as",
 /* 128 */ "selcollist ::= sclp STAR",
 /* 129 */ "selcollist ::= sclp nm DOT STAR",
 /* 130 */ "as ::= AS nm",
 /* 131 */ "as ::= ID|STRING",
 /* 132 */ "as ::=",
 /* 133 */ "from ::=",
 /* 134 */ "from ::= FROM seltablist",
 /* 135 */ "stl_prefix ::= seltablist joinop",
 /* 136 */ "stl_prefix ::=",
 /* 137 */ "seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt",
 /* 138 */ "seltablist ::= stl_prefix nm dbnm LP exprlist RP as on_opt using_opt",
 /* 139 */ "seltablist ::= stl_prefix LP select RP as on_opt using_opt",
 /* 140 */ "seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt",
 /* 141 */ "dbnm ::=",
 /* 142 */ "dbnm ::= DOT nm",
 /* 143 */ "fullname ::= nm dbnm",
 /* 144 */ "joinop ::= COMMA|JOIN",
 /* 145 */ "joinop ::= JOIN_KW JOIN",
 /* 146 */ "joinop ::= JOIN_KW nm JOIN",
 /* 147 */ "joinop ::= JOIN_KW nm nm JOIN",
 /* 148 */ "on_opt ::= ON expr",
 /* 149 */ "on_opt ::=",
 /* 150 */ "indexed_opt ::=",
 /* 151 */ "indexed_opt ::= INDEXED BY nm",
 /* 152 */ "indexed_opt ::= NOT INDEXED",
 /* 153 */ "using_opt ::= USING LP idlist RP",
 /* 154 */ "using_opt ::=",
 /* 155 */ "orderby_opt ::=",
 /* 156 */ "orderby_opt ::= ORDER BY sortlist",
 /* 157 */ "sortlist ::= sortlist COMMA expr sortorder",
 /* 158 */ "sortlist ::= expr sortorder",
 /* 159 */ "sortorder ::= ASC",
 /* 160 */ "sortorder ::= DESC",
 /* 161 */ "sortorder ::=",
 /* 162 */ "groupby_opt ::=",
 /* 163 */ "groupby_opt ::= GROUP BY nexprlist",
 /* 164 */ "having_opt ::=",
 /* 165 */ "having_opt ::= HAVING expr",
 /* 166 */ "limit_opt ::=",
 /* 167 */ "limit_opt ::= LIMIT expr",
 /* 168 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /* 169 */ "limit_opt ::= LIMIT expr COMMA expr",
 /* 170 */ "cmd ::= with DELETE FROM fullname indexed_opt where_opt",
 /* 171 */ "where_opt ::=",
 /* 172 */ "where_opt ::= WHERE expr",
 /* 173 */ "cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt",
 /* 174 */ "setlist ::= setlist COMMA nm EQ expr",
 /* 175 */ "setlist ::= nm EQ expr",
 /* 176 */ "cmd ::= with insert_cmd INTO fullname idlist_opt select",
 /* 177 */ "cmd ::= with insert_cmd INTO fullname idlist_opt DEFAULT VALUES",
 /* 178 */ "insert_cmd ::= INSERT orconf",
 /* 179 */ "insert_cmd ::= REPLACE",
 /* 180 */ "idlist_opt ::=",
 /* 181 */ "idlist_opt ::= LP idlist RP",
 /* 182 */ "idlist ::= idlist COMMA nm",
 /* 183 */ "idlist ::= nm",
 /* 184 */ "expr ::= term",
 /* 185 */ "expr ::= LP expr RP",
 /* 186 */ "term ::= NULL",
 /* 187 */ "expr ::= ID|INDEXED",
 /* 188 */ "expr ::= JOIN_KW",
 /* 189 */ "expr ::= nm DOT nm",
 /* 190 */ "expr ::= nm DOT nm DOT nm",
 /* 191 */ "term ::= INTEGER|FLOAT|BLOB",
 /* 192 */ "term ::= STRING",
 /* 193 */ "expr ::= VARIABLE",
 /* 194 */ "expr ::= expr COLLATE ID|STRING",
 /* 195 */ "expr ::= CAST LP expr AS typetoken RP",
 /* 196 */ "expr ::= ID|INDEXED LP distinct exprlist RP",
 /* 197 */ "expr ::= ID|INDEXED LP STAR RP",
 /* 198 */ "term ::= CTIME_KW",
 /* 199 */ "expr ::= expr AND expr",
 /* 200 */ "expr ::= expr OR expr",
 /* 201 */ "expr ::= expr LT|GT|GE|LE expr",
 /* 202 */ "expr ::= expr EQ|NE expr",
 /* 203 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /* 204 */ "expr ::= expr PLUS|MINUS expr",
 /* 205 */ "expr ::= expr STAR|SLASH|REM expr",
 /* 206 */ "expr ::= expr CONCAT expr",
 /* 207 */ "likeop ::= LIKE_KW|MATCH",
 /* 208 */ "likeop ::= NOT LIKE_KW|MATCH",
 /* 209 */ "expr ::= expr likeop expr",
 /* 210 */ "expr ::= expr likeop expr ESCAPE expr",
 /* 211 */ "expr ::= expr ISNULL|NOTNULL",
 /* 212 */ "expr ::= expr NOT NULL",
 /* 213 */ "expr ::= expr IS expr",
 /* 214 */ "expr ::= expr IS NOT expr",
 /* 215 */ "expr ::= NOT expr",
 /* 216 */ "expr ::= BITNOT expr",
 /* 217 */ "expr ::= MINUS expr",
 /* 218 */ "expr ::= PLUS expr",
 /* 219 */ "between_op ::= BETWEEN",
 /* 220 */ "between_op ::= NOT BETWEEN",
 /* 221 */ "expr ::= expr between_op expr AND expr",
 /* 222 */ "in_op ::= IN",
 /* 223 */ "in_op ::= NOT IN",
 /* 224 */ "expr ::= expr in_op LP exprlist RP",
 /* 225 */ "expr ::= LP select RP",
 /* 226 */ "expr ::= expr in_op LP select RP",
 /* 227 */ "expr ::= expr in_op nm dbnm",
 /* 228 */ "expr ::= EXISTS LP select RP",
 /* 229 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 230 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 231 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 232 */ "case_else ::= ELSE expr",
 /* 233 */ "case_else ::=",
 /* 234 */ "case_operand ::= expr",
 /* 235 */ "case_operand ::=",
 /* 236 */ "exprlist ::= nexprlist",
 /* 237 */ "exprlist ::=",
 /* 238 */ "nexprlist ::= nexprlist COMMA expr",
 /* 239 */ "nexprlist ::= expr",
 /* 240 */ "cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt",
 /* 241 */ "uniqueflag ::= UNIQUE",
 /* 242 */ "uniqueflag ::=",
 /* 243 */ "eidlist_opt ::=",
 /* 244 */ "eidlist_opt ::= LP eidlist RP",
 /* 245 */ "eidlist ::= eidlist COMMA nm collate sortorder",
 /* 246 */ "eidlist ::= nm collate sortorder",
 /* 247 */ "collate ::=",
 /* 248 */ "collate ::= COLLATE ID|STRING",
 /* 249 */ "cmd ::= DROP INDEX ifexists fullname",
 /* 250 */ "cmd ::= VACUUM",
 /* 251 */ "cmd ::= VACUUM nm",
 /* 252 */ "cmd ::= PRAGMA nm dbnm",
 /* 253 */ "cmd ::= PRAGMA nm dbnm EQ nmnum",
 /* 254 */ "cmd ::= PRAGMA nm dbnm LP nmnum RP",
 /* 255 */ "cmd ::= PRAGMA nm dbnm EQ minus_num",
 /* 256 */ "cmd ::= PRAGMA nm dbnm LP minus_num RP",
 /* 257 */ "nmnum ::= plus_num",
 /* 258 */ "nmnum ::= nm",
 /* 259 */ "nmnum ::= ON",
 /* 260 */ "nmnum ::= DELETE",
 /* 261 */ "nmnum ::= DEFAULT",
 /* 262 */ "plus_num ::= PLUS INTEGER|FLOAT",
 /* 263 */ "plus_num ::= INTEGER|FLOAT",
 /* 264 */ "minus_num ::= MINUS INTEGER|FLOAT",
 /* 265 */ "cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END",
 /* 266 */ "trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause",
 /* 267 */ "trigger_time ::= BEFORE",
 /* 268 */ "trigger_time ::= AFTER",
 /* 269 */ "trigger_time ::= INSTEAD OF",
 /* 270 */ "trigger_time ::=",
 /* 271 */ "trigger_event ::= DELETE|INSERT",
 /* 272 */ "trigger_event ::= UPDATE",
 /* 273 */ "trigger_event ::= UPDATE OF idlist",
 /* 274 */ "foreach_clause ::=",
 /* 275 */ "foreach_clause ::= FOR EACH ROW",
 /* 276 */ "when_clause ::=",
 /* 277 */ "when_clause ::= WHEN expr",
 /* 278 */ "trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI",
 /* 279 */ "trigger_cmd_list ::= trigger_cmd SEMI",
 /* 280 */ "trnm ::= nm",
 /* 281 */ "trnm ::= nm DOT nm",
 /* 282 */ "tridxby ::=",
 /* 283 */ "tridxby ::= INDEXED BY nm",
 /* 284 */ "tridxby ::= NOT INDEXED",
 /* 285 */ "trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt",
 /* 286 */ "trigger_cmd ::= insert_cmd INTO trnm idlist_opt select",
 /* 287 */ "trigger_cmd ::= DELETE FROM trnm tridxby where_opt",
 /* 288 */ "trigger_cmd ::= select",
 /* 289 */ "expr ::= RAISE LP IGNORE RP",
 /* 290 */ "expr ::= RAISE LP raisetype COMMA nm RP",
 /* 291 */ "raisetype ::= ROLLBACK",
 /* 292 */ "raisetype ::= ABORT",
 /* 293 */ "raisetype ::= FAIL",
 /* 294 */ "cmd ::= DROP TRIGGER ifexists fullname",
 /* 295 */ "cmd ::= ATTACH database_kw_opt expr AS expr key_opt",
 /* 296 */ "cmd ::= DETACH database_kw_opt expr",
 /* 297 */ "key_opt ::=",
 /* 298 */ "key_opt ::= KEY expr",
 /* 299 */ "database_kw_opt ::= DATABASE",
 /* 300 */ "database_kw_opt ::=",
 /* 301 */ "cmd ::= REINDEX",
 /* 302 */ "cmd ::= REINDEX nm dbnm",
 /* 303 */ "cmd ::= ANALYZE",
 /* 304 */ "cmd ::= ANALYZE nm dbnm",
 /* 305 */ "cmd ::= ALTER TABLE fullname RENAME TO nm",
 /* 306 */ "cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column",
 /* 307 */ "add_column_fullname ::= fullname",
 /* 308 */ "kwcolumn_opt ::=",
 /* 309 */ "kwcolumn_opt ::= COLUMNKW",
 /* 310 */ "cmd ::= create_vtab",
 /* 311 */ "cmd ::= create_vtab LP vtabarglist RP",
 /* 312 */ "create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm",
 /* 313 */ "vtabarglist ::= vtabarg",
 /* 314 */ "vtabarglist ::= vtabarglist COMMA vtabarg",
 /* 315 */ "vtabarg ::=",
 /* 316 */ "vtabarg ::= vtabarg vtabargtoken",
 /* 317 */ "vtabargtoken ::= ANY",
 /* 318 */ "vtabargtoken ::= lp anylist RP",
 /* 319 */ "lp ::= LP",
 /* 320 */ "anylist ::=",
 /* 321 */ "anylist ::= anylist LP anylist RP",
 /* 322 */ "anylist ::= anylist ANY",
 /* 323 */ "with ::=",
 /* 324 */ "with ::= WITH wqlist",
 /* 325 */ "with ::= WITH RECURSIVE wqlist",
 /* 326 */ "wqlist ::= nm eidlist_opt AS LP select RP",
 /* 327 */ "wqlist ::= wqlist COMMA nm eidlist_opt AS LP select RP",
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
** second argument to sqlite3ParserAlloc() below.  This can be changed by
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
** to sqlite3Parser and sqlite3ParserFree.
*/
void *sqlite3ParserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
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
  sqlite3ParserARG_FETCH;
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
    case 163: /* select */
    case 196: /* selectnowith */
    case 197: /* oneselect */
    case 208: /* values */
{
#line 428 "parse.y"
sqlite3SelectDelete(pParse->db, (yypminor->yy387));
#line 1506 "parse.c"
}
      break;
    case 174: /* term */
    case 175: /* expr */
{
#line 845 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy118).pExpr);
#line 1514 "parse.c"
}
      break;
    case 179: /* eidlist_opt */
    case 188: /* sortlist */
    case 189: /* eidlist */
    case 201: /* selcollist */
    case 204: /* groupby_opt */
    case 206: /* orderby_opt */
    case 209: /* nexprlist */
    case 210: /* exprlist */
    case 211: /* sclp */
    case 220: /* setlist */
    case 227: /* case_exprlist */
{
#line 1272 "parse.y"
sqlite3ExprListDelete(pParse->db, (yypminor->yy322));
#line 1531 "parse.c"
}
      break;
    case 195: /* fullname */
    case 202: /* from */
    case 213: /* seltablist */
    case 214: /* stl_prefix */
{
#line 659 "parse.y"
sqlite3SrcListDelete(pParse->db, (yypminor->yy259));
#line 1541 "parse.c"
}
      break;
    case 198: /* with */
    case 251: /* wqlist */
{
#line 1550 "parse.y"
sqlite3WithDelete(pParse->db, (yypminor->yy451));
#line 1549 "parse.c"
}
      break;
    case 203: /* where_opt */
    case 205: /* having_opt */
    case 217: /* on_opt */
    case 226: /* case_operand */
    case 228: /* case_else */
    case 237: /* when_clause */
    case 242: /* key_opt */
{
#line 772 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy314));
#line 1562 "parse.c"
}
      break;
    case 218: /* using_opt */
    case 219: /* idlist */
    case 222: /* idlist_opt */
{
#line 690 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy384));
#line 1571 "parse.c"
}
      break;
    case 233: /* trigger_cmd_list */
    case 238: /* trigger_cmd */
{
#line 1386 "parse.y"
sqlite3DeleteTriggerStep(pParse->db, (yypminor->yy203));
#line 1579 "parse.c"
}
      break;
    case 235: /* trigger_event */
{
#line 1372 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy90).b);
#line 1586 "parse.c"
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
void sqlite3ParserFree(
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
int sqlite3ParserStackPeak(void *p){
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
   sqlite3ParserARG_FETCH;
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
#line 37 "parse.y"

  UNUSED_PARAMETER(yypMinor); /* Silence some compiler warnings */
  sqlite3ErrorMsg(pParse, "parser stack overflow");
#line 1763 "parse.c"
/******** End %stack_overflow code ********************************************/
   sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
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
  { 144, 1 },
  { 145, 2 },
  { 145, 1 },
  { 146, 1 },
  { 146, 3 },
  { 147, 0 },
  { 147, 1 },
  { 147, 3 },
  { 148, 1 },
  { 149, 3 },
  { 151, 0 },
  { 151, 1 },
  { 151, 2 },
  { 150, 0 },
  { 150, 1 },
  { 150, 1 },
  { 150, 1 },
  { 149, 2 },
  { 149, 2 },
  { 149, 2 },
  { 153, 1 },
  { 153, 0 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 2 },
  { 154, 6 },
  { 156, 1 },
  { 158, 0 },
  { 158, 3 },
  { 157, 1 },
  { 157, 0 },
  { 155, 5 },
  { 155, 2 },
  { 162, 0 },
  { 162, 2 },
  { 160, 3 },
  { 160, 1 },
  { 164, 3 },
  { 165, 1 },
  { 152, 1 },
  { 152, 1 },
  { 152, 1 },
  { 166, 0 },
  { 166, 1 },
  { 168, 1 },
  { 168, 4 },
  { 168, 6 },
  { 169, 1 },
  { 169, 2 },
  { 170, 1 },
  { 170, 1 },
  { 167, 2 },
  { 167, 0 },
  { 173, 2 },
  { 173, 2 },
  { 173, 4 },
  { 173, 3 },
  { 173, 3 },
  { 173, 2 },
  { 173, 2 },
  { 173, 3 },
  { 173, 5 },
  { 173, 2 },
  { 173, 4 },
  { 173, 4 },
  { 173, 1 },
  { 173, 2 },
  { 178, 0 },
  { 178, 1 },
  { 180, 0 },
  { 180, 2 },
  { 182, 2 },
  { 182, 3 },
  { 182, 3 },
  { 182, 3 },
  { 183, 2 },
  { 183, 2 },
  { 183, 1 },
  { 183, 1 },
  { 183, 2 },
  { 181, 3 },
  { 181, 2 },
  { 184, 0 },
  { 184, 2 },
  { 184, 2 },
  { 161, 0 },
  { 161, 2 },
  { 185, 3 },
  { 185, 1 },
  { 186, 1 },
  { 186, 0 },
  { 187, 2 },
  { 187, 7 },
  { 187, 5 },
  { 187, 5 },
  { 187, 10 },
  { 190, 0 },
  { 190, 1 },
  { 176, 0 },
  { 176, 3 },
  { 191, 0 },
  { 191, 2 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 149, 4 },
  { 194, 2 },
  { 194, 0 },
  { 149, 9 },
  { 149, 4 },
  { 149, 1 },
  { 163, 2 },
  { 196, 1 },
  { 196, 3 },
  { 199, 1 },
  { 199, 2 },
  { 199, 1 },
  { 197, 9 },
  { 197, 1 },
  { 208, 4 },
  { 208, 5 },
  { 200, 1 },
  { 200, 1 },
  { 200, 0 },
  { 211, 2 },
  { 211, 0 },
  { 201, 3 },
  { 201, 2 },
  { 201, 4 },
  { 212, 2 },
  { 212, 1 },
  { 212, 0 },
  { 202, 0 },
  { 202, 2 },
  { 214, 2 },
  { 214, 0 },
  { 213, 7 },
  { 213, 9 },
  { 213, 7 },
  { 213, 7 },
  { 159, 0 },
  { 159, 2 },
  { 195, 2 },
  { 215, 1 },
  { 215, 2 },
  { 215, 3 },
  { 215, 4 },
  { 217, 2 },
  { 217, 0 },
  { 216, 0 },
  { 216, 3 },
  { 216, 2 },
  { 218, 4 },
  { 218, 0 },
  { 206, 0 },
  { 206, 3 },
  { 188, 4 },
  { 188, 2 },
  { 177, 1 },
  { 177, 1 },
  { 177, 0 },
  { 204, 0 },
  { 204, 3 },
  { 205, 0 },
  { 205, 2 },
  { 207, 0 },
  { 207, 2 },
  { 207, 4 },
  { 207, 4 },
  { 149, 6 },
  { 203, 0 },
  { 203, 2 },
  { 149, 8 },
  { 220, 5 },
  { 220, 3 },
  { 149, 6 },
  { 149, 7 },
  { 221, 2 },
  { 221, 1 },
  { 222, 0 },
  { 222, 3 },
  { 219, 3 },
  { 219, 1 },
  { 175, 1 },
  { 175, 3 },
  { 174, 1 },
  { 175, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 5 },
  { 174, 1 },
  { 174, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 6 },
  { 175, 5 },
  { 175, 4 },
  { 174, 1 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 223, 1 },
  { 223, 2 },
  { 175, 3 },
  { 175, 5 },
  { 175, 2 },
  { 175, 3 },
  { 175, 3 },
  { 175, 4 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 224, 1 },
  { 224, 2 },
  { 175, 5 },
  { 225, 1 },
  { 225, 2 },
  { 175, 5 },
  { 175, 3 },
  { 175, 5 },
  { 175, 4 },
  { 175, 4 },
  { 175, 5 },
  { 227, 5 },
  { 227, 4 },
  { 228, 2 },
  { 228, 0 },
  { 226, 1 },
  { 226, 0 },
  { 210, 1 },
  { 210, 0 },
  { 209, 3 },
  { 209, 1 },
  { 149, 12 },
  { 229, 1 },
  { 229, 0 },
  { 179, 0 },
  { 179, 3 },
  { 189, 5 },
  { 189, 3 },
  { 230, 0 },
  { 230, 2 },
  { 149, 4 },
  { 149, 1 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 6 },
  { 149, 5 },
  { 149, 6 },
  { 231, 1 },
  { 231, 1 },
  { 231, 1 },
  { 231, 1 },
  { 231, 1 },
  { 171, 2 },
  { 171, 1 },
  { 172, 2 },
  { 149, 5 },
  { 232, 11 },
  { 234, 1 },
  { 234, 1 },
  { 234, 2 },
  { 234, 0 },
  { 235, 1 },
  { 235, 1 },
  { 235, 3 },
  { 236, 0 },
  { 236, 3 },
  { 237, 0 },
  { 237, 2 },
  { 233, 3 },
  { 233, 2 },
  { 239, 1 },
  { 239, 3 },
  { 240, 0 },
  { 240, 3 },
  { 240, 2 },
  { 238, 7 },
  { 238, 5 },
  { 238, 5 },
  { 238, 1 },
  { 175, 4 },
  { 175, 6 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 149, 4 },
  { 149, 6 },
  { 149, 3 },
  { 242, 0 },
  { 242, 2 },
  { 241, 1 },
  { 241, 0 },
  { 149, 1 },
  { 149, 3 },
  { 149, 1 },
  { 149, 3 },
  { 149, 6 },
  { 149, 6 },
  { 243, 1 },
  { 244, 0 },
  { 244, 1 },
  { 149, 1 },
  { 149, 4 },
  { 245, 8 },
  { 246, 1 },
  { 246, 3 },
  { 247, 0 },
  { 247, 2 },
  { 248, 1 },
  { 248, 3 },
  { 249, 1 },
  { 250, 0 },
  { 250, 4 },
  { 250, 2 },
  { 198, 0 },
  { 198, 2 },
  { 198, 3 },
  { 251, 6 },
  { 251, 8 },
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
  sqlite3ParserARG_FETCH;
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
      case 6: /* explain ::= EXPLAIN */
#line 128 "parse.y"
{ pParse->explain = 1; }
#line 2202 "parse.c"
        break;
      case 7: /* explain ::= EXPLAIN QUERY PLAN */
#line 129 "parse.y"
{ pParse->explain = 2; }
#line 2207 "parse.c"
        break;
      case 8: /* cmdx ::= cmd */
#line 131 "parse.y"
{ sqlite3FinishCoding(pParse); }
#line 2212 "parse.c"
        break;
      case 9: /* cmd ::= BEGIN transtype trans_opt */
#line 136 "parse.y"
{sqlite3BeginTransaction(pParse, yymsp[-1].minor.yy4);}
#line 2217 "parse.c"
        break;
      case 13: /* transtype ::= */
#line 141 "parse.y"
{yygotominor.yy4 = TK_DEFERRED;}
#line 2222 "parse.c"
        break;
      case 14: /* transtype ::= DEFERRED */
      case 15: /* transtype ::= IMMEDIATE */ yytestcase(yyruleno==15);
      case 16: /* transtype ::= EXCLUSIVE */ yytestcase(yyruleno==16);
      case 115: /* multiselect_op ::= UNION */ yytestcase(yyruleno==115);
      case 117: /* multiselect_op ::= EXCEPT|INTERSECT */ yytestcase(yyruleno==117);
#line 142 "parse.y"
{yygotominor.yy4 = yymsp[0].major;}
#line 2231 "parse.c"
        break;
      case 17: /* cmd ::= COMMIT trans_opt */
      case 18: /* cmd ::= END trans_opt */ yytestcase(yyruleno==18);
#line 145 "parse.y"
{sqlite3CommitTransaction(pParse);}
#line 2237 "parse.c"
        break;
      case 19: /* cmd ::= ROLLBACK trans_opt */
#line 147 "parse.y"
{sqlite3RollbackTransaction(pParse);}
#line 2242 "parse.c"
        break;
      case 22: /* cmd ::= SAVEPOINT nm */
#line 151 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_BEGIN, &yymsp[0].minor.yy0);
}
#line 2249 "parse.c"
        break;
      case 23: /* cmd ::= RELEASE savepoint_opt nm */
#line 154 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_RELEASE, &yymsp[0].minor.yy0);
}
#line 2256 "parse.c"
        break;
      case 24: /* cmd ::= ROLLBACK trans_opt TO savepoint_opt nm */
#line 157 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_ROLLBACK, &yymsp[0].minor.yy0);
}
#line 2263 "parse.c"
        break;
      case 26: /* create_table ::= createkw temp TABLE ifnotexists nm dbnm */
#line 164 "parse.y"
{
   sqlite3StartTable(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,yymsp[-4].minor.yy4,0,0,yymsp[-2].minor.yy4);
}
#line 2270 "parse.c"
        break;
      case 27: /* createkw ::= CREATE */
#line 167 "parse.y"
{
  disableLookaside(pParse);
  yygotominor.yy0 = yymsp[0].minor.yy0;
}
#line 2278 "parse.c"
        break;
      case 28: /* ifnotexists ::= */
      case 31: /* temp ::= */ yytestcase(yyruleno==31);
      case 34: /* table_options ::= */ yytestcase(yyruleno==34);
      case 68: /* autoinc ::= */ yytestcase(yyruleno==68);
      case 81: /* defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt */ yytestcase(yyruleno==81);
      case 83: /* init_deferred_pred_opt ::= */ yytestcase(yyruleno==83);
      case 85: /* init_deferred_pred_opt ::= INITIALLY IMMEDIATE */ yytestcase(yyruleno==85);
      case 97: /* defer_subclause_opt ::= */ yytestcase(yyruleno==97);
      case 108: /* ifexists ::= */ yytestcase(yyruleno==108);
      case 124: /* distinct ::= */ yytestcase(yyruleno==124);
      case 219: /* between_op ::= BETWEEN */ yytestcase(yyruleno==219);
      case 222: /* in_op ::= IN */ yytestcase(yyruleno==222);
      case 247: /* collate ::= */ yytestcase(yyruleno==247);
#line 172 "parse.y"
{yygotominor.yy4 = 0;}
#line 2295 "parse.c"
        break;
      case 29: /* ifnotexists ::= IF NOT EXISTS */
      case 30: /* temp ::= TEMP */ yytestcase(yyruleno==30);
      case 69: /* autoinc ::= AUTOINCR */ yytestcase(yyruleno==69);
      case 84: /* init_deferred_pred_opt ::= INITIALLY DEFERRED */ yytestcase(yyruleno==84);
      case 107: /* ifexists ::= IF EXISTS */ yytestcase(yyruleno==107);
      case 220: /* between_op ::= NOT BETWEEN */ yytestcase(yyruleno==220);
      case 223: /* in_op ::= NOT IN */ yytestcase(yyruleno==223);
      case 248: /* collate ::= COLLATE ID|STRING */ yytestcase(yyruleno==248);
#line 173 "parse.y"
{yygotominor.yy4 = 1;}
#line 2307 "parse.c"
        break;
      case 32: /* create_table_args ::= LP columnlist conslist_opt RP table_options */
#line 179 "parse.y"
{
  sqlite3EndTable(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,yymsp[0].minor.yy4,0);
}
#line 2314 "parse.c"
        break;
      case 33: /* create_table_args ::= AS select */
#line 182 "parse.y"
{
  sqlite3EndTable(pParse,0,0,0,yymsp[0].minor.yy387);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy387);
}
#line 2322 "parse.c"
        break;
      case 35: /* table_options ::= WITHOUT nm */
#line 188 "parse.y"
{
  if( yymsp[0].minor.yy0.n==5 && sqlite3_strnicmp(yymsp[0].minor.yy0.z,"rowid",5)==0 ){
    yygotominor.yy4 = TF_WithoutRowid | TF_NoVisibleRowid;
  }else{
    yygotominor.yy4 = 0;
    sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
  }
}
#line 2334 "parse.c"
        break;
      case 38: /* column ::= columnid type carglist */
#line 204 "parse.y"
{
  yygotominor.yy0.z = yymsp[-2].minor.yy0.z;
  yygotominor.yy0.n = (int)(pParse->sLastToken.z-yymsp[-2].minor.yy0.z) + pParse->sLastToken.n;
}
#line 2342 "parse.c"
        break;
      case 39: /* columnid ::= nm */
#line 208 "parse.y"
{
  sqlite3AddColumn(pParse,&yymsp[0].minor.yy0);
  yygotominor.yy0 = yymsp[0].minor.yy0;
  pParse->constraintName.n = 0;
}
#line 2351 "parse.c"
        break;
      case 40: /* nm ::= ID|INDEXED */
      case 41: /* nm ::= STRING */ yytestcase(yyruleno==41);
      case 42: /* nm ::= JOIN_KW */ yytestcase(yyruleno==42);
      case 45: /* typetoken ::= typename */ yytestcase(yyruleno==45);
      case 48: /* typename ::= ID|STRING */ yytestcase(yyruleno==48);
      case 130: /* as ::= AS nm */ yytestcase(yyruleno==130);
      case 131: /* as ::= ID|STRING */ yytestcase(yyruleno==131);
      case 142: /* dbnm ::= DOT nm */ yytestcase(yyruleno==142);
      case 151: /* indexed_opt ::= INDEXED BY nm */ yytestcase(yyruleno==151);
      case 257: /* nmnum ::= plus_num */ yytestcase(yyruleno==257);
      case 258: /* nmnum ::= nm */ yytestcase(yyruleno==258);
      case 259: /* nmnum ::= ON */ yytestcase(yyruleno==259);
      case 260: /* nmnum ::= DELETE */ yytestcase(yyruleno==260);
      case 261: /* nmnum ::= DEFAULT */ yytestcase(yyruleno==261);
      case 262: /* plus_num ::= PLUS INTEGER|FLOAT */ yytestcase(yyruleno==262);
      case 263: /* plus_num ::= INTEGER|FLOAT */ yytestcase(yyruleno==263);
      case 264: /* minus_num ::= MINUS INTEGER|FLOAT */ yytestcase(yyruleno==264);
      case 280: /* trnm ::= nm */ yytestcase(yyruleno==280);
#line 268 "parse.y"
{yygotominor.yy0 = yymsp[0].minor.yy0;}
#line 2373 "parse.c"
        break;
      case 44: /* type ::= typetoken */
#line 278 "parse.y"
{sqlite3AddColumnType(pParse,&yymsp[0].minor.yy0);}
#line 2378 "parse.c"
        break;
      case 46: /* typetoken ::= typename LP signed RP */
#line 280 "parse.y"
{
  yygotominor.yy0.z = yymsp[-3].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy0.z);
}
#line 2386 "parse.c"
        break;
      case 47: /* typetoken ::= typename LP signed COMMA signed RP */
#line 284 "parse.y"
{
  yygotominor.yy0.z = yymsp[-5].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy0.z);
}
#line 2394 "parse.c"
        break;
      case 49: /* typename ::= typename ID|STRING */
#line 290 "parse.y"
{yygotominor.yy0.z=yymsp[-1].minor.yy0.z; yygotominor.yy0.n=yymsp[0].minor.yy0.n+(int)(yymsp[0].minor.yy0.z-yymsp[-1].minor.yy0.z);}
#line 2399 "parse.c"
        break;
      case 54: /* ccons ::= CONSTRAINT nm */
      case 92: /* tcons ::= CONSTRAINT nm */ yytestcase(yyruleno==92);
#line 299 "parse.y"
{pParse->constraintName = yymsp[0].minor.yy0;}
#line 2405 "parse.c"
        break;
      case 55: /* ccons ::= DEFAULT term */
      case 57: /* ccons ::= DEFAULT PLUS term */ yytestcase(yyruleno==57);
#line 300 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[0].minor.yy118);}
#line 2411 "parse.c"
        break;
      case 56: /* ccons ::= DEFAULT LP expr RP */
#line 301 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[-1].minor.yy118);}
#line 2416 "parse.c"
        break;
      case 58: /* ccons ::= DEFAULT MINUS term */
#line 303 "parse.y"
{
  ExprSpan v;
  v.pExpr = sqlite3PExpr(pParse, TK_UMINUS, yymsp[0].minor.yy118.pExpr, 0, 0);
  v.zStart = yymsp[-1].minor.yy0.z;
  v.zEnd = yymsp[0].minor.yy118.zEnd;
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2427 "parse.c"
        break;
      case 59: /* ccons ::= DEFAULT ID|INDEXED */
#line 310 "parse.y"
{
  ExprSpan v;
  spanExpr(&v, pParse, TK_STRING, &yymsp[0].minor.yy0);
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2436 "parse.c"
        break;
      case 61: /* ccons ::= NOT NULL onconf */
#line 320 "parse.y"
{sqlite3AddNotNull(pParse, yymsp[0].minor.yy4);}
#line 2441 "parse.c"
        break;
      case 62: /* ccons ::= PRIMARY KEY sortorder onconf autoinc */
#line 322 "parse.y"
{sqlite3AddPrimaryKey(pParse,0,yymsp[-1].minor.yy4,yymsp[0].minor.yy4,yymsp[-2].minor.yy4);}
#line 2446 "parse.c"
        break;
      case 63: /* ccons ::= UNIQUE onconf */
#line 323 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,0,yymsp[0].minor.yy4,0,0,0,0);}
#line 2451 "parse.c"
        break;
      case 64: /* ccons ::= CHECK LP expr RP */
#line 324 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-1].minor.yy118.pExpr);}
#line 2456 "parse.c"
        break;
      case 65: /* ccons ::= REFERENCES nm eidlist_opt refargs */
#line 326 "parse.y"
{sqlite3CreateForeignKey(pParse,0,&yymsp[-2].minor.yy0,yymsp[-1].minor.yy322,yymsp[0].minor.yy4);}
#line 2461 "parse.c"
        break;
      case 66: /* ccons ::= defer_subclause */
#line 327 "parse.y"
{sqlite3DeferForeignKey(pParse,yymsp[0].minor.yy4);}
#line 2466 "parse.c"
        break;
      case 67: /* ccons ::= COLLATE ID|STRING */
#line 328 "parse.y"
{sqlite3AddCollateType(pParse, &yymsp[0].minor.yy0);}
#line 2471 "parse.c"
        break;
      case 70: /* refargs ::= */
#line 341 "parse.y"
{ yygotominor.yy4 = OE_None*0x0101; /* EV: R-19803-45884 */}
#line 2476 "parse.c"
        break;
      case 71: /* refargs ::= refargs refarg */
#line 342 "parse.y"
{ yygotominor.yy4 = (yymsp[-1].minor.yy4 & ~yymsp[0].minor.yy215.mask) | yymsp[0].minor.yy215.value; }
#line 2481 "parse.c"
        break;
      case 72: /* refarg ::= MATCH nm */
      case 73: /* refarg ::= ON INSERT refact */ yytestcase(yyruleno==73);
#line 344 "parse.y"
{ yygotominor.yy215.value = 0;     yygotominor.yy215.mask = 0x000000; }
#line 2487 "parse.c"
        break;
      case 74: /* refarg ::= ON DELETE refact */
#line 346 "parse.y"
{ yygotominor.yy215.value = yymsp[0].minor.yy4;     yygotominor.yy215.mask = 0x0000ff; }
#line 2492 "parse.c"
        break;
      case 75: /* refarg ::= ON UPDATE refact */
#line 347 "parse.y"
{ yygotominor.yy215.value = yymsp[0].minor.yy4<<8;  yygotominor.yy215.mask = 0x00ff00; }
#line 2497 "parse.c"
        break;
      case 76: /* refact ::= SET NULL */
#line 349 "parse.y"
{ yygotominor.yy4 = OE_SetNull;  /* EV: R-33326-45252 */}
#line 2502 "parse.c"
        break;
      case 77: /* refact ::= SET DEFAULT */
#line 350 "parse.y"
{ yygotominor.yy4 = OE_SetDflt;  /* EV: R-33326-45252 */}
#line 2507 "parse.c"
        break;
      case 78: /* refact ::= CASCADE */
#line 351 "parse.y"
{ yygotominor.yy4 = OE_Cascade;  /* EV: R-33326-45252 */}
#line 2512 "parse.c"
        break;
      case 79: /* refact ::= RESTRICT */
#line 352 "parse.y"
{ yygotominor.yy4 = OE_Restrict; /* EV: R-33326-45252 */}
#line 2517 "parse.c"
        break;
      case 80: /* refact ::= NO ACTION */
#line 353 "parse.y"
{ yygotominor.yy4 = OE_None;     /* EV: R-33326-45252 */}
#line 2522 "parse.c"
        break;
      case 82: /* defer_subclause ::= DEFERRABLE init_deferred_pred_opt */
      case 98: /* defer_subclause_opt ::= defer_subclause */ yytestcase(yyruleno==98);
      case 100: /* onconf ::= ON CONFLICT resolvetype */ yytestcase(yyruleno==100);
      case 102: /* orconf ::= OR resolvetype */ yytestcase(yyruleno==102);
      case 103: /* resolvetype ::= raisetype */ yytestcase(yyruleno==103);
      case 178: /* insert_cmd ::= INSERT orconf */ yytestcase(yyruleno==178);
#line 356 "parse.y"
{yygotominor.yy4 = yymsp[0].minor.yy4;}
#line 2532 "parse.c"
        break;
      case 86: /* conslist_opt ::= */
#line 362 "parse.y"
{yygotominor.yy0.n = 0; yygotominor.yy0.z = 0;}
#line 2537 "parse.c"
        break;
      case 87: /* conslist_opt ::= COMMA conslist */
#line 363 "parse.y"
{yygotominor.yy0 = yymsp[-1].minor.yy0;}
#line 2542 "parse.c"
        break;
      case 90: /* tconscomma ::= COMMA */
#line 366 "parse.y"
{pParse->constraintName.n = 0;}
#line 2547 "parse.c"
        break;
      case 93: /* tcons ::= PRIMARY KEY LP sortlist autoinc RP onconf */
#line 370 "parse.y"
{sqlite3AddPrimaryKey(pParse,yymsp[-3].minor.yy322,yymsp[0].minor.yy4,yymsp[-2].minor.yy4,0);}
#line 2552 "parse.c"
        break;
      case 94: /* tcons ::= UNIQUE LP sortlist RP onconf */
#line 372 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,yymsp[-2].minor.yy322,yymsp[0].minor.yy4,0,0,0,0);}
#line 2557 "parse.c"
        break;
      case 95: /* tcons ::= CHECK LP expr RP onconf */
#line 374 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-2].minor.yy118.pExpr);}
#line 2562 "parse.c"
        break;
      case 96: /* tcons ::= FOREIGN KEY LP eidlist RP REFERENCES nm eidlist_opt refargs defer_subclause_opt */
#line 376 "parse.y"
{
    sqlite3CreateForeignKey(pParse, yymsp[-6].minor.yy322, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy322, yymsp[-1].minor.yy4);
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy4);
}
#line 2570 "parse.c"
        break;
      case 99: /* onconf ::= */
      case 101: /* orconf ::= */ yytestcase(yyruleno==101);
#line 390 "parse.y"
{yygotominor.yy4 = OE_Default;}
#line 2576 "parse.c"
        break;
      case 104: /* resolvetype ::= IGNORE */
#line 395 "parse.y"
{yygotominor.yy4 = OE_Ignore;}
#line 2581 "parse.c"
        break;
      case 105: /* resolvetype ::= REPLACE */
      case 179: /* insert_cmd ::= REPLACE */ yytestcase(yyruleno==179);
#line 396 "parse.y"
{yygotominor.yy4 = OE_Replace;}
#line 2587 "parse.c"
        break;
      case 106: /* cmd ::= DROP TABLE ifexists fullname */
#line 400 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy259, 0, yymsp[-1].minor.yy4);
}
#line 2594 "parse.c"
        break;
      case 109: /* cmd ::= createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select */
#line 411 "parse.y"
{
  sqlite3CreateView(pParse, &yymsp[-8].minor.yy0, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy322, yymsp[0].minor.yy387, yymsp[-7].minor.yy4, yymsp[-5].minor.yy4);
}
#line 2601 "parse.c"
        break;
      case 110: /* cmd ::= DROP VIEW ifexists fullname */
#line 414 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy259, 1, yymsp[-1].minor.yy4);
}
#line 2608 "parse.c"
        break;
      case 111: /* cmd ::= select */
#line 421 "parse.y"
{
  SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0};
  sqlite3Select(pParse, yymsp[0].minor.yy387, &dest);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy387);
}
#line 2617 "parse.c"
        break;
      case 112: /* select ::= with selectnowith */
#line 458 "parse.y"
{
  Select *p = yymsp[0].minor.yy387;
  if( p ){
    p->pWith = yymsp[-1].minor.yy451;
    parserDoubleLinkSelect(pParse, p);
  }else{
    sqlite3WithDelete(pParse->db, yymsp[-1].minor.yy451);
  }
  yygotominor.yy387 = p;
}
#line 2631 "parse.c"
        break;
      case 113: /* selectnowith ::= oneselect */
      case 119: /* oneselect ::= values */ yytestcase(yyruleno==119);
#line 469 "parse.y"
{yygotominor.yy387 = yymsp[0].minor.yy387;}
#line 2637 "parse.c"
        break;
      case 114: /* selectnowith ::= selectnowith multiselect_op oneselect */
#line 471 "parse.y"
{
  Select *pRhs = yymsp[0].minor.yy387;
  Select *pLhs = yymsp[-2].minor.yy387;
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    parserDoubleLinkSelect(pParse, pRhs);
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)yymsp[-1].minor.yy4;
    pRhs->pPrior = pLhs;
    if( ALWAYS(pLhs) ) pLhs->selFlags &= ~SF_MultiValue;
    pRhs->selFlags &= ~SF_MultiValue;
    if( yymsp[-1].minor.yy4!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, pLhs);
  }
  yygotominor.yy387 = pRhs;
}
#line 2663 "parse.c"
        break;
      case 116: /* multiselect_op ::= UNION ALL */
#line 495 "parse.y"
{yygotominor.yy4 = TK_ALL;}
#line 2668 "parse.c"
        break;
      case 118: /* oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt */
#line 499 "parse.y"
{
  yygotominor.yy387 = sqlite3SelectNew(pParse,yymsp[-6].minor.yy322,yymsp[-5].minor.yy259,yymsp[-4].minor.yy314,yymsp[-3].minor.yy322,yymsp[-2].minor.yy314,yymsp[-1].minor.yy322,yymsp[-7].minor.yy4,yymsp[0].minor.yy292.pLimit,yymsp[0].minor.yy292.pOffset);
#if SELECTTRACE_ENABLED
  /* Populate the Select.zSelName[] string that is used to help with
  ** query planner debugging, to differentiate between multiple Select
  ** objects in a complex query.
  **
  ** If the SELECT keyword is immediately followed by a C-style comment
  ** then extract the first few alphanumeric characters from within that
  ** comment to be the zSelName value.  Otherwise, the label is #N where
  ** is an integer that is incremented with each SELECT statement seen.
  */
  if( yygotominor.yy387!=0 ){
    const char *z = yymsp[-8].minor.yy0.z+6;
    int i;
    sqlite3_snprintf(sizeof(yygotominor.yy387->zSelName), yygotominor.yy387->zSelName, "#%d",
                     ++pParse->nSelect);
    while( z[0]==' ' ) z++;
    if( z[0]=='/' && z[1]=='*' ){
      z += 2;
      while( z[0]==' ' ) z++;
      for(i=0; sqlite3Isalnum(z[i]); i++){}
      sqlite3_snprintf(sizeof(yygotominor.yy387->zSelName), yygotominor.yy387->zSelName, "%.*s", i, z);
    }
  }
#endif /* SELECTRACE_ENABLED */
}
#line 2699 "parse.c"
        break;
      case 120: /* values ::= VALUES LP nexprlist RP */
#line 530 "parse.y"
{
  yygotominor.yy387 = sqlite3SelectNew(pParse,yymsp[-1].minor.yy322,0,0,0,0,0,SF_Values,0,0);
}
#line 2706 "parse.c"
        break;
      case 121: /* values ::= values COMMA LP exprlist RP */
#line 533 "parse.y"
{
  Select *pRight, *pLeft = yymsp[-4].minor.yy387;
  pRight = sqlite3SelectNew(pParse,yymsp[-1].minor.yy322,0,0,0,0,0,SF_Values|SF_MultiValue,0,0);
  if( ALWAYS(pLeft) ) pLeft->selFlags &= ~SF_MultiValue;
  if( pRight ){
    pRight->op = TK_ALL;
    pLeft = yymsp[-4].minor.yy387;
    pRight->pPrior = pLeft;
    yygotominor.yy387 = pRight;
  }else{
    yygotominor.yy387 = pLeft;
  }
}
#line 2723 "parse.c"
        break;
      case 122: /* distinct ::= DISTINCT */
#line 551 "parse.y"
{yygotominor.yy4 = SF_Distinct;}
#line 2728 "parse.c"
        break;
      case 123: /* distinct ::= ALL */
#line 552 "parse.y"
{yygotominor.yy4 = SF_All;}
#line 2733 "parse.c"
        break;
      case 125: /* sclp ::= selcollist COMMA */
      case 244: /* eidlist_opt ::= LP eidlist RP */ yytestcase(yyruleno==244);
#line 564 "parse.y"
{yygotominor.yy322 = yymsp[-1].minor.yy322;}
#line 2739 "parse.c"
        break;
      case 126: /* sclp ::= */
      case 155: /* orderby_opt ::= */ yytestcase(yyruleno==155);
      case 162: /* groupby_opt ::= */ yytestcase(yyruleno==162);
      case 237: /* exprlist ::= */ yytestcase(yyruleno==237);
      case 243: /* eidlist_opt ::= */ yytestcase(yyruleno==243);
#line 565 "parse.y"
{yygotominor.yy322 = 0;}
#line 2748 "parse.c"
        break;
      case 127: /* selcollist ::= sclp expr as */
#line 566 "parse.y"
{
   yygotominor.yy322 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy322, yymsp[-1].minor.yy118.pExpr);
   if( yymsp[0].minor.yy0.n>0 ) sqlite3ExprListSetName(pParse, yygotominor.yy322, &yymsp[0].minor.yy0, 1);
   sqlite3ExprListSetSpan(pParse,yygotominor.yy322,&yymsp[-1].minor.yy118);
}
#line 2757 "parse.c"
        break;
      case 128: /* selcollist ::= sclp STAR */
#line 571 "parse.y"
{
  Expr *p = sqlite3Expr(pParse->db, TK_ASTERISK, 0);
  yygotominor.yy322 = sqlite3ExprListAppend(pParse, yymsp[-1].minor.yy322, p);
}
#line 2765 "parse.c"
        break;
      case 129: /* selcollist ::= sclp nm DOT STAR */
#line 575 "parse.y"
{
  Expr *pRight = sqlite3PExpr(pParse, TK_ASTERISK, 0, 0, &yymsp[0].minor.yy0);
  Expr *pLeft = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *pDot = sqlite3PExpr(pParse, TK_DOT, pLeft, pRight, 0);
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy322, pDot);
}
#line 2775 "parse.c"
        break;
      case 132: /* as ::= */
#line 588 "parse.y"
{yygotominor.yy0.n = 0;}
#line 2780 "parse.c"
        break;
      case 133: /* from ::= */
#line 600 "parse.y"
{yygotominor.yy259 = sqlite3DbMallocZero(pParse->db, sizeof(*yygotominor.yy259));}
#line 2785 "parse.c"
        break;
      case 134: /* from ::= FROM seltablist */
#line 601 "parse.y"
{
  yygotominor.yy259 = yymsp[0].minor.yy259;
  sqlite3SrcListShiftJoinType(yygotominor.yy259);
}
#line 2793 "parse.c"
        break;
      case 135: /* stl_prefix ::= seltablist joinop */
#line 609 "parse.y"
{
   yygotominor.yy259 = yymsp[-1].minor.yy259;
   if( ALWAYS(yygotominor.yy259 && yygotominor.yy259->nSrc>0) ) yygotominor.yy259->a[yygotominor.yy259->nSrc-1].fg.jointype = (u8)yymsp[0].minor.yy4;
}
#line 2801 "parse.c"
        break;
      case 136: /* stl_prefix ::= */
#line 613 "parse.y"
{yygotominor.yy259 = 0;}
#line 2806 "parse.c"
        break;
      case 137: /* seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt */
#line 615 "parse.y"
{
  yygotominor.yy259 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy259,&yymsp[-5].minor.yy0,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,0,yymsp[-1].minor.yy314,yymsp[0].minor.yy384);
  sqlite3SrcListIndexedBy(pParse, yygotominor.yy259, &yymsp[-2].minor.yy0);
}
#line 2814 "parse.c"
        break;
      case 138: /* seltablist ::= stl_prefix nm dbnm LP exprlist RP as on_opt using_opt */
#line 620 "parse.y"
{
  yygotominor.yy259 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-8].minor.yy259,&yymsp[-7].minor.yy0,&yymsp[-6].minor.yy0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy314,yymsp[0].minor.yy384);
  sqlite3SrcListFuncArgs(pParse, yygotominor.yy259, yymsp[-4].minor.yy322);
}
#line 2822 "parse.c"
        break;
      case 139: /* seltablist ::= stl_prefix LP select RP as on_opt using_opt */
#line 626 "parse.y"
{
    yygotominor.yy259 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy259,0,0,&yymsp[-2].minor.yy0,yymsp[-4].minor.yy387,yymsp[-1].minor.yy314,yymsp[0].minor.yy384);
  }
#line 2829 "parse.c"
        break;
      case 140: /* seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt */
#line 630 "parse.y"
{
    if( yymsp[-6].minor.yy259==0 && yymsp[-2].minor.yy0.n==0 && yymsp[-1].minor.yy314==0 && yymsp[0].minor.yy384==0 ){
      yygotominor.yy259 = yymsp[-4].minor.yy259;
    }else if( yymsp[-4].minor.yy259->nSrc==1 ){
      yygotominor.yy259 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy259,0,0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy314,yymsp[0].minor.yy384);
      if( yygotominor.yy259 ){
        struct SrcList_item *pNew = &yygotominor.yy259->a[yygotominor.yy259->nSrc-1];
        struct SrcList_item *pOld = yymsp[-4].minor.yy259->a;
        pNew->zName = pOld->zName;
        pNew->zDatabase = pOld->zDatabase;
        pNew->pSelect = pOld->pSelect;
        pOld->zName = pOld->zDatabase = 0;
        pOld->pSelect = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-4].minor.yy259);
    }else{
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(yymsp[-4].minor.yy259);
      pSubquery = sqlite3SelectNew(pParse,0,yymsp[-4].minor.yy259,0,0,0,0,SF_NestedFrom,0,0);
      yygotominor.yy259 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy259,0,0,&yymsp[-2].minor.yy0,pSubquery,yymsp[-1].minor.yy314,yymsp[0].minor.yy384);
    }
  }
#line 2855 "parse.c"
        break;
      case 141: /* dbnm ::= */
      case 150: /* indexed_opt ::= */ yytestcase(yyruleno==150);
#line 655 "parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=0;}
#line 2861 "parse.c"
        break;
      case 143: /* fullname ::= nm dbnm */
#line 660 "parse.y"
{yygotominor.yy259 = sqlite3SrcListAppend(pParse->db,0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);}
#line 2866 "parse.c"
        break;
      case 144: /* joinop ::= COMMA|JOIN */
#line 663 "parse.y"
{ yygotominor.yy4 = JT_INNER; }
#line 2871 "parse.c"
        break;
      case 145: /* joinop ::= JOIN_KW JOIN */
#line 664 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0); }
#line 2876 "parse.c"
        break;
      case 146: /* joinop ::= JOIN_KW nm JOIN */
#line 665 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,0); }
#line 2881 "parse.c"
        break;
      case 147: /* joinop ::= JOIN_KW nm nm JOIN */
#line 667 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0); }
#line 2886 "parse.c"
        break;
      case 148: /* on_opt ::= ON expr */
      case 165: /* having_opt ::= HAVING expr */ yytestcase(yyruleno==165);
      case 172: /* where_opt ::= WHERE expr */ yytestcase(yyruleno==172);
      case 232: /* case_else ::= ELSE expr */ yytestcase(yyruleno==232);
      case 234: /* case_operand ::= expr */ yytestcase(yyruleno==234);
#line 671 "parse.y"
{yygotominor.yy314 = yymsp[0].minor.yy118.pExpr;}
#line 2895 "parse.c"
        break;
      case 149: /* on_opt ::= */
      case 164: /* having_opt ::= */ yytestcase(yyruleno==164);
      case 171: /* where_opt ::= */ yytestcase(yyruleno==171);
      case 233: /* case_else ::= */ yytestcase(yyruleno==233);
      case 235: /* case_operand ::= */ yytestcase(yyruleno==235);
#line 672 "parse.y"
{yygotominor.yy314 = 0;}
#line 2904 "parse.c"
        break;
      case 152: /* indexed_opt ::= NOT INDEXED */
#line 687 "parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=1;}
#line 2909 "parse.c"
        break;
      case 153: /* using_opt ::= USING LP idlist RP */
      case 181: /* idlist_opt ::= LP idlist RP */ yytestcase(yyruleno==181);
#line 691 "parse.y"
{yygotominor.yy384 = yymsp[-1].minor.yy384;}
#line 2915 "parse.c"
        break;
      case 154: /* using_opt ::= */
      case 180: /* idlist_opt ::= */ yytestcase(yyruleno==180);
#line 692 "parse.y"
{yygotominor.yy384 = 0;}
#line 2921 "parse.c"
        break;
      case 156: /* orderby_opt ::= ORDER BY sortlist */
      case 163: /* groupby_opt ::= GROUP BY nexprlist */ yytestcase(yyruleno==163);
      case 236: /* exprlist ::= nexprlist */ yytestcase(yyruleno==236);
#line 706 "parse.y"
{yygotominor.yy322 = yymsp[0].minor.yy322;}
#line 2928 "parse.c"
        break;
      case 157: /* sortlist ::= sortlist COMMA expr sortorder */
#line 707 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy322,yymsp[-1].minor.yy118.pExpr);
  sqlite3ExprListSetSortOrder(yygotominor.yy322,yymsp[0].minor.yy4);
}
#line 2936 "parse.c"
        break;
      case 158: /* sortlist ::= expr sortorder */
#line 711 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,0,yymsp[-1].minor.yy118.pExpr);
  sqlite3ExprListSetSortOrder(yygotominor.yy322,yymsp[0].minor.yy4);
}
#line 2944 "parse.c"
        break;
      case 159: /* sortorder ::= ASC */
#line 718 "parse.y"
{yygotominor.yy4 = SQLITE_SO_ASC;}
#line 2949 "parse.c"
        break;
      case 160: /* sortorder ::= DESC */
#line 719 "parse.y"
{yygotominor.yy4 = SQLITE_SO_DESC;}
#line 2954 "parse.c"
        break;
      case 161: /* sortorder ::= */
#line 720 "parse.y"
{yygotominor.yy4 = SQLITE_SO_UNDEFINED;}
#line 2959 "parse.c"
        break;
      case 166: /* limit_opt ::= */
#line 745 "parse.y"
{yygotominor.yy292.pLimit = 0; yygotominor.yy292.pOffset = 0;}
#line 2964 "parse.c"
        break;
      case 167: /* limit_opt ::= LIMIT expr */
#line 746 "parse.y"
{yygotominor.yy292.pLimit = yymsp[0].minor.yy118.pExpr; yygotominor.yy292.pOffset = 0;}
#line 2969 "parse.c"
        break;
      case 168: /* limit_opt ::= LIMIT expr OFFSET expr */
#line 748 "parse.y"
{yygotominor.yy292.pLimit = yymsp[-2].minor.yy118.pExpr; yygotominor.yy292.pOffset = yymsp[0].minor.yy118.pExpr;}
#line 2974 "parse.c"
        break;
      case 169: /* limit_opt ::= LIMIT expr COMMA expr */
#line 750 "parse.y"
{yygotominor.yy292.pOffset = yymsp[-2].minor.yy118.pExpr; yygotominor.yy292.pLimit = yymsp[0].minor.yy118.pExpr;}
#line 2979 "parse.c"
        break;
      case 170: /* cmd ::= with DELETE FROM fullname indexed_opt where_opt */
#line 764 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy451, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-2].minor.yy259, &yymsp[-1].minor.yy0);
  sqlite3DeleteFrom(pParse,yymsp[-2].minor.yy259,yymsp[0].minor.yy314);
}
#line 2988 "parse.c"
        break;
      case 173: /* cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt */
#line 791 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-7].minor.yy451, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-4].minor.yy259, &yymsp[-3].minor.yy0);
  sqlite3ExprListCheckLength(pParse,yymsp[-1].minor.yy322,"set list"); 
  sqlite3Update(pParse,yymsp[-4].minor.yy259,yymsp[-1].minor.yy322,yymsp[0].minor.yy314,yymsp[-5].minor.yy4);
}
#line 2998 "parse.c"
        break;
      case 174: /* setlist ::= setlist COMMA nm EQ expr */
#line 802 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy322, yymsp[0].minor.yy118.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy322, &yymsp[-2].minor.yy0, 1);
}
#line 3006 "parse.c"
        break;
      case 175: /* setlist ::= nm EQ expr */
#line 806 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy118.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy322, &yymsp[-2].minor.yy0, 1);
}
#line 3014 "parse.c"
        break;
      case 176: /* cmd ::= with insert_cmd INTO fullname idlist_opt select */
#line 813 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy451, 1);
  sqlite3Insert(pParse, yymsp[-2].minor.yy259, yymsp[0].minor.yy387, yymsp[-1].minor.yy384, yymsp[-4].minor.yy4);
}
#line 3022 "parse.c"
        break;
      case 177: /* cmd ::= with insert_cmd INTO fullname idlist_opt DEFAULT VALUES */
#line 818 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-6].minor.yy451, 1);
  sqlite3Insert(pParse, yymsp[-3].minor.yy259, 0, yymsp[-2].minor.yy384, yymsp[-5].minor.yy4);
}
#line 3030 "parse.c"
        break;
      case 182: /* idlist ::= idlist COMMA nm */
#line 835 "parse.y"
{yygotominor.yy384 = sqlite3IdListAppend(pParse->db,yymsp[-2].minor.yy384,&yymsp[0].minor.yy0);}
#line 3035 "parse.c"
        break;
      case 183: /* idlist ::= nm */
#line 837 "parse.y"
{yygotominor.yy384 = sqlite3IdListAppend(pParse->db,0,&yymsp[0].minor.yy0);}
#line 3040 "parse.c"
        break;
      case 184: /* expr ::= term */
#line 868 "parse.y"
{yygotominor.yy118 = yymsp[0].minor.yy118;}
#line 3045 "parse.c"
        break;
      case 185: /* expr ::= LP expr RP */
#line 869 "parse.y"
{yygotominor.yy118.pExpr = yymsp[-1].minor.yy118.pExpr; spanSet(&yygotominor.yy118,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);}
#line 3050 "parse.c"
        break;
      case 186: /* term ::= NULL */
      case 191: /* term ::= INTEGER|FLOAT|BLOB */ yytestcase(yyruleno==191);
      case 192: /* term ::= STRING */ yytestcase(yyruleno==192);
#line 870 "parse.y"
{spanExpr(&yygotominor.yy118, pParse, yymsp[0].major, &yymsp[0].minor.yy0);}
#line 3057 "parse.c"
        break;
      case 187: /* expr ::= ID|INDEXED */
      case 188: /* expr ::= JOIN_KW */ yytestcase(yyruleno==188);
#line 871 "parse.y"
{spanExpr(&yygotominor.yy118, pParse, TK_ID, &yymsp[0].minor.yy0);}
#line 3063 "parse.c"
        break;
      case 189: /* expr ::= nm DOT nm */
#line 873 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp2, 0);
  spanSet(&yygotominor.yy118,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3073 "parse.c"
        break;
      case 190: /* expr ::= nm DOT nm DOT nm */
#line 879 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-4].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp3 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  Expr *temp4 = sqlite3PExpr(pParse, TK_DOT, temp2, temp3, 0);
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp4, 0);
  spanSet(&yygotominor.yy118,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3085 "parse.c"
        break;
      case 193: /* expr ::= VARIABLE */
#line 889 "parse.y"
{
  if( yymsp[0].minor.yy0.n>=2 && yymsp[0].minor.yy0.z[0]=='#' && sqlite3Isdigit(yymsp[0].minor.yy0.z[1]) ){
    /* When doing a nested parse, one can include terms in an expression
    ** that look like this:   #1 #2 ...  These terms refer to registers
    ** in the virtual machine.  #N is the N-th register. */
    if( pParse->nested==0 ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &yymsp[0].minor.yy0);
      yygotominor.yy118.pExpr = 0;
    }else{
      yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_REGISTER, 0, 0, &yymsp[0].minor.yy0);
      if( yygotominor.yy118.pExpr ) sqlite3GetInt32(&yymsp[0].minor.yy0.z[1], &yygotominor.yy118.pExpr->iTable);
    }
  }else{
    spanExpr(&yygotominor.yy118, pParse, TK_VARIABLE, &yymsp[0].minor.yy0);
    sqlite3ExprAssignVarNumber(pParse, yygotominor.yy118.pExpr);
  }
  spanSet(&yygotominor.yy118, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 3107 "parse.c"
        break;
      case 194: /* expr ::= expr COLLATE ID|STRING */
#line 907 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy118.pExpr, &yymsp[0].minor.yy0, 1);
  yygotominor.yy118.zStart = yymsp[-2].minor.yy118.zStart;
  yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3116 "parse.c"
        break;
      case 195: /* expr ::= CAST LP expr AS typetoken RP */
#line 913 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_CAST, yymsp[-3].minor.yy118.pExpr, 0, &yymsp[-1].minor.yy0);
  spanSet(&yygotominor.yy118,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3124 "parse.c"
        break;
      case 196: /* expr ::= ID|INDEXED LP distinct exprlist RP */
#line 918 "parse.y"
{
  if( yymsp[-1].minor.yy322 && yymsp[-1].minor.yy322->nExpr>pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", &yymsp[-4].minor.yy0);
  }
  yygotominor.yy118.pExpr = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy322, &yymsp[-4].minor.yy0);
  spanSet(&yygotominor.yy118,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy4==SF_Distinct && yygotominor.yy118.pExpr ){
    yygotominor.yy118.pExpr->flags |= EP_Distinct;
  }
}
#line 3138 "parse.c"
        break;
      case 197: /* expr ::= ID|INDEXED LP STAR RP */
#line 928 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0);
  spanSet(&yygotominor.yy118,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3146 "parse.c"
        break;
      case 198: /* term ::= CTIME_KW */
#line 932 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0);
  spanSet(&yygotominor.yy118, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 3154 "parse.c"
        break;
      case 199: /* expr ::= expr AND expr */
      case 200: /* expr ::= expr OR expr */ yytestcase(yyruleno==200);
      case 201: /* expr ::= expr LT|GT|GE|LE expr */ yytestcase(yyruleno==201);
      case 202: /* expr ::= expr EQ|NE expr */ yytestcase(yyruleno==202);
      case 203: /* expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr */ yytestcase(yyruleno==203);
      case 204: /* expr ::= expr PLUS|MINUS expr */ yytestcase(yyruleno==204);
      case 205: /* expr ::= expr STAR|SLASH|REM expr */ yytestcase(yyruleno==205);
      case 206: /* expr ::= expr CONCAT expr */ yytestcase(yyruleno==206);
#line 961 "parse.y"
{spanBinaryExpr(&yygotominor.yy118,pParse,yymsp[-1].major,&yymsp[-2].minor.yy118,&yymsp[0].minor.yy118);}
#line 3166 "parse.c"
        break;
      case 207: /* likeop ::= LIKE_KW|MATCH */
#line 974 "parse.y"
{yygotominor.yy342.eOperator = yymsp[0].minor.yy0; yygotominor.yy342.bNot = 0;}
#line 3171 "parse.c"
        break;
      case 208: /* likeop ::= NOT LIKE_KW|MATCH */
#line 975 "parse.y"
{yygotominor.yy342.eOperator = yymsp[0].minor.yy0; yygotominor.yy342.bNot = 1;}
#line 3176 "parse.c"
        break;
      case 209: /* expr ::= expr likeop expr */
#line 976 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[0].minor.yy118.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-2].minor.yy118.pExpr);
  yygotominor.yy118.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy342.eOperator);
  exprNot(pParse, yymsp[-1].minor.yy342.bNot, &yygotominor.yy118.pExpr);
  yygotominor.yy118.zStart = yymsp[-2].minor.yy118.zStart;
  yygotominor.yy118.zEnd = yymsp[0].minor.yy118.zEnd;
  if( yygotominor.yy118.pExpr ) yygotominor.yy118.pExpr->flags |= EP_InfixFunc;
}
#line 3190 "parse.c"
        break;
      case 210: /* expr ::= expr likeop expr ESCAPE expr */
#line 986 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy118.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-4].minor.yy118.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy118.pExpr);
  yygotominor.yy118.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy342.eOperator);
  exprNot(pParse, yymsp[-3].minor.yy342.bNot, &yygotominor.yy118.pExpr);
  yygotominor.yy118.zStart = yymsp[-4].minor.yy118.zStart;
  yygotominor.yy118.zEnd = yymsp[0].minor.yy118.zEnd;
  if( yygotominor.yy118.pExpr ) yygotominor.yy118.pExpr->flags |= EP_InfixFunc;
}
#line 3205 "parse.c"
        break;
      case 211: /* expr ::= expr ISNULL|NOTNULL */
#line 1014 "parse.y"
{spanUnaryPostfix(&yygotominor.yy118,pParse,yymsp[0].major,&yymsp[-1].minor.yy118,&yymsp[0].minor.yy0);}
#line 3210 "parse.c"
        break;
      case 212: /* expr ::= expr NOT NULL */
#line 1015 "parse.y"
{spanUnaryPostfix(&yygotominor.yy118,pParse,TK_NOTNULL,&yymsp[-2].minor.yy118,&yymsp[0].minor.yy0);}
#line 3215 "parse.c"
        break;
      case 213: /* expr ::= expr IS expr */
#line 1036 "parse.y"
{
  spanBinaryExpr(&yygotominor.yy118,pParse,TK_IS,&yymsp[-2].minor.yy118,&yymsp[0].minor.yy118);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy118.pExpr, yygotominor.yy118.pExpr, TK_ISNULL);
}
#line 3223 "parse.c"
        break;
      case 214: /* expr ::= expr IS NOT expr */
#line 1040 "parse.y"
{
  spanBinaryExpr(&yygotominor.yy118,pParse,TK_ISNOT,&yymsp[-3].minor.yy118,&yymsp[0].minor.yy118);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy118.pExpr, yygotominor.yy118.pExpr, TK_NOTNULL);
}
#line 3231 "parse.c"
        break;
      case 215: /* expr ::= NOT expr */
      case 216: /* expr ::= BITNOT expr */ yytestcase(yyruleno==216);
#line 1063 "parse.y"
{spanUnaryPrefix(&yygotominor.yy118,pParse,yymsp[-1].major,&yymsp[0].minor.yy118,&yymsp[-1].minor.yy0);}
#line 3237 "parse.c"
        break;
      case 217: /* expr ::= MINUS expr */
#line 1066 "parse.y"
{spanUnaryPrefix(&yygotominor.yy118,pParse,TK_UMINUS,&yymsp[0].minor.yy118,&yymsp[-1].minor.yy0);}
#line 3242 "parse.c"
        break;
      case 218: /* expr ::= PLUS expr */
#line 1068 "parse.y"
{spanUnaryPrefix(&yygotominor.yy118,pParse,TK_UPLUS,&yymsp[0].minor.yy118,&yymsp[-1].minor.yy0);}
#line 3247 "parse.c"
        break;
      case 221: /* expr ::= expr between_op expr AND expr */
#line 1073 "parse.y"
{
  ExprList *pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy118.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy118.pExpr);
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_BETWEEN, yymsp[-4].minor.yy118.pExpr, 0, 0);
  if( yygotominor.yy118.pExpr ){
    yygotominor.yy118.pExpr->x.pList = pList;
  }else{
    sqlite3ExprListDelete(pParse->db, pList);
  } 
  exprNot(pParse, yymsp[-3].minor.yy4, &yygotominor.yy118.pExpr);
  yygotominor.yy118.zStart = yymsp[-4].minor.yy118.zStart;
  yygotominor.yy118.zEnd = yymsp[0].minor.yy118.zEnd;
}
#line 3264 "parse.c"
        break;
      case 224: /* expr ::= expr in_op LP exprlist RP */
#line 1090 "parse.y"
{
    if( yymsp[-1].minor.yy322==0 ){
      /* Expressions of the form
      **
      **      expr1 IN ()
      **      expr1 NOT IN ()
      **
      ** simplify to constants 0 (false) and 1 (true), respectively,
      ** regardless of the value of expr1.
      */
      yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_INTEGER, 0, 0, &sqlite3IntTokens[yymsp[-3].minor.yy4]);
      sqlite3ExprDelete(pParse->db, yymsp[-4].minor.yy118.pExpr);
    }else if( yymsp[-1].minor.yy322->nExpr==1 ){
      /* Expressions of the form:
      **
      **      expr1 IN (?1)
      **      expr1 NOT IN (?2)
      **
      ** with exactly one value on the RHS can be simplified to something
      ** like this:
      **
      **      expr1 == ?1
      **      expr1 <> ?2
      **
      ** But, the RHS of the == or <> is marked with the EP_Generic flag
      ** so that it may not contribute to the computation of comparison
      ** affinity or the collating sequence to use for comparison.  Otherwise,
      ** the semantics would be subtly different from IN or NOT IN.
      */
      Expr *pRHS = yymsp[-1].minor.yy322->a[0].pExpr;
      yymsp[-1].minor.yy322->a[0].pExpr = 0;
      sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy322);
      /* pRHS cannot be NULL because a malloc error would have been detected
      ** before now and control would have never reached this point */
      if( ALWAYS(pRHS) ){
        pRHS->flags &= ~EP_Collate;
        pRHS->flags |= EP_Generic;
      }
      yygotominor.yy118.pExpr = sqlite3PExpr(pParse, yymsp[-3].minor.yy4 ? TK_NE : TK_EQ, yymsp[-4].minor.yy118.pExpr, pRHS, 0);
    }else{
      yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy118.pExpr, 0, 0);
      if( yygotominor.yy118.pExpr ){
        yygotominor.yy118.pExpr->x.pList = yymsp[-1].minor.yy322;
        sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy118.pExpr);
      }else{
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy322);
      }
      exprNot(pParse, yymsp[-3].minor.yy4, &yygotominor.yy118.pExpr);
    }
    yygotominor.yy118.zStart = yymsp[-4].minor.yy118.zStart;
    yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3320 "parse.c"
        break;
      case 225: /* expr ::= LP select RP */
#line 1142 "parse.y"
{
    yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_SELECT, 0, 0, 0);
    if( yygotominor.yy118.pExpr ){
      yygotominor.yy118.pExpr->x.pSelect = yymsp[-1].minor.yy387;
      ExprSetProperty(yygotominor.yy118.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy118.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy387);
    }
    yygotominor.yy118.zStart = yymsp[-2].minor.yy0.z;
    yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3336 "parse.c"
        break;
      case 226: /* expr ::= expr in_op LP select RP */
#line 1154 "parse.y"
{
    yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy118.pExpr, 0, 0);
    if( yygotominor.yy118.pExpr ){
      yygotominor.yy118.pExpr->x.pSelect = yymsp[-1].minor.yy387;
      ExprSetProperty(yygotominor.yy118.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy118.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy387);
    }
    exprNot(pParse, yymsp[-3].minor.yy4, &yygotominor.yy118.pExpr);
    yygotominor.yy118.zStart = yymsp[-4].minor.yy118.zStart;
    yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3353 "parse.c"
        break;
      case 227: /* expr ::= expr in_op nm dbnm */
#line 1167 "parse.y"
{
    SrcList *pSrc = sqlite3SrcListAppend(pParse->db, 0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);
    yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-3].minor.yy118.pExpr, 0, 0);
    if( yygotominor.yy118.pExpr ){
      yygotominor.yy118.pExpr->x.pSelect = sqlite3SelectNew(pParse, 0,pSrc,0,0,0,0,0,0,0);
      ExprSetProperty(yygotominor.yy118.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy118.pExpr);
    }else{
      sqlite3SrcListDelete(pParse->db, pSrc);
    }
    exprNot(pParse, yymsp[-2].minor.yy4, &yygotominor.yy118.pExpr);
    yygotominor.yy118.zStart = yymsp[-3].minor.yy118.zStart;
    yygotominor.yy118.zEnd = yymsp[0].minor.yy0.z ? &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] : &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n];
  }
#line 3371 "parse.c"
        break;
      case 228: /* expr ::= EXISTS LP select RP */
#line 1181 "parse.y"
{
    Expr *p = yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_EXISTS, 0, 0, 0);
    if( p ){
      p->x.pSelect = yymsp[-1].minor.yy387;
      ExprSetProperty(p, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, p);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy387);
    }
    yygotominor.yy118.zStart = yymsp[-3].minor.yy0.z;
    yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3387 "parse.c"
        break;
      case 229: /* expr ::= CASE case_operand case_exprlist case_else END */
#line 1196 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_CASE, yymsp[-3].minor.yy314, 0, 0);
  if( yygotominor.yy118.pExpr ){
    yygotominor.yy118.pExpr->x.pList = yymsp[-1].minor.yy314 ? sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy322,yymsp[-1].minor.yy314) : yymsp[-2].minor.yy322;
    sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy118.pExpr);
  }else{
    sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy322);
    sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy314);
  }
  yygotominor.yy118.zStart = yymsp[-4].minor.yy0.z;
  yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3403 "parse.c"
        break;
      case 230: /* case_exprlist ::= case_exprlist WHEN expr THEN expr */
#line 1210 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy322, yymsp[-2].minor.yy118.pExpr);
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,yygotominor.yy322, yymsp[0].minor.yy118.pExpr);
}
#line 3411 "parse.c"
        break;
      case 231: /* case_exprlist ::= WHEN expr THEN expr */
#line 1214 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy118.pExpr);
  yygotominor.yy322 = sqlite3ExprListAppend(pParse,yygotominor.yy322, yymsp[0].minor.yy118.pExpr);
}
#line 3419 "parse.c"
        break;
      case 238: /* nexprlist ::= nexprlist COMMA expr */
#line 1235 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy322,yymsp[0].minor.yy118.pExpr);}
#line 3424 "parse.c"
        break;
      case 239: /* nexprlist ::= expr */
#line 1237 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(pParse,0,yymsp[0].minor.yy118.pExpr);}
#line 3429 "parse.c"
        break;
      case 240: /* cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt */
#line 1243 "parse.y"
{
  sqlite3CreateIndex(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, 
                     sqlite3SrcListAppend(pParse->db,0,&yymsp[-4].minor.yy0,0), yymsp[-2].minor.yy322, yymsp[-10].minor.yy4,
                      &yymsp[-11].minor.yy0, yymsp[0].minor.yy314, SQLITE_SO_ASC, yymsp[-8].minor.yy4);
}
#line 3438 "parse.c"
        break;
      case 241: /* uniqueflag ::= UNIQUE */
      case 292: /* raisetype ::= ABORT */ yytestcase(yyruleno==292);
#line 1250 "parse.y"
{yygotominor.yy4 = OE_Abort;}
#line 3444 "parse.c"
        break;
      case 242: /* uniqueflag ::= */
#line 1251 "parse.y"
{yygotominor.yy4 = OE_None;}
#line 3449 "parse.c"
        break;
      case 245: /* eidlist ::= eidlist COMMA nm collate sortorder */
#line 1301 "parse.y"
{
  yygotominor.yy322 = parserAddExprIdListTerm(pParse, yymsp[-4].minor.yy322, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy4, yymsp[0].minor.yy4);
}
#line 3456 "parse.c"
        break;
      case 246: /* eidlist ::= nm collate sortorder */
#line 1304 "parse.y"
{
  yygotominor.yy322 = parserAddExprIdListTerm(pParse, 0, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy4, yymsp[0].minor.yy4);
}
#line 3463 "parse.c"
        break;
      case 249: /* cmd ::= DROP INDEX ifexists fullname */
#line 1315 "parse.y"
{sqlite3DropIndex(pParse, yymsp[0].minor.yy259, yymsp[-1].minor.yy4);}
#line 3468 "parse.c"
        break;
      case 250: /* cmd ::= VACUUM */
      case 251: /* cmd ::= VACUUM nm */ yytestcase(yyruleno==251);
#line 1321 "parse.y"
{sqlite3Vacuum(pParse);}
#line 3474 "parse.c"
        break;
      case 252: /* cmd ::= PRAGMA nm dbnm */
#line 1329 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,0,0);}
#line 3479 "parse.c"
        break;
      case 253: /* cmd ::= PRAGMA nm dbnm EQ nmnum */
#line 1330 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,0);}
#line 3484 "parse.c"
        break;
      case 254: /* cmd ::= PRAGMA nm dbnm LP nmnum RP */
#line 1331 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,0);}
#line 3489 "parse.c"
        break;
      case 255: /* cmd ::= PRAGMA nm dbnm EQ minus_num */
#line 1333 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,1);}
#line 3494 "parse.c"
        break;
      case 256: /* cmd ::= PRAGMA nm dbnm LP minus_num RP */
#line 1335 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,1);}
#line 3499 "parse.c"
        break;
      case 265: /* cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END */
#line 1351 "parse.y"
{
  Token all;
  all.z = yymsp[-3].minor.yy0.z;
  all.n = (int)(yymsp[0].minor.yy0.z - yymsp[-3].minor.yy0.z) + yymsp[0].minor.yy0.n;
  sqlite3FinishTrigger(pParse, yymsp[-1].minor.yy203, &all);
}
#line 3509 "parse.c"
        break;
      case 266: /* trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause */
#line 1360 "parse.y"
{
  sqlite3BeginTrigger(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, yymsp[-5].minor.yy4, yymsp[-4].minor.yy90.a, yymsp[-4].minor.yy90.b, yymsp[-2].minor.yy259, yymsp[0].minor.yy314, yymsp[-10].minor.yy4, yymsp[-8].minor.yy4);
  yygotominor.yy0 = (yymsp[-6].minor.yy0.n==0?yymsp[-7].minor.yy0:yymsp[-6].minor.yy0);
}
#line 3517 "parse.c"
        break;
      case 267: /* trigger_time ::= BEFORE */
      case 270: /* trigger_time ::= */ yytestcase(yyruleno==270);
#line 1366 "parse.y"
{ yygotominor.yy4 = TK_BEFORE; }
#line 3523 "parse.c"
        break;
      case 268: /* trigger_time ::= AFTER */
#line 1367 "parse.y"
{ yygotominor.yy4 = TK_AFTER;  }
#line 3528 "parse.c"
        break;
      case 269: /* trigger_time ::= INSTEAD OF */
#line 1368 "parse.y"
{ yygotominor.yy4 = TK_INSTEAD;}
#line 3533 "parse.c"
        break;
      case 271: /* trigger_event ::= DELETE|INSERT */
      case 272: /* trigger_event ::= UPDATE */ yytestcase(yyruleno==272);
#line 1373 "parse.y"
{yygotominor.yy90.a = yymsp[0].major; yygotominor.yy90.b = 0;}
#line 3539 "parse.c"
        break;
      case 273: /* trigger_event ::= UPDATE OF idlist */
#line 1375 "parse.y"
{yygotominor.yy90.a = TK_UPDATE; yygotominor.yy90.b = yymsp[0].minor.yy384;}
#line 3544 "parse.c"
        break;
      case 276: /* when_clause ::= */
      case 297: /* key_opt ::= */ yytestcase(yyruleno==297);
#line 1382 "parse.y"
{ yygotominor.yy314 = 0; }
#line 3550 "parse.c"
        break;
      case 277: /* when_clause ::= WHEN expr */
      case 298: /* key_opt ::= KEY expr */ yytestcase(yyruleno==298);
#line 1383 "parse.y"
{ yygotominor.yy314 = yymsp[0].minor.yy118.pExpr; }
#line 3556 "parse.c"
        break;
      case 278: /* trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI */
#line 1387 "parse.y"
{
  assert( yymsp[-2].minor.yy203!=0 );
  yymsp[-2].minor.yy203->pLast->pNext = yymsp[-1].minor.yy203;
  yymsp[-2].minor.yy203->pLast = yymsp[-1].minor.yy203;
  yygotominor.yy203 = yymsp[-2].minor.yy203;
}
#line 3566 "parse.c"
        break;
      case 279: /* trigger_cmd_list ::= trigger_cmd SEMI */
#line 1393 "parse.y"
{ 
  assert( yymsp[-1].minor.yy203!=0 );
  yymsp[-1].minor.yy203->pLast = yymsp[-1].minor.yy203;
  yygotominor.yy203 = yymsp[-1].minor.yy203;
}
#line 3575 "parse.c"
        break;
      case 281: /* trnm ::= nm DOT nm */
#line 1405 "parse.y"
{
  yygotominor.yy0 = yymsp[0].minor.yy0;
  sqlite3ErrorMsg(pParse, 
        "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
        "statements within triggers");
}
#line 3585 "parse.c"
        break;
      case 283: /* tridxby ::= INDEXED BY nm */
#line 1417 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the INDEXED BY clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3594 "parse.c"
        break;
      case 284: /* tridxby ::= NOT INDEXED */
#line 1422 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the NOT INDEXED clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3603 "parse.c"
        break;
      case 285: /* trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt */
#line 1435 "parse.y"
{ yygotominor.yy203 = sqlite3TriggerUpdateStep(pParse->db, &yymsp[-4].minor.yy0, yymsp[-1].minor.yy322, yymsp[0].minor.yy314, yymsp[-5].minor.yy4); }
#line 3608 "parse.c"
        break;
      case 286: /* trigger_cmd ::= insert_cmd INTO trnm idlist_opt select */
#line 1439 "parse.y"
{yygotominor.yy203 = sqlite3TriggerInsertStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy384, yymsp[0].minor.yy387, yymsp[-4].minor.yy4);}
#line 3613 "parse.c"
        break;
      case 287: /* trigger_cmd ::= DELETE FROM trnm tridxby where_opt */
#line 1443 "parse.y"
{yygotominor.yy203 = sqlite3TriggerDeleteStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[0].minor.yy314);}
#line 3618 "parse.c"
        break;
      case 288: /* trigger_cmd ::= select */
#line 1446 "parse.y"
{yygotominor.yy203 = sqlite3TriggerSelectStep(pParse->db, yymsp[0].minor.yy387); }
#line 3623 "parse.c"
        break;
      case 289: /* expr ::= RAISE LP IGNORE RP */
#line 1449 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, 0); 
  if( yygotominor.yy118.pExpr ){
    yygotominor.yy118.pExpr->affinity = OE_Ignore;
  }
  yygotominor.yy118.zStart = yymsp[-3].minor.yy0.z;
  yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3635 "parse.c"
        break;
      case 290: /* expr ::= RAISE LP raisetype COMMA nm RP */
#line 1457 "parse.y"
{
  yygotominor.yy118.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, &yymsp[-1].minor.yy0); 
  if( yygotominor.yy118.pExpr ) {
    yygotominor.yy118.pExpr->affinity = (char)yymsp[-3].minor.yy4;
  }
  yygotominor.yy118.zStart = yymsp[-5].minor.yy0.z;
  yygotominor.yy118.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3647 "parse.c"
        break;
      case 291: /* raisetype ::= ROLLBACK */
#line 1468 "parse.y"
{yygotominor.yy4 = OE_Rollback;}
#line 3652 "parse.c"
        break;
      case 293: /* raisetype ::= FAIL */
#line 1470 "parse.y"
{yygotominor.yy4 = OE_Fail;}
#line 3657 "parse.c"
        break;
      case 294: /* cmd ::= DROP TRIGGER ifexists fullname */
#line 1475 "parse.y"
{
  sqlite3DropTrigger(pParse,yymsp[0].minor.yy259,yymsp[-1].minor.yy4);
}
#line 3664 "parse.c"
        break;
      case 295: /* cmd ::= ATTACH database_kw_opt expr AS expr key_opt */
#line 1482 "parse.y"
{
  sqlite3Attach(pParse, yymsp[-3].minor.yy118.pExpr, yymsp[-1].minor.yy118.pExpr, yymsp[0].minor.yy314);
}
#line 3671 "parse.c"
        break;
      case 296: /* cmd ::= DETACH database_kw_opt expr */
#line 1485 "parse.y"
{
  sqlite3Detach(pParse, yymsp[0].minor.yy118.pExpr);
}
#line 3678 "parse.c"
        break;
      case 301: /* cmd ::= REINDEX */
#line 1500 "parse.y"
{sqlite3Reindex(pParse, 0, 0);}
#line 3683 "parse.c"
        break;
      case 302: /* cmd ::= REINDEX nm dbnm */
#line 1501 "parse.y"
{sqlite3Reindex(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3688 "parse.c"
        break;
      case 303: /* cmd ::= ANALYZE */
#line 1506 "parse.y"
{sqlite3Analyze(pParse, 0, 0);}
#line 3693 "parse.c"
        break;
      case 304: /* cmd ::= ANALYZE nm dbnm */
#line 1507 "parse.y"
{sqlite3Analyze(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3698 "parse.c"
        break;
      case 305: /* cmd ::= ALTER TABLE fullname RENAME TO nm */
#line 1512 "parse.y"
{
  sqlite3AlterRenameTable(pParse,yymsp[-3].minor.yy259,&yymsp[0].minor.yy0);
}
#line 3705 "parse.c"
        break;
      case 306: /* cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column */
#line 1515 "parse.y"
{
  sqlite3AlterFinishAddColumn(pParse, &yymsp[0].minor.yy0);
}
#line 3712 "parse.c"
        break;
      case 307: /* add_column_fullname ::= fullname */
#line 1518 "parse.y"
{
  disableLookaside(pParse);
  sqlite3AlterBeginAddColumn(pParse, yymsp[0].minor.yy259);
}
#line 3720 "parse.c"
        break;
      case 310: /* cmd ::= create_vtab */
#line 1528 "parse.y"
{sqlite3VtabFinishParse(pParse,0);}
#line 3725 "parse.c"
        break;
      case 311: /* cmd ::= create_vtab LP vtabarglist RP */
#line 1529 "parse.y"
{sqlite3VtabFinishParse(pParse,&yymsp[0].minor.yy0);}
#line 3730 "parse.c"
        break;
      case 312: /* create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm */
#line 1531 "parse.y"
{
    sqlite3VtabBeginParse(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy4);
}
#line 3737 "parse.c"
        break;
      case 315: /* vtabarg ::= */
#line 1536 "parse.y"
{sqlite3VtabArgInit(pParse);}
#line 3742 "parse.c"
        break;
      case 317: /* vtabargtoken ::= ANY */
      case 318: /* vtabargtoken ::= lp anylist RP */ yytestcase(yyruleno==318);
      case 319: /* lp ::= LP */ yytestcase(yyruleno==319);
#line 1538 "parse.y"
{sqlite3VtabArgExtend(pParse,&yymsp[0].minor.yy0);}
#line 3749 "parse.c"
        break;
      case 323: /* with ::= */
#line 1553 "parse.y"
{yygotominor.yy451 = 0;}
#line 3754 "parse.c"
        break;
      case 324: /* with ::= WITH wqlist */
      case 325: /* with ::= WITH RECURSIVE wqlist */ yytestcase(yyruleno==325);
#line 1555 "parse.y"
{ yygotominor.yy451 = yymsp[0].minor.yy451; }
#line 3760 "parse.c"
        break;
      case 326: /* wqlist ::= nm eidlist_opt AS LP select RP */
#line 1558 "parse.y"
{
  yygotominor.yy451 = sqlite3WithAdd(pParse, 0, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy322, yymsp[-1].minor.yy387);
}
#line 3767 "parse.c"
        break;
      case 327: /* wqlist ::= wqlist COMMA nm eidlist_opt AS LP select RP */
#line 1561 "parse.y"
{
  yygotominor.yy451 = sqlite3WithAdd(pParse, yymsp[-7].minor.yy451, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy322, yymsp[-1].minor.yy387);
}
#line 3774 "parse.c"
        break;
      default:
      /* (0) input ::= cmdlist */ yytestcase(yyruleno==0);
      /* (1) cmdlist ::= cmdlist ecmd */ yytestcase(yyruleno==1);
      /* (2) cmdlist ::= ecmd */ yytestcase(yyruleno==2);
      /* (3) ecmd ::= SEMI */ yytestcase(yyruleno==3);
      /* (4) ecmd ::= explain cmdx SEMI */ yytestcase(yyruleno==4);
      /* (5) explain ::= */ yytestcase(yyruleno==5);
      /* (10) trans_opt ::= */ yytestcase(yyruleno==10);
      /* (11) trans_opt ::= TRANSACTION */ yytestcase(yyruleno==11);
      /* (12) trans_opt ::= TRANSACTION nm */ yytestcase(yyruleno==12);
      /* (20) savepoint_opt ::= SAVEPOINT */ yytestcase(yyruleno==20);
      /* (21) savepoint_opt ::= */ yytestcase(yyruleno==21);
      /* (25) cmd ::= create_table create_table_args */ yytestcase(yyruleno==25);
      /* (36) columnlist ::= columnlist COMMA column */ yytestcase(yyruleno==36);
      /* (37) columnlist ::= column */ yytestcase(yyruleno==37);
      /* (43) type ::= */ yytestcase(yyruleno==43);
      /* (50) signed ::= plus_num */ yytestcase(yyruleno==50);
      /* (51) signed ::= minus_num */ yytestcase(yyruleno==51);
      /* (52) carglist ::= carglist ccons */ yytestcase(yyruleno==52);
      /* (53) carglist ::= */ yytestcase(yyruleno==53);
      /* (60) ccons ::= NULL onconf */ yytestcase(yyruleno==60);
      /* (88) conslist ::= conslist tconscomma tcons */ yytestcase(yyruleno==88);
      /* (89) conslist ::= tcons */ yytestcase(yyruleno==89);
      /* (91) tconscomma ::= */ yytestcase(yyruleno==91);
      /* (274) foreach_clause ::= */ yytestcase(yyruleno==274);
      /* (275) foreach_clause ::= FOR EACH ROW */ yytestcase(yyruleno==275);
      /* (282) tridxby ::= */ yytestcase(yyruleno==282);
      /* (299) database_kw_opt ::= DATABASE */ yytestcase(yyruleno==299);
      /* (300) database_kw_opt ::= */ yytestcase(yyruleno==300);
      /* (308) kwcolumn_opt ::= */ yytestcase(yyruleno==308);
      /* (309) kwcolumn_opt ::= COLUMNKW */ yytestcase(yyruleno==309);
      /* (313) vtabarglist ::= vtabarg */ yytestcase(yyruleno==313);
      /* (314) vtabarglist ::= vtabarglist COMMA vtabarg */ yytestcase(yyruleno==314);
      /* (316) vtabarg ::= vtabarg vtabargtoken */ yytestcase(yyruleno==316);
      /* (320) anylist ::= */ yytestcase(yyruleno==320);
      /* (321) anylist ::= anylist LP anylist RP */ yytestcase(yyruleno==321);
      /* (322) anylist ::= anylist ANY */ yytestcase(yyruleno==322);
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
  sqlite3ParserARG_FETCH;
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
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
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
  sqlite3ParserARG_FETCH;
#define TOKEN (yyminor.yy0)
/************ Begin %syntax_error code ****************************************/
#line 32 "parse.y"

  UNUSED_PARAMETER(yymajor);  /* Silence some compiler warnings */
  assert( TOKEN.z[0] );  /* The tokenizer always gives us a token */
  sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &TOKEN);
#line 3881 "parse.c"
/************ End %syntax_error code ******************************************/
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
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
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3ParserAlloc" which describes the current state of the parser.
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
void sqlite3Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3ParserARG_PDECL               /* Optional %extra_argument parameter */
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
  sqlite3ParserARG_STORE;

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
