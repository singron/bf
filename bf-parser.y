%{

#include <stdio.h>
#include "ast.h"

void yyerror(const char * s) {printf("ERROR: %s\n", s);}

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
		   | TOK_RIGHT  { $$.instr = '>'; $$.type = NINSTR; }
		   | TOK_LEFT   { $$.instr = '<'; $$.type = NINSTR; }
		   | TOK_INC    { $$.instr = '+'; $$.type = NINSTR; }
		   | TOK_DEC    { $$.instr = '-'; $$.type = NINSTR; }
		   | TOK_OUTPUT { $$.instr = '.'; $$.type = NINSTR; }
		   | TOK_INPUT  { $$.instr = ','; $$.type = NINSTR; }
		   ;

loop: TOK_LOOP_START list TOK_LOOP_END { $$.list = $2; }
	| TOK_LOOP_START TOK_LOOP_END { nlist_init(&$$.list); }
	;

