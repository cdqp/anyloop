#ifndef _OPENAO_VONKARMAN_SCREEN_H
#define _OPENAO_VONKARMAN_SCREEN_H

#include "openao.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>

struct oao_vonkarman_screen_data {
	double Cn2;
	double L;
	double pitch;
	size_t width;
	gsl_rng *rng;
	gsl_matrix *phase_screen;
};

// initialize vonkarman_screen device
int vonkarman_screen_init(struct oao_device *self);

// process vonkarman_screen device once per loop
int vonkarman_screen_process(struct oao_device *self, struct oao_state *state);

// close vonkarman_screen device when loop exits
int vonkarman_screen_close(struct oao_device *self);

#endif

