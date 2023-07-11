#ifndef AYLP_BLOCK_H_
#define AYLP_BLOCK_H_

#include "anyloop.h"

#include <gsl/gsl_matrix.h>

// due to tda and stride considerations, we cannot simply memcpy() a matrix or
// vector to a gsl_block; this file makes that simpler

// write matrix to continuous block
gsl_block *mat2blk(gsl_matrix *m);

// write vector to continuous block
gsl_block *vec2blk(gsl_vector *v);

// get contiguous gsl_blocK_uchar from state
int get_contiguous_bytes(gsl_block_uchar *bytes, struct aylp_state *state);

#endif

