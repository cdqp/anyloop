#include <string.h>
#include <gsl/gsl_block.h>
#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "logging.h"
#include "block.h"
#include "xalloc.h"


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


/** Finds or makes the contiguous gsl_block_uchar of the current state.
 * Returns 1 if we allocated new memory for bytes->data, zero if we didn't, and
 * negative on error.
 */
int get_contiguous_bytes(gsl_block_uchar *bytes, struct aylp_state *state)
{
	switch (state->header.type) {
	case AYLP_T_BLOCK:
		bytes->size = sizeof(double) * state->block->size;
		bytes->data = (unsigned char *)state->block->data;
		return 0;
	case AYLP_T_VECTOR: {
		gsl_vector *v = state->vector;	// brevity
		bytes->size = sizeof(double) * v->size;
		if (LIKELY(v->stride == 1)) {
			bytes->data = (unsigned char *)v->data;
			return 0;
		} else {
			bytes->data = xmalloc(bytes->size);
			for (size_t i = 0; i < v->size; i++) {
				bytes->data[i*sizeof(double)] =
					v->data[i * v->stride]
				;
			}
			return 1;
		}
	}
	case AYLP_T_MATRIX: {
		gsl_matrix *m = state->matrix;	// brevity
		bytes->size = sizeof(double) * m->size1 * m->size2;
		if (LIKELY(m->tda == m->size2)) {
			// rows are contiguous
			bytes->data = (unsigned char *)m->data;
			return 0;
		} else {
			// rows are not contiguous
			bytes->data = xmalloc(bytes->size);
			for (size_t i = 0; i < m->size1; i++) {
				memcpy(bytes->data + sizeof(double)*i*m->size2,
					m->data + i*m->tda,
					sizeof(double)*m->size2
				);
			}
			return 1;
		}
	}
	case AYLP_T_BYTES:
		bytes = state->bytes;
		return 0;
	default:
		log_error("Bug: unsupported type 0x%hX", state->header.type);
		return -1;
	}
}

