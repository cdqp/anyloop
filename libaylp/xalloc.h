#ifndef AYLP_XALLOC_H_
#define AYLP_XALLOC_H_

#include <stdarg.h>
#include <stdlib.h>

// x*alloc functions behave identically to their normal *alloc counterparts
// except that they abort the program on memory allocation failure
void *xmalloc(size_t size);
void *xcalloc(size_t nelem, size_t elsize);
void *xrealloc(void *ptr, size_t size);
#define xfree(x) xfree_impl((void **) &x)
void xfree_impl(void **ptr);
void *alloc_check(void *ptr);

// these are so we can do (e.g.) `xmalloc_type(gsl_vector, 5)`
#define xmalloc_type(type, ...) \
	(type *)alloc_check(type##_alloc(__VA_ARGS__))
#define xcalloc_type(type, ...) \
	(type *)alloc_check(type##_calloc(__VA_ARGS__))
#define xfree_type(type, ptr) \
	do { type##_free(ptr); ptr=0; } while (0)

char *xstrdup(const char *str);

#endif

