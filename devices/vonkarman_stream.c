#include <math.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <json-c/json.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "vonkarman_stream.h"


/** Return the Fourier amplitude (not power) of the von Kármán spectrum.
 * See: https://doi.org/10.1364/AO.43.004527 and
 * https://doi.org/10.1117/12.279029.
 * L0 is outer scale length [m],
 * r0 is Fried parameter [m],
 * Gx and Gy are width and height of phase screen [m],
 * x and y are dimensionless (indices), and
 * output is dimensionless (1/px).
 * Ivy's original MATLAB code was (escaping the at sign)
 * karmanSpec = atsign(L0,r0,Gx,Gy,x,y) (0.15132*(Gx*Gy)^(-1/2)*r0^(-5/6) ...
 *     * ((x/Gx).^2+(y/Gy).^2+1/L0^2).^(-11/12));
 */
double _karman_spec(double L0, double r0, double Gx, double Gy,
	size_t x, size_t y
){
	return (
		0.15132
		/ sqrt(Gx*Gy)
		/ pow(r0, 5.0/6.0)
		/ pow(gsl_hypot3(x/Gx, y/Gy, 1/L0), 11.0/6.0)
	);
}


/** Compute the 1D in-place half-complex-to-real backward FFT.
 * Uses gsl, of course, choosing which routine to use depending on if the size
 * is a power of two.
 */
void _backward_fft(double data[], size_t stride, size_t n)
{
	int err;
	// check if size is power of 2
	//if (!(n & (n-1))) {
	// just kidding, do NOT do this; the radix2 function expects the real
	// and imaginary parts to be far away from one another; if you uncomment
	// the above power-of-2 check, make sure to shift things around or
	// something (seems not worth it)
	if (0) {
		err = gsl_fft_halfcomplex_radix2_backward(data, stride, n);
	} else {
		gsl_fft_halfcomplex_wavetable *wt =
			gsl_fft_halfcomplex_wavetable_alloc(n);
		gsl_fft_real_workspace *ws = gsl_fft_real_workspace_alloc(n);
		// this function is not actually mentioned in the docs, but it's
		// in gsl_fft_halfcomplex.h in the sources, and presumably works
		// the same way as gsl_fft_halfcomplex_transform() (which *is*
		// documented), but with the sign change in the exponent
		err = gsl_fft_halfcomplex_backward(data, stride, n, wt, ws);
		gsl_fft_halfcomplex_wavetable_free(wt);
		gsl_fft_real_workspace_free(ws);
	}
	if (err) {
		log_error("Backward fft failed, gsl_errno=%d\n", err);
	}
	return;
}


/** Generate a von Kármán phase screen.
 * This function generates a von Kármán phase screen by first weighting the
 * spectrum with Hermitian unit gaussian noise, then taking the inverse Fourier
 * transform of that 2D spectrum. This method significantly undersamples low
 * frequency components. There are ways to compensate for this, but I have yet
 * to implement them. For these other ways, see
 * https://doi.org/10.1364/OE.14.000988, https://doi.org/10.1364/OE.20.000681,
 * and (again) parts of https://doi.org/10.1364/AO.43.004527. That being said,
 * complete accuracy is not a huge priority, because the Fried diameter is by no
 * means a certain quantity and fluctuates greatly based on time and
 * environment.
 */
int _generate_phase_screen(struct aylp_device *self)
{
	struct aylp_vonkarman_stream_data *data = self->device_data;
	// We assume we're making a square phase screen; this is generally a
	// good idea (see https://doi.org/10.1364/AO.37.004605), but this could
	// be changed here if one wishes.
	data->phase_screen = gsl_matrix_alloc(
		data->screen_size,
		data->screen_size
	);
	// We fill a matrix with Gaussian noise to act as the phasors by which
	// we weight our IFFT. We don't need the phasors to be complex, because
	// the IFFT later will make them conjugate-symmetric. However, we do
	// need to make sure their rms is only 1/sqrt(2), rather than 1, because
	// the symmetrization adds an imaginary part. Oh, and we multiply them
	// by the spectrum as we go.
	size_t M = data->phase_screen->size1;
	size_t N = data->phase_screen->size2;
	log_trace("Allocated %u by %u matrix for phase screen", M, N);
	for (size_t x=0; x<M; x++) {
		for (size_t y=0; y<N; y++) {
			// von Kármán spectrum amplitude
			double amp = _karman_spec(
				data->L0, data->r0,
				data->screen_size * data->pitch,
				data->screen_size * data->pitch,
				// divide by two since we're calculating both
				// complex and real parts next to each other
				x/2, y/2
			);
			gsl_matrix_set(data->phase_screen, x, y,
				gsl_ran_gaussian(data->rng, 1.0/sqrt(2)) * amp
			);
		}
	}
	// set the 0,0 element to zero (it's nonphysical)
	gsl_matrix_set(data->phase_screen, 0, 0, 0.0);
	log_trace("Generated Fourier components");
	// perform a 2D backward-fft by doing a bunch of 1D backward ffts
	for (size_t x=0; x<M; x++) {
		// backward fft each row
		_backward_fft(
			data->phase_screen->data + x*N,
			1, N
		);
	}
	for (size_t y=0; y<N; y++) {
		// backward fft each column
		_backward_fft(
			data->phase_screen->data + y,
			data->phase_screen->tda, M
		);
	}
	if (log_get_level() <= LOG_DEBUG) {
		// DEBUG: write the matrix to a file
		log_debug("Writing screen to screen.bin");
		FILE *fp = fopen("screen.bin", "wb");
		gsl_matrix_fwrite(fp, data->phase_screen);
		fflush(fp);
		fclose(fp);
	}
	return 0;
}


