#include <string.h>
#include <gsl/gsl_block.h>
#include <gsl/gsl_matrix.h>
#include "block.h"


gsl_block *mat2blk(gsl_matrix *m)
{
	gsl_block *ret = gsl_block_alloc(m->size1 * m->size2);
	if (m->tda == m->size2) {
		// rows are contiguous
		memcpy(ret->data, m->data, sizeof(double) * ret->size);
	} else {
		// rows are not contiguous; write each row separately
		for (size_t idx=0; idx<m->size1; idx++) {
			memcpy(
				ret->data + idx * m->size2,
				m->data + idx * m->tda,
				sizeof(double) * m->size2
			);
		}
	}
	return ret;
}


gsl_block *vec2blk(gsl_vector *v)
{
	gsl_block *ret = gsl_block_alloc(v->size);
	if (v->stride == 1) {
		// elements are contiguous
		memcpy(ret->data, v->data, sizeof(double) * ret->size);
	} else {
		// elements are not contiguous; write each element separately
		for (size_t idx=0; idx<v->size; idx++) {
			ret->data[idx] = v->data[idx*v->stride];
		}
	}
	return ret;
}

