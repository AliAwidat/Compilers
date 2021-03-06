
%{
/*************************************************
 *    lexer.l
 *
 *    Revision 0.0.0.0  1996/04/01  06:20:40  kurt
 *    o	Wrong macro used in rule for character constants
 *    o	Minor spelling typos corrected
 *    -- Kurt Cockrum <kurt@grogatch.seaslug.org>
 *
 *    Programmer: Shaun Flisakowski
 *
 *************************************************/

#undef yywrap
int yywrap(void);

#define  YY_BUFFER_STATE_DEFD

#include <stdio.h>
#include <stdlib.h>
//#include <io.h>

#include "globals.h"
#include "gram_parser.cpp.h"
#include "lexer.h"
#include "token.h"
#include "nmetab.h"
#include "symtab.h"
#include "heap.h"


//#define isatty  _isatty
//#define fileno  _fileno


//FILE *yyin;
char line_buf[MAX_TOKN_LEN];

int  err_cnt = 0;
int  err_tok = 0;
int  err_type = 0;

int  in_preprocess = 0;
int  in_charconst = 0;

static int cpp_comment_warn = 1;
int  gcc2_paren_cnt = 0;

char  tokn_buff[MAX_TOKN_LEN];
char *tokn_end = &tokn_buff[MAX_TOKN_LEN-1];
char *tokn_ptr;

int cnt_nl(char *txt, int len);

#undef YY_INPUT

#    ifdef STRING_PARSE

#define    min(X,Y)    ((X)<(Y) ? (X) : (Y))

    /* Scan input from a string instead */
#define YY_INPUT(buf,result,max_size) \
  do { \
     int n = min(max_size, Parse_TOS->parselimit - Parse_TOS->parseptr);\
     if (n>0){ \
        memcpy(buf,Parse_TOS->parseptr,(unsigned int) n); \
        Parse_TOS->parseptr += n; \
        result = n; \
     } else \
        result = YY_NULL; \
  } while(0)

#    else    /* not STRING_PARSE */

    /* Scan input from a file pointer (default) */
#define YY_INPUT(buf,result,max_size) \
    do { \
        if (fgets((char*) (buf),max_size,yyin) == NULL) { \
           result = YY_NULL; \
        } else { \
           result = strlen((char*)(buf)); \
           strncpy(line_buf, (char*)(buf), MAX_TOKN_LEN); \
        } \
    } while(0)

#    endif    /* not STRING_PARSE */

#define YY_USER_ACTION \
    { \
        Parse_TOS->yycolno = Parse_TOS->yynxtcol; \
        Parse_TOS->yynxtcol += yyleng; \
    }

#undef SETPOS_NDE
#define SETPOS_NDE(TYPE) \
	do { \
          err_type = 0; \
          lvalp->node = make_node(TN_EMPTY, ParseStack->contxt); \
          lvalp->node->hdr.line = Parse_TOS->yylineno; \
          lvalp->node->hdr.col  = Parse_TOS->yycolno; \
          err_tok = lvalp->node->hdr.tok = (TYPE); \
	} while(0)

#undef SETPOS_LEAF
#define SETPOS_LEAF(TOK_TYPE,PRSE_TYPE) \
	do { \
          lvalp->leaf = make_leaf((tn_t) (err_type=PRSE_TYPE), ParseStack->contxt); \
          lvalp->leaf->hdr.line = Parse_TOS->yylineno; \
          lvalp->leaf->hdr.col  = Parse_TOS->yycolno; \
          err_tok = lvalp->leaf->hdr.tok  = (TOK_TYPE); \
	} while(0)

#undef SETPOS_IF
#define SETPOS_IF(TYPE) \
	do { \
          err_type = 0; \
          lvalp->ifn = make_if((tn_t)(err_type=TN_IF), ParseStack->contxt); \
          lvalp->ifn->hdr.line = Parse_TOS->yylineno; \
          lvalp->ifn->hdr.col  = Parse_TOS->yycolno; \
          err_tok = lvalp->ifn->hdr.tok  = (TYPE); \
	} while(0)

#undef SETPOS_FOR
#define SETPOS_FOR(TYPE) \
	do { \
          err_type = 0; \
          lvalp->forn = make_for((tn_t)(err_type=TN_FOR), ParseStack->contxt); \
          lvalp->forn->hdr.line = Parse_TOS->yylineno; \
          lvalp->forn->hdr.col  = Parse_TOS->yycolno; \
          err_tok = lvalp->forn->hdr.tok  = (TYPE); \
	} while(0)

#undef  LAST_STATE
#define LAST_STATE() \
        do { \
          if (in_preprocess) \
            BEGIN(PP); \
          else \
            BEGIN(INITIAL); \
        } while (0)  

#undef  BEGIN_PP
#define BEGIN_PP() \
        do { \
          BEGIN(PP); \
          in_preprocess = 1; \
        } while(0)

YYSTYPE my_var;

EXTERN int yylex ARGS((YYSTYPE *lvalp));

/*  To give some args to yylex */
#undef YY_DECL
#ifdef  __STDC__
/*  #define YY_DECL int yylex YY_PROTO((YYSTYPE *lvalp))  */
#define YY_DECL int yylex (YYSTYPE *lvalp)
#else
#define YY_DECL int yylex(lvalp) YYSTYPE *lvalp;
#endif

%}
%x Start PP CMMT CC STR GCC2
%a 9000
%o 9000
/* %option nounput */

