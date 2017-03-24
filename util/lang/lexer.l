%option nounput

%{
    #include "parser.tab.hpp"
    #include <unordered_map>
    #include <string>
    using namespace std;

    unordered_map<string, yytokentype> keywords = {
        {"AND", OP_AND},
        {"OR", OP_OR},
        {"XOR", OP_XOR},
        {"ADD", OP_ADD},
        {"SUB", OP_SUB},
        {"MUL", OP_MUL},
        {"DIV", OP_DIV},
        {"MOD", OP_MOD},
        {"LSR", OP_LSR},
        {"LSL", OP_LSL},
        {"ASR", OP_ASR},
        {"NOT", OP_NOT},
        {"SLICE", OP_SLICE},
        {"MERGE", OP_MERGE},
        {"CLSR", OP_CLSR},
        {"CLSL", OP_CLSL},
        {"CASR", OP_CASR},
        {"DELAY", DELAY},
        {"TRI", TRISTATE},
        {"let", LET},
    };
%}
%%

[ \t\n]                     ;
->                          { return ARROW;}
[[:alpha:]][[:alnum:]]*     {
                                try {
                                    return keywords.at(yytext);
                                } catch(const out_of_range& e) {
                                    yylval.sval = yytext;
                                    return IDENT;
                                }
                            }
[[:digit:]]+                {
                                yylval.ival = atoi(yytext);
                                return NUMBER;
                            }
.                           { return yytext[0]; }
<<EOF>>                     { return TOK_EOF; }
