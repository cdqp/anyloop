#include "xalloc.h"

#include <stdlib.h>
#include <string.h>
#include "logging.h"

void *xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (!ptr) {
		log_fatal("Failed to allocate memory");
		abort();
	}
	return ptr;
}

void *xcalloc(size_t nelem, size_t elsize)
{
	void *ptr = calloc(nelem, elsize);
	if (!ptr) {
		log_fatal("Failed to allocate memory");
		abort();
	}
	return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (!ptr) {
		log_fatal("Failed to allocate memory");
		abort();
	}
	return ptr;
}

void xfree_impl(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}

char *xstrdup(const char *str)
{
	char *str_dup = strdup(str);
	if (!str_dup) {
		log_fatal("Failed to allocate memory");
		abort();
  	}
	return str_dup;
}