dot		"."
digit		[0-9]
octdigit	[0-7]
hexdigit	[0-9a-fA-F]
digits          {digit}+
alpha		[a-zA-Z_]
alphanum	{alpha}|{digit}

usuffix         [uU]
lsuffix         [lL]
intsuffix       {usuffix}{lsuffix}?|{lsuffix}{usuffix}?
intnum		{digit}+{intsuffix}?
octnum		0{octdigit}+{intsuffix}?
hexnum		0[xX]{hexdigit}+{intsuffix}?

exponent	[Ee][+-]?{digits}
floatsuffix     [fFlL]

whitespace      [ \t\f\v]
allwhite        [ \t\f\b\v\r\n]

pp_strt         ^{whitespace}*"#"{whitespace}*

%%

<INITIAL>"/*"       { BEGIN(CMMT);
                      tokn_ptr = tokn_buff;
                     *tokn_ptr++ = '/';
                     *tokn_ptr++ = '*';
                      SETPOS_LEAF(COMMENT,TN_COMMENT);
                    }
<INITIAL>"'"        { BEGIN(CC);    SETPOS_LEAF(CHAR_CONST,TN_INT); }
<INITIAL>\"         { BEGIN(STR);   tokn_ptr = tokn_buff;
                         SETPOS_LEAF(STRING,TN_STRING);
                    }

<INITIAL>"//*"      { /* Ambiguous C++ style comment, must parse as
                            '/' '/*' to be Ansi compliant */
                         yywarn(
                            "Ambiguous C++ comment, use '/ /*' -or- '// *'");
                         yyless(1);
                         SETPOS_NDE(DIV);
                         return(DIV);
                    }

<INITIAL>"//"[^*].*$ { /* C++ style comment */
                         if (cpp_comment_warn){
                             cpp_comment_warn = 0;
                             yywarn("C++ style comment encountered");
                         }
                     }

