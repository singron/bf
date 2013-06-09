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

void nlist_delete(nlist *list, int n) {
	int i;

	if (list->instrs[n].type == NLOOP) {
		nlist_destroy(&list->instrs[n].loop.list);
	}
	list->size--;
	for (i = n; i < list->size; ++i) {
		list->instrs[i] = list->instrs[i+1];
	}
}

void nlist_print_indent(nlist *list, int indent) {
	int i;
	char * spaces = alloca(indent+1);
	spaces[indent] = '\0';
	memset(spaces, ' ', indent);
	int indented = 0;
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type != NLOOP) {
			char c;
			switch(list->instrs[i].type) {
				case NADD:
					c = '+';
					break;
				case NSUB:
					c = '-';
					break;
				case NLEFT:
					c = '<';
					break;
				case NRIGHT:
					c = '>';
					break;
				case NINPUT:
					c = ',';
					break;
				case NOUTPUT:
					c = '.';
					break;
				case NLOOP:
					/* shouldn't happen */
					break;
			}
			if (!indented) {
				printf(spaces);
				indented = 1;
			}
			if (list->instrs[i].amount == 1) {
				printf("%c", c);
			} else {
				printf("%d%c", list->instrs[i].amount, c);
			}
		} else {
			if (list->instrs[i].loop.list.size == 0) {
				/* shorten empty loops */
				printf("[]");
			} else {
				printf("\n%s[\n", spaces);
				nlist_print_indent(&list->instrs[i].loop.list, indent+2);
				printf("%s]\n", spaces);
				indented = 0;
			}
		}
	}
	printf("\n");
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
