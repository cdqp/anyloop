#ifndef _OPENAO_VONKARMAN_SCREEN_H
#define _OPENAO_VONKARMAN_SCREEN_H

#include "openao.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>

struct oao_vonkarman_stream_data {
	double r0;
	double L0;
	double pitch;
	size_t width;
	gsl_rng *rng;
	gsl_matrix *phase_screen;
	size_t cur_idx;
	size_t cur_jdx;
	int cur_step;
};

// initialize vonkarman_stream device
int vonkarman_stream_init(struct oao_device *self);

// process vonkarman_stream device once per loop
int vonkarman_stream_process(struct oao_device *self, struct oao_state *state);

// close vonkarman_stream device when loop exits
int vonkarman_stream_close(struct oao_device *self);

#endif