<INITIAL>"auto"     { SETPOS_LEAF(AUTO,TN_TYPE); return(AUTO); }
<INITIAL>"break"    { SETPOS_NDE(BREAK); return(BREAK); }
<INITIAL>"case"     { SETPOS_NDE(CASE); return(CASE); }
<INITIAL>"char"     { SETPOS_LEAF(CHAR,TN_TYPE); return(CHAR); }
<INITIAL>"const"    { SETPOS_LEAF(CONST,TN_TYPE); return(CONST); }
<INITIAL>"continue" { SETPOS_NDE(CONT); return(CONT); }
<INITIAL>"flatopt" { SETPOS_NDE(FOPT); return(FOPT); }
<INITIAL>"default"  { SETPOS_LEAF(DEFLT,TN_LABEL); return(DEFLT); }
<INITIAL>"do"       { SETPOS_NDE(DO); return(DO); }
<INITIAL>"double"   { SETPOS_LEAF(DOUBLE,TN_TYPE); return(DOUBLE); }
<INITIAL>"else"     { SETPOS_NDE(ELSE); return(ELSE); }
<INITIAL>"enum"     { SETPOS_NDE(ENUM); return(ENUM); }
<INITIAL>"extern"   { SETPOS_LEAF(EXTRN,TN_TYPE); return(EXTRN); }
<INITIAL>"float"    { SETPOS_LEAF(FLOAT,TN_TYPE); return(FLOAT); }
<INITIAL>"for"      { SETPOS_FOR(FOR); return(FOR); }
<INITIAL>"goto"     { SETPOS_NDE(GOTO); return(GOTO); }
<INITIAL>"if"       { SETPOS_IF(IF); return(IF); }
<INITIAL>"int"      { SETPOS_LEAF(INT,TN_TYPE); return(INT); }
<INITIAL>"long"     { SETPOS_LEAF(LONG,TN_TYPE); return(LONG); }
<INITIAL>"register" { SETPOS_LEAF(REGISTR,TN_TYPE); return(REGISTR); }
<INITIAL>"return"   { SETPOS_NDE(RETURN); return(RETURN); }
<INITIAL>"short"    { SETPOS_LEAF(SHORT,TN_TYPE); return(SHORT); }
<INITIAL>"signed"   { SETPOS_LEAF(SGNED,TN_TYPE); return(SGNED); }
<INITIAL>"sizeof"   { SETPOS_NDE(SIZEOF); return(SIZEOF); }
<INITIAL>"static"   { SETPOS_LEAF(STATIC,TN_TYPE); return(STATIC); }
<INITIAL>"struct"   { SETPOS_NDE(STRUCT); return(STRUCT); }
<INITIAL>"switch"   { SETPOS_NDE(SWITCH); return(SWITCH); }
<INITIAL>"typedef"  { SETPOS_LEAF(TYPEDEF,TN_TYPE); return(TYPEDEF); }
<INITIAL>"union"    { SETPOS_NDE(UNION); return(UNION); }
<INITIAL>"unsigned" { SETPOS_LEAF(UNSGNED,TN_TYPE); return(UNSGNED); }
<INITIAL>"void"     { SETPOS_LEAF(VOID,TN_TYPE); return(VOID); }
<INITIAL>"volatile" { SETPOS_LEAF(VOLATILE,TN_TYPE); return(VOLATILE); }
<INITIAL>"while"    { SETPOS_NDE(WHILE); return(WHILE); }

   /* ParC extension */ 
<INITIAL>"parblock"      { SETPOS_FOR(PARBLOCK); return(PARBLOCK); }
<INITIAL>"parfor"        { SETPOS_FOR(PARFOR); return(PARFOR); }

   /* GCC2 Extensions */
<INITIAL>"__attribute"    { /* A gcc2 extension - get rid of it. */
                            BEGIN(GCC2);
                          }
<INITIAL>"__attribute__"  { BEGIN(GCC2); }

<GCC2>"("[^()]*           { gcc2_paren_cnt++; }
<GCC2>[^()]*              { /* Throw away */ }
<GCC2>")"                 { if (--gcc2_paren_cnt == 0)
                              BEGIN(INITIAL);
                          }

<INITIAL>"__const"    { SETPOS_LEAF(CONST,TN_TYPE); return(CONST); }
<INITIAL>"__const__"  { SETPOS_LEAF(CONST,TN_TYPE); return(CONST); }
<INITIAL>"__signed" { SETPOS_LEAF(SGNED,TN_TYPE); return(SGNED); }
<INITIAL>"__signed__" { SETPOS_LEAF(SGNED,TN_TYPE); return(SGNED); }
<INITIAL>"__volatile" { SETPOS_LEAF(VOLATILE,TN_TYPE); return(VOLATILE); }
<INITIAL>"__volatile__" { SETPOS_LEAF(VOLATILE,TN_TYPE); return(VOLATILE); }

<INITIAL>"typeof"      { }
<INITIAL>"__typeof"    { }
<INITIAL>"__typeof__"  { }

<INITIAL>"inline"      { }
<INITIAL>"__inline"    { }
<INITIAL>"__inline__"  { }

<INITIAL>"__extension__"  { }

<INITIAL>"alignof"      { }
<INITIAL>"__alignof"    { }
<INITIAL>"__alignof__"  { }

<INITIAL>"__asm"      { }
<INITIAL>"__asm__"    { }
<INITIAL>"__imag"     { }
<INITIAL>"__imag__"   { }
<INITIAL>"__real"     { }
<INITIAL>"__real__"   { }
<INITIAL>"__complex"    { }
<INITIAL>"__complex__"  { }
<INITIAL>"__iterator"   { }
<INITIAL>"__iterator__" { }
<INITIAL>"__label__"    { }

