#ifndef __BF_AST_H__
#define __BF_AST_H__
struct nlist_s;
struct nloop_s;
struct ninstr_s;

typedef struct nlist_s nlist;
typedef struct nloop_s nloop;
typedef struct ninstr_s ninstr;

struct nlist_s {
	ninstr *instrs;
	int size;
	int max_size;
};

struct nloop_s {
	nlist list;
};

struct ninstr_s {
	union {
		char instr;
		nloop loop;
	};
	enum {
		NINSTR,
		NLOOP
	} type;
};

void nlist_init(nlist *list);
void nlist_add(nlist *list, ninstr* instr);
void nlist_print(nlist *list);
void nlist_destroy(nlist *list);

#endif // __BF_AST_H__
