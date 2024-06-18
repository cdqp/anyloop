#ifndef AYLP_DEVICES_VONKARMAN_SCREEN_H_
#define AYLP_DEVICES_VONKARMAN_SCREEN_H_

#include "anyloop.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>

/** aylp:vonkarman_stream
 *
 * types and units: `[T_ANY, U_ANY] -> [T_MATRIX, U_RAD]`
 *
 * This device fills the pipeline with a matrix of simulated von Kármán
 * turbulence. A large static phase screen is generated at startup, and (in
 * keeping with the frozen flow approximation) a small window is slid along this
 * phase screen as the loop runs. The contents of this window are then put into
 * the pipeline. This device has been used in conjunction with a deformable
 * mirror to simulate atmospheric turbulence in the lab.
 *
 * See: <https://doi.org/10.1364/AO.43.004527>,
 * <https://doi.org/10.1117/12.279029>.
 *
 * Parameters:
 *   - `L0` (float) (required)
 *     - The outer scale length of the turbulence that you want to simulate.
 *   - `r0` (float) (required)
 *     - The Fried parameter of the turbulence that you want to simulate.
 *   - `pitch` (float) (required)
 *     - The physical distance between pixels on the phase screen (in meters).
 *   - `screen_size` (integer) (required)
 *     - Width (and height) of the square phase screen. Submatrices of this
 *       large screen will be placed into the pipeline.
 *   - `win_height` (integer) (optional)
 *     - Height of the window into the phase screen. Defaults to 10.
 *   - `win_width` (integer) (optional)
 *     - Width of the window into the phase screen. Defaults to 10.
 *   - `start_y` (integer) (optional)
 *     - Starting y-index of the window in the phase screen. Defaults to 0.
 *   - `start_x` (integer) (optional)
 *     - Starting x-index of the window in the phase screen. Defaults to 0.
 *   - `step_y` (integer) (optional)
 *     - How much to move the window in the y-direction every iteration.
 *       Proportional to the vertical component of wind speed under the frozen
 *       flow approximation. Defaults to 0.
 *   - `step_x` (integer) (optional)
 *     - How much to move the window in the x-direction every iteration.
 *       Proportional to the horizontal component of wind speed under the frozen
 *       flow approximation. Defaults to 0.
 */

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
int vonkarman_stream_process(
	struct aylp_device *self, struct aylp_state *state
);

// close vonkarman_stream device when loop exits
int vonkarman_stream_close(struct aylp_device *self);

#endif

