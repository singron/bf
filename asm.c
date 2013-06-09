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

void emit_header(FILE *f) {
	fprintf(f,"%s",
			".globl _start\n"
			".text\n"
			"bf_inc:\n"
			"    incb (%rbx)\n"
			"    retq\n"
			"bf_dec:\n"
			"    decb (%rbx)\n"
			"    retq\n"
			"bf_right:\n"
			"    inc %rbx\n"
			"    retq\n"
			"bf_left:\n"
			"    dec %rbx\n"
			"    retq\n"
			"bf_input:\n"
			"    movq $0, %rax\n" // sys_read
			"    movq $0, %rdi\n" // fd
			"    movq %rbx, %rsi\n" // buffer
			"    movq $1, %rdx\n" // length
			"    syscall\n"
			"    retq\n"
			"bf_output:\n"
			"    movq $1, %rax\n" // sys_write
			"    movq $1, %rdi\n" // fd
			"    movq %rbx, %rsi\n" // buffer
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
			"# User program starts here\n"
			);
}

void emit_assembly_intern(nlist *list, FILE* f, int *loopid) {
	int i;
	for (i = 0; i < list->size; ++i) {
		if (list->instrs[i].type == NINSTR) {
			switch (list->instrs[i].instr) {
				case '<':
					fprintf(f,"    callq bf_left\n");
					break;
				case '>':
					fprintf(f,"    callq bf_right\n");
					break;
				case '+':
					fprintf(f,"    callq bf_inc\n");
					break;
				case '-':
					fprintf(f,"    callq bf_dec\n");
					break;
				case '.':
					fprintf(f,"    callq bf_output\n");
					break;
				case ',':
					fprintf(f,"    callq bf_input\n");
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

void emit_assembly(nlist *list, FILE* f) {
	emit_header(f);
	int loopid = 0;
	emit_assembly_intern(list, f, &loopid);
	fprintf(f,"%s",
			"#User program ends here\n"
			"    movq $60, %rax\n"
			"    movq $0, %rdi\n"
			"    syscall\n"
			);
}