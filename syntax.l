%{
#include "syntax.tab.h"
%}

%option noyywrap

ID      [a-zA-Z_][a-zA-Z0-9_]*
NUM     [0-9]+

%%

"while"     { return WHILE; }
"if"        { return IF; }
"else"      { return ELSE; }
"int"       { return INT; }

"<="        { return LE; }
">="        { return GE; }
"=="        { return EQ; }
"!="        { return NE; }

{ID}        { return ID; }
{NUM}       { return NUM; }

[ \t\r\n]+  ;
"//".*      ;

[=+*/(){};<>-] { return yytext[0]; }

.           { return INVALID; }

%%
