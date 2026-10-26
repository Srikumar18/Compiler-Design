%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);

int temp = 0, label = 0;

char *newtemp(void)
{
    char *s = malloc(32);
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
    char a[128], op[16], b[128];

    if (sscanf(c, "%127s %15s %127s", a, op, b) == 3)
        printf("ifFalse %s %s %s goto L%d\n", a, op, b, n);
}

char *gen(char *a, char *op, char *b)
{
    char *s = newtemp();
    printf("%s = %s %s %s\n", s, a, op, b);
    return s;
}
%}

%union {
    char str[128];
    int num;
}

%token <str> ID NUM
%token TYPE IF ELSE WHILE FOR INVALID
%token LE GE EQ NE

%type <str> expr condition relop assignment opt_assign
%type <num> if_head while_head for_head

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
    : TYPE declarators ';'
    | assignment ';'
      {
          printf("%s\n", $1);
      }
    | '{' statements '}'
    | while_stmt
    | for_stmt
    | if_stmt
    ;

declarators
    : declarator
    | declarators ',' declarator
    ;

declarator
    : ID
    | ID '=' expr
      {
          printf("%s = %s\n", $1, $3);
      }
    ;

assignment
    : ID '=' expr
      {
          sprintf($$, "%s = %s", $1, $3);
      }
    ;

opt_assign
    : /* empty */
      {
          $$[0] = '\0';
      }
    | assignment
      {
          strcpy($$, $1);
      }
    ;

if_head
    : IF '(' condition ')'
      {
          $$ = newlabel();
          emitfalse($3, $$);
      }
    ;

if_stmt
    : if_head statement %prec IFX
      {
          emitlabel($1);
      }
    | if_head statement ELSE
      {
          $<num>$ = newlabel();
          emitgoto($<num>$);
          emitlabel($1);
      }
      statement
      {
          emitlabel($<num>4);
      }
    ;

while_head
    : WHILE '('
      {
          $$ = newlabel();
          emitlabel($$);
      }
    ;

while_stmt
    : while_head condition ')'
      {
          $<num>$ = newlabel();
          emitfalse($2, $<num>$);
      }
      statement
      {
          emitgoto($1);
          emitlabel($<num>4);
      }
    ;

for_head
    : FOR '(' opt_assign ';'
      {
          if ($3[0])
              printf("%s\n", $3);

          $$ = newlabel();
          emitlabel($$);
      }
    ;

for_stmt
    : for_head condition ';' opt_assign ')'
      {
          $<num>$ = newlabel();
          emitfalse($2, $<num>$);
      }
      statement
      {
          if ($4[0])
              printf("%s\n", $4);

          emitgoto($1);
          emitlabel($<num>6);
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
    : expr '+' expr { strcpy($$, gen($1, "+", $3)); }
    | expr '-' expr { strcpy($$, gen($1, "-", $3)); }
    | expr '*' expr { strcpy($$, gen($1, "*", $3)); }
    | expr '/' expr { strcpy($$, gen($1, "/", $3)); }
    | '-' expr %prec UMINUS
      {
          strcpy($$, gen("0", "-", $2));
      }
    | '(' expr ')'  { strcpy($$, $2); }
    | ID            { strcpy($$, $1); }
    | NUM           { strcpy($$, $1); }
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