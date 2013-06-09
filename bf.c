#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "asm.h"
#include "opt.h"

extern nlist program_begin;
extern int yyparse();

int main(int argc, char ** argv) {
	int i;
	int debug = 0;
	int optimize = 1;

	for (i = 0; i < argc; ++i) {
		if (strcmp("-d", argv[i])==0) {
			debug = 1;
		} else if (strcmp("-O0", argv[i])==0) {
			optimize = 0;
		} else if (strncmp("-O", argv[i], 2)==0) {
			optimize = 1;
		}
	}

	yyparse();

	if (optimize) {
		opt_all(&program_begin, debug);
	}
	emit_assembly(&program_begin, stdout);
	nlist_destroy(&program_begin);

	return 0;
}
