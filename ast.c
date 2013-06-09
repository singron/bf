#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

void nlist_init(nlist *list) {
	list->size = list->max_size = 0;
	list->instrs = NULL;
}

void nlist_add(nlist *list, ninstr* instr) {
	if (list->size == list->max_size) {
		if (list->max_size == 0)
			list->max_size = 4;
		list->max_size *= 2;
		list->instrs = realloc(list->instrs, list->max_size*sizeof(ninstr));
	}
	list->instrs[list->size] = *instr;
	++list->size;
}

void nlist_print_indent(nlist *list, int indent) {
	int i;
	char * spaces = alloca(indent+1);
	spaces[indent] = '\0';
	memset(spaces, ' ', indent);
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type == NINSTR) {
			printf("%c", list->instrs[i].instr);
		} else {
			if (list->instrs[i].loop.list.size == 0) {
				/* shorten empty loops */
				printf("[]");
			} else {
				printf("\n%s[\n  %s", spaces, spaces);
				nlist_print_indent(&list->instrs[i].loop.list, indent+2);
				printf("\n%s]\n  %s", spaces, spaces);
			}
		}
	}
}

void nlist_print(nlist *list) {
	nlist_print_indent(list, 0);
}

void nlist_destroy(nlist *list) {
	int i;
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type == NLOOP) {
			nlist_destroy(&list->instrs[i].loop.list);
		}
	}
	free(list->instrs);
}
