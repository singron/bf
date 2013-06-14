#ifndef __BF_ASM_H__
#define __BF_ASM_H__

#include <ast.h>
#include <stdio.h>

void emit_assembly(nlist *list, FILE* f, int debug);
int lin_con(int a, int b);

#endif //__BF_ASM_H__
