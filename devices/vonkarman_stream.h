#ifndef AYLP_DEVICES_VONKARMAN_SCREEN_H_
#define AYLP_DEVICES_VONKARMAN_SCREEN_H_

#include "anyloop.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>

struct aylp_vonkarman_stream_data {
	// Fried diameter
	double r0;
	// outer scale length
	double L0;
	// physical distance between rows/columns on phase screen
	double pitch;
	// logical size of phase screen
	size_t screen_size;
	// logical width of sliding window on phase screen
	size_t win_width;
	// logical height of sliding window on phase screen
	size_t win_height;
	// random number generator for the Fourier coefficients
	gsl_rng *rng;
	// matrix of the whole phase screen that we slide along
	gsl_matrix *phase_screen;
	// subview of phase screen
	gsl_matrix_view sub_view;
	// current row of 0,0 corner of window
	size_t cur_y;
	// current column of 0,0 corner of window
	size_t cur_x;
	// current step in number of rows
	int cur_step_y;
	// current step in number of columns
	int cur_step_x;
};

// initialize vonkarman_stream device
int vonkarman_stream_init(struct aylp_device *self);

// process vonkarman_stream device once per loop
int vonkarman_stream_proc(struct aylp_device *self, struct aylp_state *state);

// close vonkarman_stream device when loop exits
int vonkarman_stream_fini(struct aylp_device *self);

#endif

