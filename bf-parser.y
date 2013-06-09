%{

#include <stdio.h>
#include "ast.h"

void yyerror(const char * s) {printf("ERROR: %s\n", s);}
int yylex(void);

nlist program_begin;

%}
%union {
	nloop loop;
	nlist list;
	ninstr instr;
	int token;
}

%token <token> TOK_RIGHT
%token <token> TOK_LEFT
%token <token> TOK_INC
%token <token> TOK_DEC
%token <token> TOK_OUTPUT
%token <token> TOK_INPUT
%token <token> TOK_LOOP_START
%token <token> TOK_LOOP_END

%type <loop> loop
%type <list> list
%type <list> program
%type <instr> instruction

%start program

%%

program: list { program_begin = $$ = $1; }

list: list instruction { $$ = $1; nlist_add(&$$, &$2); }
	|  { nlist_init(&$$); }
	;

instruction: loop       { $$.loop = $1; $$.type = NLOOP; }
		   | TOK_RIGHT  { $$.amount = 1; $$.type = NRIGHT; }
		   | TOK_LEFT   { $$.amount = 1; $$.type = NLEFT; }
		   | TOK_INC    { $$.amount = 1; $$.type = NADD; }
		   | TOK_DEC    { $$.amount = 1; $$.type = NSUB; }
		   | TOK_OUTPUT { $$.amount = 1; $$.type = NOUTPUT; }
		   | TOK_INPUT  { $$.amount = 1; $$.type = NINPUT; }
		   ;

loop: TOK_LOOP_START list TOK_LOOP_END { $$.list = $2; }
	;