<INITIAL>"+"        { SETPOS_NDE(PLUS); return(PLUS); }
<INITIAL>"-"        { SETPOS_NDE(MINUS); return(MINUS); }
<INITIAL>"*"        { SETPOS_NDE(STAR); return(STAR); }
<INITIAL>"/"        { SETPOS_NDE(DIV); return(DIV); }
<INITIAL>"%"        { SETPOS_NDE(MOD); return(MOD); }

<INITIAL>"+="       { SETPOS_NDE(PLUS_EQ); return(ASSIGN); }
<INITIAL>"-="       { SETPOS_NDE(MINUS_EQ); return(ASSIGN); }
<INITIAL>"*="       { SETPOS_NDE(STAR_EQ); return(ASSIGN); }
<INITIAL>"/="       { SETPOS_NDE(DIV_EQ); return(ASSIGN); }
<INITIAL>"%="       { SETPOS_NDE(MOD_EQ); return(ASSIGN); }

<INITIAL>"!"        { SETPOS_NDE(NOT); return(NOT); }
<INITIAL>"&&"       { SETPOS_NDE(AND); return(AND); }
<INITIAL>"||"       { SETPOS_NDE(OR); return(OR); }

<INITIAL>"~"        { SETPOS_NDE(B_NOT); return(B_NOT); }
<INITIAL>"&"        { SETPOS_NDE(B_AND); return(B_AND); }
<INITIAL>"|"        { SETPOS_NDE(B_OR); return(B_OR); }
<INITIAL>"^"        { SETPOS_NDE(B_XOR); return(B_XOR); }

<INITIAL>"~="       { SETPOS_NDE(B_NOT_EQ); return(ASSIGN); }
<INITIAL>"&="       { SETPOS_NDE(B_AND_EQ); return(ASSIGN); }
<INITIAL>"|="       { SETPOS_NDE(B_OR_EQ); return(ASSIGN); }
<INITIAL>"^="       { SETPOS_NDE(B_XOR_EQ); return(ASSIGN); }

<INITIAL>"<<"       { SETPOS_NDE(L_SHIFT); return(L_SHIFT); }
<INITIAL>">>"       { SETPOS_NDE(R_SHIFT); return(R_SHIFT); }
<INITIAL>"<<="      { SETPOS_NDE(L_SHIFT_EQ); return(ASSIGN); }
<INITIAL>">>="      { SETPOS_NDE(R_SHIFT_EQ); return(ASSIGN); }

<INITIAL>"=="       { SETPOS_NDE(EQUAL); return(COMP_EQ); }
<INITIAL>"<"        { SETPOS_NDE(LESS); return(COMP_ARITH); }
<PP>"<"             { SETPOS_NDE(LESS); return(LESS); }
<INITIAL>"<="       { SETPOS_NDE(LESS_EQ); return(COMP_ARITH); }
<INITIAL>">"        { SETPOS_NDE(GRTR); return(COMP_ARITH); }
<PP>">"             { SETPOS_NDE(GRTR); return(GRTR); }
<INITIAL>">="       { SETPOS_NDE(GRTR_EQ); return(COMP_ARITH); }
<INITIAL>"!="       { SETPOS_NDE(NOT_EQ); return(COMP_EQ); }

<INITIAL>"="        { SETPOS_NDE(EQ); return(EQ); }
<INITIAL>"++"       { SETPOS_NDE(INCR); return(INCR); }
<INITIAL>"--"       { SETPOS_NDE(DECR); return(DECR); }

<INITIAL>"("        { SETPOS_NDE(LPAREN); return(LPAREN); }
<INITIAL>")"        { SETPOS_NDE(RPAREN); return(RPAREN); }
<INITIAL>"["        { SETPOS_NDE(LBRCKT); return(LBRCKT); }
<INITIAL>"]"        { SETPOS_NDE(RBRCKT); return(RBRCKT); }
<INITIAL>"{"        { SETPOS_NDE(LBRACE); return(LBRACE); }
<INITIAL>"}"        { SETPOS_NDE(RBRACE); return(RBRACE); }

