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
		changes_made += opt_make_offsets(list);
		changes_made += opt_fixed_loops(list);

		if (debug) {
			printf("round %d, %d changes made:\n", rounds, changes_made);
			nlist_print(list);
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
		if (list->instrs[i].type != NLOOP && list->instrs[i].type != NFIXEDLOOP) {
			if (i < list->size - 1 && list->instrs[i+1].type != NLOOP && list->instrs[i].type != NFIXEDLOOP) {
				ninstr *a = &list->instrs[i];
				ninstr *b = &list->instrs[i+1];
				switch(a->type) {
					case NADD:
						if (a->offset != b->offset)
							break;
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
						if (a->offset != b->offset)
							break;
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
					case NMULTADD:
					case NMULTSUB:
					case NOUTPUT:
						/* cannot be reduced */
						break;
					case NLOOP:
					case NFIXEDLOOP:
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

int opt_fixed_loops_intern(ninstr *instr) {
	int changes_made = 0;
	int i;
	int loop_found = 0;
	int lefts = 0;
	int rights = 0;
	int io = 0;
	nloop *loop = &instr->loop;
	for (i = 0; i < loop->list.size; ++i) {
		switch (loop->list.instrs[i].type) {
			case NLEFT:
				lefts += loop->list.instrs[i].amount;
				break;
			case NRIGHT:
				rights += loop->list.instrs[i].amount;
				break;
			case NINPUT:
			case NOUTPUT:
				io = 1;
				break;
			case NLOOP:
				loop_found = 1;
				changes_made += opt_fixed_loops_intern(&loop->list.instrs[i]);
				break;
			case NFIXEDLOOP:
				/* ignore for now */
				loop_found = 1;
				break;
			case NADD:
			case NSUB:
			case NMULTADD:
			case NMULTSUB:
				break;
		}
	}

	if (loop_found || io || lefts != rights) {
		return changes_made;
	}
	/* all iterations of the loop can be done at once with mults */
	for (i = 0; i < loop->list.size; ++i) {
		switch (loop->list.instrs[i].type) {
			case NADD:
				loop->list.instrs[i].type = NMULTADD;
				break;
			case NSUB:
				loop->list.instrs[i].type = NMULTSUB;
				break;
			case NLOOP:
			case NFIXEDLOOP:
			case NLEFT:
			case NRIGHT:
			case NINPUT:
			case NOUTPUT:
			case NMULTADD:
			case NMULTSUB:
				printf("Should not be here %s:%d\n", __FILE__, __LINE__);
				break;
		}
	}
	instr->type = NFIXEDLOOP;
	instr->amount = 1;
	return changes_made;
}

int opt_fixed_loops(nlist *list) {
	int changes_made = 0;
	int i;
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type == NLOOP) {
			changes_made += opt_fixed_loops_intern(&list->instrs[i]);
		}
	}
	return changes_made;
}


int opt_make_offsets(nlist *list) {
	int changes_made = 0;
	int offset = 0;
	int i;
	for (i = 0; i < list->size; ++i) {
		int repeat = 0;
		switch (list->instrs[i].type) {
			case NLEFT:
				offset -= list->instrs[i].amount;
				nlist_delete(list, i);
				repeat = 1;
				changes_made++;
				break;
			case NRIGHT:
				offset += list->instrs[i].amount;
				nlist_delete(list, i);
				repeat = 1;
				changes_made++;
				break;
			case NADD:
			case NSUB:
			case NMULTADD:
			case NMULTSUB:
			case NINPUT:
			case NOUTPUT:
				list->instrs[i].offset += offset;
				break;
			case NLOOP:
			case NFIXEDLOOP:
				if (offset != 0) {
					nlist_insert(list, &list->instrs[i], i);
					if (offset > 0) {
						list->instrs[i].type = NRIGHT;
						list->instrs[i].amount = offset;
						list->instrs[i].offset = 0;
					} else {
						list->instrs[i].type = NLEFT;
						list->instrs[i].amount = -offset;
						list->instrs[i].offset = 0;
					}
					++i;
					offset = 0;
					if (changes_made)
						changes_made--;
				}
				if (list->instrs[i].type == NLOOP)
					changes_made += opt_make_offsets(&list->instrs[i].loop.list);
				break;
		}
		if (repeat) {
			i--;
		}
	}
	if (offset != 0) {
		nlist_add(list, &list->instrs[0]);
		if (offset > 0) {
			list->instrs[i].type = NRIGHT;
			list->instrs[i].amount = offset;
			list->instrs[i].offset = 0;
		} else {
			list->instrs[i].type = NLEFT;
			list->instrs[i].amount = -offset;
			list->instrs[i].offset = 0;
		}
		offset = 0;
		if (changes_made)
			changes_made--;
	}
	return changes_made;
}
