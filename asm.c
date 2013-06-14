#include <stdio.h>
#include <sys/mman.h>
#include "ast.h"
#include "asm.h"


#ifndef MAX_HEAP
#define MAX_HEAP 0x10000 // 64k
#endif

#define TAPE_START 0x100000


#define _STR(x) #x
#define STR(x) _STR(x)

int isPowerOfTwo (unsigned int x)
{
	  return ((x != 0) && ((x & (~x + 1)) == x));
}

/* euclidian extended Elgorithm? */
inline void eee (int a, int b, int *gcd, int *x, int *y) {
	*x=0, *y=1; 
	int u=1, v=0, m, n, q, r;
	*gcd = b;
	while (a!=0) {
		q=*gcd/a; r=*gcd%a;
		m=*x-u*q; n=*y-v*q;
		*gcd=a; a=r; *x=u; *y=v; u=m; v=n;
	}
}

/* linear congruence solver of the form a*x = b (mod 256) 
 * gives the lowest positive integer answer, or 0 if there isn't one */
int lin_con(int a, int b) {
	int d;
	int r;
	int s;
	eee(a, 256, &d, &r, &s);
	if (b % d != 0)
		return 0;
	int x = r*b/d;
	while (x < 0) {
		x += 256;
	}
	while (x - 256 >= 0) {
		x -= 256;
	}
	int inc = ((d<0) ? -d : d) / 256;
	printf("inc = %d\n", inc);
	while (inc && x - inc >= 0) {
		x -= inc;
	}
	return x;
}

void emit_gcd_table(FILE *f) {
	int a;
	int b;
	fprintf(f,".data\nlin_con_table:\n");
	for (a = 0; a < 256; ++a) {
		for (b = 0; b < 256; ++b) {
			fprintf(f, ".byte %d\n", lin_con(a,-b));
		}
	}
}

void emit_header(FILE *f) {
	fprintf(f,"%s",
			".globl _start\n"
			".text\n"
			"bf_input:\n" // buffer passed in rsi
			"    movq $0, %rax\n" // sys_read
			"    movq $0, %rdi\n" // fd
			// "    movq %rbx, %rsi\n" // buffer
			"    movq $1, %rdx\n" // length
			"    syscall\n"
			"    retq\n"
			"bf_output:\n" // buffer passed in rsi
			"    movq $1, %rax\n" // sys_write
			"    movq $1, %rdi\n" // fd
			//"    movq %rbx, %rsi\n" // buffer
			"    movq $1, %rdx\n" // length
			"    syscall\n"
			"    retq\n"
			"\n"
			"_start:\n"
			/* set up tape */
			"    movq $9, %rax\n" // sys_mmap
			"    movq $" STR(TAPE_START) ", %rdi\n" // addr
			"    movq $" STR(MAX_HEAP) ", %rsi\n" // length
			);
	fprintf(f,
			"    movq $%d, %%rdx\n", PROT_READ | PROT_WRITE); // protection
	fprintf(f,
			"    movq $%d, %%r10\n", MAP_PRIVATE | MAP_ANONYMOUS );
	fprintf(f,"%s",
			"    movq $-1, %r8\n"
			"    syscall\n" // result in rax
			"    movq %rax, %rbx\n"
			);
}

void emit_fixed_loop(nloop *loop, FILE *f, int *loopid) {
	int i;
	nlist *list = &loop->list;
	int i_inc = 0;
	int tmpid = *loopid;
	(*loopid)++;
	/* find how much the index variable would be changed each loop */
	for (i = 0; i < list->size; ++i) {
		ninstr *a = &list->instrs[i];
		switch (list->instrs[i].type) {
			case NMULTADD:
				if (a->offset == 0) {
					i_inc += a->amount;
				}
				break;
			case NMULTSUB:
				if (a->offset == 0) {
					i_inc -= a->amount;
				}
				break;
			case NADD:
			case NSUB:
			case NLEFT:
			case NRIGHT:
			case NOUTPUT:
			case NINPUT:
			case NFIXEDLOOP:
			case NLOOP:
				break;
		}
	}
	while (i_inc > 255) {
		i_inc -= 256;
	}
	while (i_inc < 0) {
		i_inc += 256;
	}
	/* skip maybe */
	fprintf(f,"    cmpb $0 ,(%%rbx)\n");
	fprintf(f,"    je loop%d_end\n", tmpid);

	/* find how many loops would be made, store in rcx */
	fprintf(f,"    movzxb (%%rbx), %%rax\n"); //b
	fprintf(f,"    addq $lin_con_table, %%rax\n"); // lin_con_table + b
	fprintf(f,"    movq $%d, %%rcx\n", i_inc * 256); // a * 256
	/* addr = lin_con_table + b + a * 256 */
	fprintf(f,"    movzxb (%%rax,%%rcx), %%rcx\n");


	for (i = 0; i < list->size; ++i) {
		ninstr *a = &list->instrs[i];
		switch (list->instrs[i].type) {
			case NMULTADD:
				if (a->amount == 1) {
					fprintf(f,"    addb %%cl, %d(%%rbx)\n", a->offset);
				} else if (isPowerOfTwo(a->amount)) {
					fprintf(f,"    movq %%rcx, %%rax\n");
					fprintf(f,"    shlq $%d, %%rax\n", __builtin_ffs(a->amount)-1);
					fprintf(f,"    addb %%al, %d(%%rbx)\n", a->offset);
				} else {
					fprintf(f,"    movq %%rcx, %%rax\n");
					fprintf(f,"    movq $%d, %%rdx\n", a->amount);
					fprintf(f,"    mulq %%rdx\n");
					fprintf(f,"    addb %%al, %d(%%rbx)\n", a->offset);
				}
				break;
			case NMULTSUB:
				if (a->amount == 1) {
					fprintf(f,"    subb %%cl, %d(%%rbx)\n", a->offset);
				} else if (isPowerOfTwo(a->amount)) {
					fprintf(f,"    movq %%rcx, %%rax\n");
					fprintf(f,"    shlq $%d, %%rax\n", __builtin_ffs(a->amount)-1);
					fprintf(f,"    subb %%al, %d(%%rbx)\n", a->offset);
				} else {
					fprintf(f,"    movq %%rcx, %%rax\n");
					fprintf(f,"    movq $%d, %%rdx\n", a->amount);
					fprintf(f,"    mulq %%rdx\n");
					fprintf(f,"    subb %%al, %d(%%rbx)\n", a->offset);
				}
				break;
			case NLEFT:
				break;
			case NRIGHT:
				break;
			case NFIXEDLOOP:
				/* emit_fixed_loop(f,&a); */
				// break
			case NADD:
			case NSUB:
			case NOUTPUT:
			case NINPUT:
			case NLOOP:
				/* should never get here */
				break;
		}
	}
	fprintf(f,"loop%d_end:\n", tmpid);
}

