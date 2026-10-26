%{
#ifdef TAC
#include "tac.tab.h"
#include <string.h>
#else
#include "syntax.tab.h"
#endif
%}

%option noyywrap

ID      [a-zA-Z_][a-zA-Z0-9_]*
NUM     [0-9]+(\.[0-9]+)?

%%

"while"     { return WHILE; }
"for"       { return FOR; }
"if"        { return IF; }
"else"      { return ELSE; }
"int"|"float"|"double" { return TYPE; }

"<="        { return LE; }
">="        { return GE; }
"=="        { return EQ; }
"!="        { return NE; }

{ID} {
#ifdef TAC
    strcpy(yylval.str, yytext);
#endif
    return ID;
}

{NUM} {
#ifdef TAC
    strcpy(yylval.str, yytext);
#endif
    return NUM;
}

[ \t\r\n]+  ;
"//".*      ;

[=+*/(){};,\-<>] { return yytext[0]; }

.           { return INVALID; }

%%