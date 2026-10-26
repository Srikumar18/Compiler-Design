%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);

int temp = 0, label = 0;

char *newtemp(void)
{
    char *s = malloc(30);
    sprintf(s, "t%d", temp++);
    return s;
}

int newlabel(void)
{
    return label++;
}

void emitlabel(int n)
{
    printf("L%d:\n", n);
}

void emitgoto(int n)
{
    printf("goto L%d\n", n);
}

void emitfalse(char *c, int n)
{
    printf("ifFalse %s goto L%d\n", c, n);
}
%}

%union {
    char str[128];
    int num;
}

%token <str> ID NUM
%token WHILE IF ELSE INT INVALID
%token LE GE EQ NE

%type <str> expr condition relop

%left '+' '-'
%left '*' '/'
%right UMINUS
%nonassoc '<' '>' LE GE EQ NE
%nonassoc IFX
%nonassoc ELSE

%%

program
    : statements
    ;

statements
    : /* empty */
    | statements statement
    ;

statement
    : ID '=' expr ';'
      {
          printf("%s = %s\n", $1, $3);
      }
    | INT ID ';'
    | '{' statements '}'
    | while_stmt
    | if_stmt
    ;

while_stmt
    : WHILE '('
      {
          $<num>$ = newlabel();
          emitlabel($<num>$);
      }
      condition ')'
      {
          $<num>$ = newlabel();
          emitfalse($4, $<num>$);
      }
      statement
      {
          emitgoto($<num>3);
          emitlabel($<num>6);
      }
    ;

if_stmt
    : IF '(' condition ')'
      {
          $<num>$ = newlabel();
          emitfalse($3, $<num>$);
      }
      statement %prec IFX
      {
          emitlabel($<num>5);
      }
    | IF '(' condition ')'
      {
          $<num>$ = newlabel();
          emitfalse($3, $<num>$);
      }
      statement ELSE
      {
          $<num>$ = newlabel();
          emitgoto($<num>$);
          emitlabel($<num>5);
      }
      statement
      {
          emitlabel($<num>8);
      }
    ;

condition
    : expr relop expr
      {
          sprintf($$, "%s %s %s", $1, $2, $3);
      }
    ;

relop
    : '<'  { strcpy($$, "<"); }
    | '>'  { strcpy($$, ">"); }
    | LE   { strcpy($$, "<="); }
    | GE   { strcpy($$, ">="); }
    | EQ   { strcpy($$, "=="); }
    | NE   { strcpy($$, "!="); }
    ;

expr
    : expr '+' expr
      {
          char *t = newtemp();
          printf("%s = %s + %s\n", t, $1, $3);
          strcpy($$, t);
      }
    | expr '-' expr
      {
          char *t = newtemp();
          printf("%s = %s - %s\n", t, $1, $3);
          strcpy($$, t);
      }
    | expr '*' expr
      {
          char *t = newtemp();
          printf("%s = %s * %s\n", t, $1, $3);
          strcpy($$, t);
      }
    | expr '/' expr
      {
          char *t = newtemp();
          printf("%s = %s / %s\n", t, $1, $3);
          strcpy($$, t);
      }
    | '-' expr %prec UMINUS
      {
          char *t = newtemp();
          printf("%s = 0 - %s\n", t, $2);
          strcpy($$, t);
      }
    | '(' expr ')' { strcpy($$, $2); }
    | ID           { strcpy($$, $1); }
    | NUM          { strcpy($$, $1); }
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "Syntax error: %s\n", s);
}

int main(void)
{
    return yyparse();
}