int vonkarman_stream_init(struct aylp_device *self)
{
	self->process = &vonkarman_stream_process;
	self->close = &vonkarman_stream_close;
	self->device_data = (struct aylp_vonkarman_stream_data *)calloc(
		1, sizeof(struct aylp_vonkarman_stream_data)
	);
	struct aylp_vonkarman_stream_data *data = self->device_data;

	// some default params
	data->win_width = 10;
	data->win_height = 10;

	// parse the params json into our data struct
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "L0")) {
			data->L0 = strtod(json_object_get_string(val), 0);
			log_trace("L0 = %E", data->L0);
		} else if (!strcmp(key, "r0")) {
			data->r0 = strtod(json_object_get_string(val), 0);
			log_trace("r0 = %E", data->r0);
		} else if (!strcmp(key, "pitch")) {
			data->pitch = strtod(json_object_get_string(val), 0);
			log_trace("pitch = %E", data->pitch);
		} else if (!strcmp(key, "screen_size")) {
			data->screen_size= (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("screen_size = %u", data->screen_size);
		} else if (!strcmp(key, "start_y")) {
			data->cur_y= (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("start_y = %u", data->cur_y);
		} else if (!strcmp(key, "win_height")) {
			data->win_height = (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("win_height = %u", data->win_height);
		} else if (!strcmp(key, "win_width")) {
			data->win_height = (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("win_width = %u", data->win_width);
		} else if (!strcmp(key, "start_x")) {
			data->cur_x = (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("start_x = %u", data->cur_x);
		} else if (!strcmp(key, "step_y")) {
			// no strtoi in stdlib and atoi is insecure
			data->cur_step_y = (int)strtod(
				json_object_get_string(val), 0
			);
			// that'll do ;)
			log_trace("step_y = %u", data->cur_step_y);
		} else if (!strcmp(key, "step_x")) {
			data->cur_step_x = (int)strtod(
				json_object_get_string(val), 0
			);
			log_trace("step_x = %u", data->cur_step_x);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}

	// make sure we didn't miss any params
	if (!data->L0 || !data->r0 || !data->pitch || !data->screen_size) {
		log_error("You must provide the following nonzero params: L0, "
			"r0, pitch, screen_size."
		);
		return -1;
	}

	// check for edge conditions
	if (data->win_width > data->screen_size
	|| data->win_height > data->screen_size) {
		log_error("Window width/height greater than screen size; "
			"falling back to 1,1"
		);
		data->win_width = 1; data->win_height = 1;
	}
	if (abs(data->cur_step_x) > data->screen_size - data->win_width
	|| abs(data->cur_step_y) > data->screen_size - data->win_height) {
		log_error("Step size > size - width or size - height; "
			"falling back to 0,0"
		);
		data->cur_step_x = 0; data->cur_step_y = 0;
	}
	if (data->cur_x > data->screen_size - data->win_width
	|| data->cur_y > data->screen_size - data->win_height) {
		log_error("Start position > size - width or size - height; "
			"falling back to 0,0"
		);
		data->cur_x = 0; data->cur_y = 0;
	}

	// set up the rng
	data->rng = gsl_rng_alloc(gsl_rng_ranlxs2);
	gsl_rng_set(data->rng, time(0));

	// make the phase screen
	if (_generate_phase_screen(self)) {
		log_error("Failed to generate phase screen.");
		return -1;
	}

	// set types
	self->type_in = AYLP_T_ANY | AYLP_U_ANY;
	self->type_out = AYLP_T_BLOCK | AYLP_U_RAD;	// TODO: matrix?

	return 0;
}


int vonkarman_stream_process(struct aylp_device *self, struct aylp_state *state)
{
	// move along the screen, not calculating wind speed, just relying on
	// any delay devices to set the speed
	struct aylp_vonkarman_stream_data *data = self->device_data;
	// if we're at the edge, flip direction if needed
	size_t max_x = data->phase_screen->size1 - data->win_width
		- data->cur_step_x;
	size_t max_y = data->phase_screen->size2 - data->win_height
		- data->cur_step_y;
	if ((data->cur_y >= max_y && data->cur_step_y > 0)
	|| (data->cur_y <= 0 && data->cur_step_y < 0)) {
		data->cur_step_y *= -1;
	}
	if ((data->cur_x >= max_x && data->cur_step_x > 0)
	|| (data->cur_x <= 0 && data->cur_step_x < 0)) {
		data->cur_step_x *= -1;
	}
	data->cur_y += data->cur_step_y;
	data->cur_x += data->cur_step_x;
	log_trace("Window at y,x indices %lu,%lu", data->cur_y, data->cur_x);
	gsl_matrix_view sub_view = gsl_matrix_submatrix(
		data->phase_screen,
		data->cur_y, data->cur_x,
		data->win_height, data->win_width
	);
	// it's possible that there's a faster way of doing this than the memcpy
	// that mat2blk requires---we'd need to somehow present a block view of
	// the submatrix, and make sure that downstream devices are compatible
	// with this kind of view. TODO: look into this.
	state->block = mat2blk(&sub_view.matrix);
	// housekeeping on the info struct
	state->header.type = AYLP_T_BLOCK | AYLP_U_RAD;
	state->header.log_dim.y = data->win_height;
	state->header.log_dim.x = data->win_width;
	state->header.pitch.y = data->pitch;
	state->header.pitch.x = data->pitch;
	return 0;
}


int vonkarman_stream_close(struct aylp_device *self)
{
	json_object_object_foreach(self->params, key, _) {
		json_object_object_del(self->params, key);
	}
	free(self->params); self->params = 0;
	struct aylp_vonkarman_stream_data *data = self->device_data;
	gsl_matrix_free(data->phase_screen); data->phase_screen = 0;
	free(data); self->device_data = 0;
	return 0;
}

