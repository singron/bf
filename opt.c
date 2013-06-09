#include <stdio.h>
#include "opt.h"

int opt_all(nlist *list, int debug) {
	int changes_made = 0;
	int rounds = 0;
	int total_changes_made = 0;

	if (debug) {
		printf("Original Program:\n");
		nlist_print(list);
	}

	do {
		changes_made = 0;
		changes_made += opt_reduce(list);

		if (debug) {
			printf("round %d, %d changes made:\n", rounds, changes_made);
			if (changes_made) {
				nlist_print(list);
			} else {
				printf("duplicate AST ommitted\n");
			}
		}
		total_changes_made += changes_made;
		rounds++;
	} while (changes_made);

	if (debug) {
		printf("%d rounds, %d total changes made\n", rounds, total_changes_made);
	}

	return total_changes_made;
}

int opt_reduce(nlist *list) {
	int changes_made = 0;
	int i;
	for (i = 0; i < list->size; ++i) {
		int repeat = 0;
		if (list->instrs[i].type != NLOOP) {
			if (i < list->size - 1 && list->instrs[i+1].type != NLOOP) {
				ninstr *a = &list->instrs[i];
				ninstr *b = &list->instrs[i+1];
				switch(a->type) {
					case NADD:
						if (b->type == NADD) {
							a->amount += b->amount;
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						} else if (b->type == NSUB) {
							if (a->amount >= b->amount) {
								a->amount -= b->amount;
							} else {
								a->type = NSUB;
								a->amount = b->amount - a->amount;
							}
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						}
						break;
					case NSUB:
						if (b->type == NSUB) {
							a->amount += b->amount;
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						} else if (b->type == NADD) {
							if (a->amount >= b->amount) {
								a->amount -= b->amount;
							} else {
								a->type = NADD;
								a->amount = b->amount - a->amount;
							}
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						}
						break;
					case NLEFT:
						if (b->type == NLEFT) {
							a->amount += b->amount;
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						} else if (b->type == NRIGHT) {
							if (a->amount >= b->amount) {
								a->amount -= b->amount;
							} else {
								a->type = NRIGHT;
								a->amount = b->amount - a->amount;
							}
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						}
						break;
					case NRIGHT:
						if (b->type == NRIGHT) {
							a->amount += b->amount;
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						} else if (b->type == NLEFT) {
							if (a->amount >= b->amount) {
								a->amount -= b->amount;
							} else {
								a->type = NLEFT;
								a->amount = b->amount - a->amount;
							}
							nlist_delete(list, i+1);
							changes_made++;
							repeat = 1;
						}
						break;
					case NINPUT:
						/* input cannot be reduced */
						break;
					case NOUTPUT:
						/* output cannot be reduced */
						break;
					case NLOOP:
						/* compiler complains but this should never show up */
						break;
				}
				if (list->instrs[i].amount == 0) {
					nlist_delete(list, i);
					changes_made++;
				}
			}
			if (repeat)
				i--;
		} else {
			changes_made += opt_reduce(&list->instrs[i].loop.list);
		}
	}
	return changes_made;
}
