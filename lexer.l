%{
#include <stdio.h>
%}

%option noyywrap

%%
"while"|"for"|"if"|"else"|"int"|"float"|"double"  { printf("KEYWORD: %s\n", yytext); }
[a-zA-Z_][a-zA-Z0-9_]*                            { printf("IDENTIFIER: %s\n", yytext); }
[0-9]+(\.[0-9]+)?                                 { printf("NUMBER: %s\n", yytext); }
"<="|">="|"=="|"!="|"<"|">"                       { printf("RELATIONAL: %s\n", yytext); }
"="                                               { printf("ASSIGNMENT: %s\n", yytext); }
"+"|"-"|"*"|"/"                                   { printf("ARITHMETIC: %s\n", yytext); }
"("|")"|"{"|"}"|";"|","                           { printf("DELIMITER: %s\n", yytext); }                                            
[ \t\r\n]+                                        ;
"//".*                                            ;
.                                                 { printf("INVALID: %s\n", yytext); }
%%

int main(void)
{
    yylex();
    return 0;
}