<INITIAL>"."        { SETPOS_NDE(DOT); return(DOT); }
<INITIAL>"->"       { SETPOS_NDE(ARROW); return(ARROW); }

<INITIAL>"?"        { SETPOS_NDE(QUESTMARK); return(QUESTMARK); }
<INITIAL>":"        { SETPOS_NDE(COLON); return(COLON); }
<INITIAL>";"        { SETPOS_NDE(SEMICOLON); return(SEMICOLON); }
<INITIAL>","        { SETPOS_NDE(COMMA); return(COMMA); }
<INITIAL>"..."      { SETPOS_LEAF(ELLIPSIS,TN_ELLIPSIS); return(ELLIPSIS); }

	/* Unused (invalid) characters */
<INITIAL>"`"        { SETPOS_NDE(BACKQUOTE); return(BACKQUOTE); }
<INITIAL>"@"        { SETPOS_NDE(AT); return(AT); }
<INITIAL>"$"        { SETPOS_NDE(DOLLAR); return(DOLLAR); }

	/* Preprocessor Stuff */
<INITIAL>{pp_strt}\n       { Parse_TOS->yylineno++; Parse_TOS->yynxtcol = 0; }
<INITIAL>{pp_strt}{digits}.*$ { BEGIN_PP(); get_lineno(); }
<INITIAL>{pp_strt}"define" {
                             int n = yyleng;
                             BEGIN_PP();
                             tokn_ptr = tokn_buff;
                             if (tokn_ptr + n < tokn_end) {
                                 memcpy(tokn_ptr,yytext,n);
                                 tokn_ptr += n;
                             } else 
                                yywarn("Preprocessor line too long");
                             SETPOS_LEAF(PP_DEFINE,TN_CPP);
                           }
<INITIAL>{pp_strt}"include" { BEGIN_PP(); }
<INITIAL>{pp_strt}"line"   { BEGIN_PP(); get_lineno(); }
<INITIAL>{pp_strt}"pragma" { BEGIN_PP(); }
<INITIAL>{pp_strt}"ident"  { BEGIN_PP(); }
<INITIAL>{pp_strt}"if"     { BEGIN_PP(); }
<INITIAL>{pp_strt}"ifdef"  { BEGIN_PP(); }
<INITIAL>{pp_strt}"ifndef" { BEGIN_PP(); }
<INITIAL>{pp_strt}"else"   { BEGIN_PP(); }
<INITIAL>{pp_strt}"elif"   { BEGIN_PP(); }
<INITIAL>{pp_strt}"endif"  { BEGIN_PP(); }
<INITIAL>{pp_strt}"error"  { BEGIN_PP(); }
<INITIAL>{pp_strt}"undef"  { BEGIN_PP(); }

<PP>"#"                { SETPOS_NDE(LB_SIGN); return(LB_SIGN); }
<PP>"##"               { SETPOS_NDE(DOUB_LB_SIGN); return(DOUB_LB_SIGN); }

<PP>\\(\n)             {  /* Preprocessor continuation line */
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                       }

<PP>\n                 {  /* End of this preprocessor logical line */
                         lvalp->node = (treenode *) NULL;
                         in_preprocess = 0;
                         yyless(0);    /* Hack! */
                         BEGIN(INITIAL);
                       }

<PP>[^\n\\]+           { /* Swallow cpp junk to prevent it being echo'd */
                       }

<INITIAL>{alpha}{alphanum}*  {    /* Identifier */
                         int last_was_type = (err_type == TN_TYPE)
                                          && (err_tok != TYPEDEF)
                                          && (err_tok != AUTO)
                                          && (err_tok != EXTRN)
                                          && (err_tok != STATIC)
                                          && (err_tok != VOLATILE)
                                          && (err_tok != CONST)
                                          && (err_tok != REGISTR);

                         SETPOS_LEAF(IDENT,TN_IDENT);
                         lvalp->leaf->data.sval = nmelook(yytext,yyleng+1);

                         /* Check the symbol table and insert a  */
                         /* ptr back to the definition.  */
                         {
                           symentry_t *se
                              = symtab_lookup(ParseStack->contxt->syms,
                                              lvalp->leaf->data.sval);
                           lvalp->leaf->syment = se;
  
                           if (! last_was_type)
                           {
                             if (se && is_typedef(se))
                             {
                                 err_type = lvalp->leaf->hdr.type = TN_TYPE;
                                 err_tok = lvalp->leaf->hdr.tok = TYPEDEF_NAME;
                                 return(TYPEDEF_NAME);
                             }
                           }
                         }
                         return(IDENT);
                       }

