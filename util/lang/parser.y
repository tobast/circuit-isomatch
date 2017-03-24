%code requires {
#include <vector>
#include <string>
using namespace std;

#include <isomatch.h>  // build it against isomatch
#include "parseTools.h"
}

%code {
void yyerror(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
}

extern "C" {
    int yywrap() {
        return 1;
    }
}

extern int yylex(void);
}

%union {
    char* sval;
    int ival;
    CircuitTree* circtree_val;
/*
    vector<string> strvect_val;
    ListElem<string> strlist_val;
    ListElem<CircuitTree*> circtreelist_val;
    vector<CircuitTree*> circtreevect_val;
*/
}

%token OP_AND OP_OR OP_XOR OP_ADD OP_SUB OP_MUL OP_DIV OP_MOD
%token OP_LSR OP_LSL OP_ASR OP_NOT OP_SLICE OP_MERGE
%token OP_CLSR OP_CLSL OP_CASR
%token DELAY TRISTATE ASSERT
%token LET
%token TOK_EOF ARROW
%token <sval> IDENT
%token <ival> NUMBER

%type <circtree_val> entry
/*
%type <circtree_val> group
%type <strvect_val> identCommaList
%type <strlist_val> identCommaListInner
%type <circtreevect_val> stmtList
%type <circtreevect_val> stmtListInner
*/

%%

entry:
     TOK_EOF      { $$ = NULL; }
/*
entry:
     group TOK_EOF      { $$ = $1 }

group:
     LET IDENT
        '(' identCommaList ')' ARROW
        '(' identCommaList ')' '{'
        stmtList
        '}'
                        {
                            $$ = makeGroup($2, $4, $8, $11);
                        }

identCommaList:
    identCommaListInner { $$ = $1.toVect(); }

identCommaListInner:
    IDENT               { $$ = {NULL, $1}; }
  | IDENT identCommaListInner
                        { $$ = {$2, $1}; }

stmtList:
    stmtListInner       { $$ = $1.toVect(); }

stmtListInner:
    'a'                 { $$ = NULL } // TODO
*/
