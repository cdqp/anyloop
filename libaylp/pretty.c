#include <stdio.h>
#include <gsl/gsl_matrix.h>

#include "pretty.h"

void pretty_vector(gsl_vector *v)
{
	fputc('[', stderr);
	size_t i;
	for (i = 0; i < v->size-1; i++) {
		fprintf(stderr, "%G, ",
			v->data[i * v->stride]
		);
	}
	fprintf(stderr, "%G]\n",
		v->data[i * v->stride]
	);
}

void pretty_matrix(gsl_matrix *m)
{
	fputs("[\n", stderr);
	size_t y, x;
	for (y = 0; y < m->size1; y++) {
		fputs("  ", stderr);
		for (x = 0; x < m->size2; x++) {
			fprintf(stderr, "%G, ",
				m->data[y * m->tda + x]
			);
		}
		fputs("\n", stderr);
	}
	fputs("]\n", stderr);
}