<INITIAL>{octnum}  |
<INITIAL>{intnum}  |
<INITIAL>{hexnum}   {  /* An integer */
                         SETPOS_LEAF(INUM,TN_INT);
                         lvalp->leaf->data.ival 
                                = strtol(yytext,(char **) NULL,0);
                         return(INUM);
                    }
          
<INITIAL>{digits}{dot}{digits}{exponent}?{floatsuffix}? |
<INITIAL>{digits}{dot}{exponent}?{floatsuffix}?         |
<INITIAL>{dot}{digits}{exponent}?{floatsuffix}?         |
<INITIAL>{digits}{exponent}{floatsuffix}? {
                         /*
                         ** Note: The floatsuffix, if any, will be
                         ** ignored by atof().
                         */
                         SETPOS_LEAF(RNUM,TN_REAL);
                         lvalp->leaf->data.dval = atof(yytext);
                         return(RNUM);
                       }

<STR>\"{allwhite}*\"   {   /* String Pasting */
                         cnt_nl(yytext,yyleng);
                       }

<STR>\"                {   /* Closing quote */
                         LAST_STATE();
                         *tokn_ptr = '\0';
                         if ((lvalp->leaf->data.str
                                 = (char *) malloc(strlen(tokn_buff)+1)) == NULL){
                            fputs("Error: Out of memory in Lexer.\n",stderr);
                            return(INVALID);
                         } else {
                            strcpy(lvalp->leaf->data.str,tokn_buff);
                            return(STRING);
                         }
                       }

<STR>\n                { /* Error - unterminated string constant */
                         yyerr("Unterminated string constant starting");
                         LAST_STATE();
                         return(INVALID);
                       }

<STR>\\[0-7]{1,3}      {   /* octal escape sequence */
                         uint result;

                         if (sscanf(yytext+1, "%o", &result ) < 1)
                         {
                             yyerr("Invalid octal escape sequence");
                             LAST_STATE();
                             return(INVALID);
                         }

                             /* error, constant is out-of-bounds */
                         if ( result > 0xff ){
                            yyerr("Escape sequence out-of-bounds");
                         }

                         if (tokn_ptr < tokn_end)
                            *tokn_ptr++ = result;
                         else
                            yywarn("String constant too long");
                       }

<STR>\\x{hexdigit}+    {   /* hex escape sequence - ISO C */
                         uint result;

                         if (sscanf(yytext+2, "%x", &result ) < 1)
                         {
                             yyerr("Invalid hex escape sequence");
                             LAST_STATE();
                             return(INVALID);
                         }

                             /* error, constant is out-of-bounds */
                         if ( result > 0xff ){
                            yyerr("Escape sequence out-of-bounds");
                         } else if (yyleng > 4){
                            yywarn("Hexadecimal escape exceeds two chars");
                         }

                         if (tokn_ptr < tokn_end)
                            *tokn_ptr++ = result;
                         else
                            yywarn("String constant too long");
                       }

<STR>\\{digits}        {  /* Bad escape sequence */
                         yywarn("Bad escape sequence in string");
                        *tokn_ptr++ = '?';
                       }

<STR>\\n               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\n';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\t               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\t';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\r               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\r';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\b               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\b';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\f               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\f';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\v               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\v';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\a               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = '\a';
                         else
                            yywarn("String constant too long");
                       }

<STR>\\(\n)            {  /* String continuation */
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                       }

<STR>\\.               {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = yytext[1];
                         else
                            yywarn("String constant too long");
                       }

<STR>[^\\\n\"]+        {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("String constant too long");
                       }

<CMMT>[^*\n/\\]*       {   /* Inside C-style comment */
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                       }

<CMMT>[^*\n/\\]*\n     {
                         int n = yyleng;
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                         in_preprocess = 0;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                       }
<CMMT>"/"[^*\n]        {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                       }
<CMMT>\\\n             {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                       }
<CMMT>\\[^\n]          {
                         if (tokn_ptr < tokn_end)
                           *tokn_ptr++ = yytext[1];
                         else
                            yywarn("Comment too long");
                         yyless(1);
                       }
