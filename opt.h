#include "ast.h"

/* do all optimizations */
int opt_all(nlist *list, int debug);
/* combine adjacent associative operations */
int opt_reduce(nlist *list);
/* find terminal loops and try to flatten them out */
int opt_set_loops(nlist *list);
/* create offsets for operations instead of moving head */
int opt_make_offsets(nlist *list);
