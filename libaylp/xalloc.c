#include "xalloc.h"

#include <stdlib.h>
#include <string.h>
#include "logging.h"

void *xmalloc(size_t size)
{
	void *ptr = malloc(size);
	return alloc_check(ptr);
}

void *xcalloc(size_t nelem, size_t elsize)
{
	void *ptr = calloc(nelem, elsize);
	return alloc_check(ptr);
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	return alloc_check(ptr);
}

char *xstrdup(const char *str)
{
	char *str_dup = strdup(str);
	return (char *)alloc_check(str_dup);
}

void xfree_impl(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}

void *alloc_check(void *ptr)
{
	if (!ptr) {
		log_fatal("Failed to allocate memory");
		abort();
	}
	return ptr;
}