<CMMT>"/"\n            {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                         in_preprocess = 0;
                       }
<CMMT>"/*"             {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                         yywarn("/* inside comment");
                       }
<CMMT>"*"+[^*/\n\\]*   {   /* Stars */
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                       }
<CMMT>"*"+[^*/\n\\]*\n {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                         in_preprocess = 0;
                       }
<CMMT>"*"+"/"          {
                         int n = yyleng;
                         if (tokn_ptr + n < tokn_end) {
                             memcpy(tokn_ptr,yytext,n);
                             tokn_ptr += n;
                         } else 
                            yywarn("Comment too long");

                         LAST_STATE();
                         *tokn_ptr = '\0';
                         if ((lvalp->leaf->data.str
                                 = (char*) malloc(strlen(tokn_buff)+1)) == NULL){
                            fputs("Error: Out of memory in Lexer.\n",stderr);
                            return(INVALID);
                         } else {
                            strcpy(lvalp->leaf->data.str,tokn_buff);
                            return(COMMENT);
                         }
                       }

<CC>\\[0-7]{1,3}"'"    {   /* octal escape sequence */
                         uint result;

                         if (sscanf(yytext+1, "%o", &result ) < 1)
                         {
                             yyerr("Invalid octal escape sequence");
                             LAST_STATE();
                             return(INVALID);
                         }
                         
                             /* error, constant is out-of-bounds */
                         if ( result > 0xff ){
                             yywarn("Escape sequence out-of-bounds");
                             result = 0;
                         }

                         lvalp->leaf->data.cval = result;
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\x{hexdigit}+"'"  {   /* hex escape sequence - ISO C */
                         uint result;

                         if (sscanf(yytext+2, "%x", &result ) < 1)
                         {
                             yyerr("Invalid hex escape sequence");
                             LAST_STATE();
                             return(INVALID);
                         }

                             /* error, constant is out-of-bounds */
                         if ( result > 0xff ){
                             yywarn("Escape sequence out-of-bounds");
                             result = 0;
                         } else if (yyleng > 5){
                             yywarn("Hexadecimal escape exceeds two chars");
                             result = 0;
                         }

                         lvalp->leaf->data.cval = (char) result;
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\{digits}"'"      {  /* Bad escape sequence */
                         yywarn("Bad escape sequence in char constant");
                         lvalp->leaf->data.cval = '\0';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\n                 { /* Error unterminated char constant */
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                         yywarn("Unterminated char constant");
                         lvalp->leaf->data.cval = '\0';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\n"'"             {
                         lvalp->leaf->data.cval = '\n';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\t"'"             {
                         lvalp->leaf->data.cval = '\t';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\r"'"             {
                         lvalp->leaf->data.cval = '\r';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\b"'"             {
                         lvalp->leaf->data.cval = '\b';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\f"'"             {
                         lvalp->leaf->data.cval = '\f';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\v"'"             {
                         lvalp->leaf->data.cval = '\v';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\a"'"             {
                         lvalp->leaf->data.cval = '\a';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\\n"'"            {
                         Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 1;  /* Oddity */
                         lvalp->leaf->data.cval = '\n';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>\\."'"             {
                         lvalp->leaf->data.cval = yytext[1];
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>[^']"'"            {
                         lvalp->leaf->data.cval = *yytext;
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>"'"                { /* Empty */
                         yywarn("Empty character constant");
                         lvalp->leaf->data.cval = '\0';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<CC>[^\\\n][^']+"'"    { /* Multiple characters */
                         yywarn("Invalid character constant");
                         lvalp->leaf->data.cval = '\0';
                         LAST_STATE();
                         return(CHAR_CONST);
                       }

<INITIAL>{whitespace}+ ;   /* space/tab/formfeed/vertical tab (ignore) */

<INITIAL,CMMT>\n|\r    { Parse_TOS->yylineno++;
                         Parse_TOS->yynxtcol = 0;
                         in_preprocess = 0;
                       }

<CMMT><<EOF>>          { 
                         yyerr("EOF reached inside comment");
                         in_preprocess = 0;
                         return(0);
                       }

<CC><<EOF>>            { 
                         yyerr("EOF reached inside character constant");
                         return(0);
                       }

<STR><<EOF>>           { 
                         yyerr("EOF reached inside string constant");
                         return(0);
                       }

.                      { /* Any unknown char is an error */
                        fprintf(stderr,
                            "Error: Line %d: Illegal Character",
                            Parse_TOS->yylineno);
                        if ((yytext[0] >= ' ') && (yytext[0] <= '~'))
                            fprintf(stderr,": '%c'\n", yytext[0]);
                        else
                            fprintf(stderr,
                                ", ASCII: %03o (octal)\n", yytext[0]);
                       }

%%

/************************************************************************
//
// yywrap() - This function is called by the lexer [yylex() or sslex()] when
//            the end-of-file (or end-of-string for sslex()) is reached.
//            It gets the next file/string ready (if any) and returns 1
//            which indicates to the lexer that there are no more files
//            remaining.  The lexer returns 0 to the parser, indicating
//            no more tokens are remaining. The parser function, yyparse(),
//            then returns 0, indicating that the parse is done.  This
//            behaviour allows a single item to be parsed to be placed
//            on the stack and parsed without the entire stack being consumed.
//
// **********************************************************************/

int yywrap(void)
{
    in_preprocess = 0;

    /* Prepare the next file if there is one but return 1 */
    /* so that yyparse returns anyway. */

    /*  get_next_file(ParseStack);  */
    return(1);
}

/***********************************************************************/
int cnt_nl(char *txt, int len)
{
  int ret = 0;

  do {
    switch (*txt++) {
      case '\n':
        ret++;
        Parse_TOS->yylineno++;
        Parse_TOS->yynxtcol = 0;
        break;

      default:
        break;
    }
  } while(--len > 0);

  return ret;
}

/***********************************************************************/
/*  Process a #line directive */
void get_lineno(void)
{
    char *pnt;
    int   ln = 0;

    pnt = strchr(line_buf,'#');
    if (pnt)
      {
      pnt++;
      if (*pnt == 'l')
        pnt += 4;
      }

    do {

        if (pnt) {
            if (sscanf(pnt,"%d", &ln) < 1)
                break;

            Parse_TOS->yylineno = ln - 1;
           
            pnt = strchr(pnt,'"');

            if (pnt) {
                char *pnt_end;

                pnt++;

                pnt_end = strchr(pnt,'"');
                if (pnt_end)
                    *pnt_end = '\0';
                else
                    break;

                free(Parse_TOS->filename);
                Parse_TOS->filename = (char *) malloc(strlen(pnt)+1);
                assert(Parse_TOS->filename);
                strcpy(Parse_TOS->filename,pnt); 
            }

            return;
        }

    } while(0);

    yywarn("Malformed #line directive");
}

/***********************************************************************/

void print_caret(char *s, int pos)
{
    char *str;
    int i = 0;

    for (str=s; *str && (i<pos); i++, str++)
        if (*str=='\t')
            fputs("\t",stderr);
        else
            fputs(" ",stderr);

    fputs("^\n",stderr);
}
 
/***********************************************************************/

void yywarn(char *s)
{
    fprintf(stderr,"%s:%d: Warning - %s:\n%s",
         Parse_TOS->filename, Parse_TOS->yylineno, s, line_buf); 
    print_caret(line_buf,Parse_TOS->yycolno);
}

/***********************************************************************/

int yyerr(char *s)
{
    fprintf(stderr,"%s:%d: %s:\n%s",
         Parse_TOS->filename, Parse_TOS->yylineno, s, line_buf); 
    print_caret(line_buf,Parse_TOS->yycolno);

    if (++err_cnt >= 10){
        fprintf(stderr,"Too many errors (%d detected) - quitting.\n",
                err_cnt );
        fclose(yyin);
        return(1);
    }
    return(0); 
}

/***********************************************************************/

int yyerror(char *s)
{
        /* Insure temination */
    line_buf[MAX_TOKN_LEN-1] = '\0';
    fprintf(stderr,"%s:%d: Error (%s) before '%s'\n%s",
         Parse_TOS->filename, Parse_TOS->yylineno,
         s, toksym(err_tok,0), line_buf); 
    print_caret(line_buf,Parse_TOS->yycolno);

    if (++err_cnt >= 10){
        fprintf(stderr,"Too many errors (%d detected) - quitting.\n",
                err_cnt );
        fclose(yyin);
        return(1);
    }
    return(0);
}

/***********************************************************************/
