#ifndef AYLP_XALLOC_H_
#define AYLP_XALLOC_H_

#include <stdlib.h>

// x*alloc functions behave identically to their normal *alloc counterparts
// except that they abort the program on memory allocation failure
void *xmalloc(size_t size);
void *xcalloc(size_t nelem, size_t elsize);
void *xrealloc(void *ptr, size_t size);
#define xfree(x) xfree_impl((void **) &x)
void xfree_impl(void **ptr);

char *xstrdup(const char *str);

#endif
