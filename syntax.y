%{
#include <stdio.h>
int yylex(void);
void yyerror(const char *s);
%}

%token WHILE FOR IF ELSE TYPE ID NUM INVALID
%token LE GE EQ NE

%left '+' '-'
%left '*' '/'
%right UMINUS
%nonassoc '<' '>' LE GE EQ NE
%nonassoc IFX
%nonassoc ELSE

%%

program : statements { printf("Syntax is correct\n"); };

statements : /* empty */
           | statements statement
           ;

statement : TYPE declarators ';'
          | ID '=' expr ';'
          | '{' statements '}'
          | WHILE '(' condition ')' statement
          | FOR '(' opt_assign ';' condition ';'
                opt_assign ')' statement
          | IF '(' condition ')' statement %prec IFX
          | IF '(' condition ')' statement ELSE statement
          ;

declarators : declarator
            | declarators ',' declarator
            ;

declarator : ID
           | ID '=' expr
           ;

opt_assign : /* empty */
           | ID '=' expr
           ;

condition : expr '<' expr
          | expr '>' expr
          | expr LE expr
          | expr GE expr
          | expr EQ expr
          | expr NE expr
          ;

expr : expr '+' expr
     | expr '-' expr
     | expr '*' expr
     | expr '/' expr
     | '-' expr %prec UMINUS
     | '(' expr ')'
     | ID
     | NUM
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
