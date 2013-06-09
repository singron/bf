#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "asm.h"

extern nlist program_begin;
extern int yyparse();

int main(int argc, char ** argv) {
	yyparse();
	//nlist_print(&program_begin);
	emit_assembly(&program_begin, stdout);
	nlist_destroy(&program_begin);
	return 0;
}