void emit_assembly_intern(nlist *list, FILE* f, int *loopid) {
	int i;
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type != NLOOP) {
			ninstr *a = &list->instrs[i];
			switch (list->instrs[i].type) {
				case NLEFT:
					if (a->amount == 1) {
						fprintf(f,"    decq %%rbx\n");
					} else {
						fprintf(f,"    subq $%d, %%rbx\n", a->amount);
					}
					break;
				case NRIGHT:
					if (a->amount == 1) {
						fprintf(f,"    incq %%rbx\n");
					} else {
						fprintf(f,"    addq $%d, %%rbx\n", a->amount);
					}
					break;
				case NADD:
					if (a->amount == 1) {
						if (a->offset)
							fprintf(f,"    incb %d(%%rbx)\n", a->offset);
						else
							fprintf(f,"    incb (%%rbx)\n");
					} else {
						if (a->offset)
							fprintf(f,"    addb $%d, %d(%%rbx)\n", a->amount, a->offset);
						else
							fprintf(f,"    addb $%d, (%%rbx)\n", a->amount);
					}
					break;
				case NSUB:
					if (a->amount == 1) {
						if (a->offset)
							fprintf(f,"    decb %d(%%rbx)\n", a->offset);
						else
							fprintf(f,"    decb (%%rbx)\n");
					} else {
						if (a->offset)
							fprintf(f,"    subb $%d, %d(%%rbx)\n", a->amount, a->offset);
						else
							fprintf(f,"    subb $%d, (%%rbx)\n", a->amount);
					}
					break;
				case NOUTPUT:
					if (a->offset)
						fprintf(f,"    leaq %d(%%rbx), %%rsi\n", a->offset);
					else
						fprintf(f,"    movq %%rbx, %%rsi\n");
					fprintf(f,"    callq bf_output\n");
					break;
				case NINPUT:
					if (a->offset)
						fprintf(f,"    leaq %d(%%rbx), %%rsi\n", a->offset);
					else
						fprintf(f,"    movq %%rbx, %%rsi\n");
					fprintf(f,"    callq bf_input\n");
					break;
				case NFIXEDLOOP:
					emit_fixed_loop(&a->loop, f, loopid);
					break;
				case NMULTADD:
				case NMULTSUB:
				case NLOOP:
					/* should never get here */
					break;
			}
		} else {
			int tmpid;
			tmpid = *loopid;
			++(*loopid);
			fprintf(f,
					"    cmpb $0, (%%rbx)\n"
					"    je loop%d_end\n"
					"loop%d:\n",
					tmpid, tmpid);
			emit_assembly_intern(&list->instrs[i].loop.list, f, loopid);
			fprintf(f,
					"    cmpb $0, (%%rbx)\n"
					"    jne loop%d\n"
					"loop%d_end:\n",
					tmpid, tmpid);
		}
	}
}

void emit_assembly(nlist *list, FILE* f, int debug) {
	if (!debug) {
		/* it's really long and makes stuff hard to read */
		emit_gcd_table(f);
	}
	emit_header(f);
	int loopid = 0;
	fprintf(f,"#User program starts here\n");
	emit_assembly_intern(list, f, &loopid);
	fprintf(f,"%s",
			"#User program ends here\n"
			"    movq $60, %rax\n"
			"    movq $0, %rdi\n"
			"    syscall\n"
			);
}
