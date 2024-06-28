#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_block.h>
#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "logging.h"
#include "block.h"
#include "xalloc.h"


/** Finds or makes the contiguous gsl_block_uchar of the current state.
 * Returns 1 if we allocated new memory for bytes->data, zero if we didn't, and
 * negative on error.
 */
int get_contiguous_bytes(gsl_block_uchar *bytes, struct aylp_state *state)
{
	switch (state->header.type) {
	case 0: {
		log_error("Pipeline type is null");
		return -EPIPE;
	}
	case AYLP_T_BLOCK: {
		bytes->size = sizeof(double) * state->block->size;
		bytes->data = (unsigned char *)state->block->data;
		log_trace("got block of %zu doubles", state->block->size);
		return 0;
	}
	case AYLP_T_VECTOR: {
		gsl_vector *v = state->vector;
		bytes->size = sizeof(double) * v->size;
		if (LIKELY(v->stride == 1)) {
			log_trace("got contiguous vector of %zu doubles",
				state->vector->size
			);
			bytes->data = (unsigned char *)v->data;
			return 0;
		} else {
			bytes->data = xmalloc(bytes->size);
			for (size_t i = 0; i < v->size; i++) {
				bytes->data[i*sizeof(double)] =
					v->data[i * v->stride]
				;
			}
			log_trace("got non-contiguous vector of %zu doubles",
				state->vector->size
			);
			return 1;
		}
	}
	case AYLP_T_MATRIX: {
		gsl_matrix *m = state->matrix;
		bytes->size = sizeof(double) * m->size1 * m->size2;
		if (LIKELY(m->tda == m->size2)) {
			// rows are contiguous
			bytes->data = (unsigned char *)m->data;
			log_trace("got contiguous matrix of %zu by %zu "
				"doubles",
				state->matrix->size1, state->matrix->size2
			);
			return 0;
		} else {
			// rows are not contiguous
			bytes->data = xmalloc(bytes->size);
			log_trace("got non-contiguous matrix of %zu by %zu "
				"doubles",
				state->matrix->size1, state->matrix->size2
			);
			for (size_t i = 0; i < m->size1; i++) {
				memcpy(bytes->data + sizeof(double)*i*m->size2,
					m->data + i*m->tda,
					sizeof(double)*m->size2
				);
			}
			return 1;
		}
	}
	case AYLP_T_BLOCK_UCHAR: {
		bytes->size = state->block_uchar->size;
		bytes->data = state->block_uchar->data;
		log_trace("got block of %zu uchars", bytes->size);
		return 0;
	}
	case AYLP_T_MATRIX_UCHAR: {
		gsl_matrix_uchar *m = state->matrix_uchar;
		bytes->size = m->size1 * m->size2;
		if (LIKELY(m->tda == m->size2)) {
			// rows are contiguous
			bytes->data = m->data;
			log_trace("got contiguous matrix of %zu uchars",
				bytes->size
			);
			return 0;
		} else {
			// rows are not contiguous
			bytes->data = xmalloc(bytes->size);
			log_trace("got non-contiguous matrix of %zu uchars",
				bytes->size
			);
			for (size_t i = 0; i < m->size1; i++) {
				memcpy(bytes->data + i*m->size2,
					m->data + i*m->tda,
					m->size2
				);
			}
			return 1;
		}
	}
	default: {
		log_fatal("Bug: unsupported type 0x%hhX", state->header.type);
		exit(EXIT_FAILURE);
		return -1;
	}
	}
}

