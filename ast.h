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
		int offset; // used with non-NLOOP
		nloop loop; // used with NLOOP
	};
	int amount; 
	enum {
		NLEFT,
		NRIGHT,
		NADD,
		NSUB,
		NINPUT,
		NOUTPUT,
		NLOOP,
		NMULTADD,
		NMULTSUB,
		NFIXEDLOOP
	} type;
};

void nlist_init(nlist *list);
void nlist_add(nlist *list, ninstr* instr);
void nlist_delete(nlist *list, int n);
void nlist_insert(nlist *list, ninstr *instr, int n);
void nlist_print(nlist *list);
void nlist_destroy(nlist *list);

#endif // __BF_AST_H__
