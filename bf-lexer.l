%{
#include <stdio.h>
#include "bf-parser.tab.h"
	int linenum = 1;
%}
%option noyywrap
%option nounput
%option noinput
%%


\n  { ++linenum; }

">" { return TOK_RIGHT; }
"<" { return TOK_LEFT; }
"+" { return TOK_INC; }
"-" { return TOK_DEC; }
"." { return TOK_OUTPUT; }
"," { return TOK_INPUT; }
"[" { return TOK_LOOP_START; }
"]" { return TOK_LOOP_END; }
. ;

%%
