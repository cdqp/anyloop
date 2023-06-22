#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <json-c/json.h>
#include "openao.h"
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
		/ powf(r0, 5.0/6.0)
		/ powf(gsl_hypot3(x/Gx, y/Gy, 1/L0), 11.0/6.0)
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
		log_error("backward fft failed, gsl_errno=%d\n", err);
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
int _generate_phase_screen(struct oao_device *self)
{
	struct oao_vonkarman_stream_data *data = self->device_data;
	// We assume we're making a square phase screen; this is generally a
	// good idea (see https://doi.org/10.1364/AO.37.004605), but this could
	// be changed here if one wishes.
	data->phase_screen = gsl_matrix_alloc(data->width, data->width);
	// We fill a matrix with Gaussian noise to act as the phasors by which
	// we weight our IFFT. We don't need the phasors to be complex, because
	// the IFFT later will make them conjugate-symmetric. However, we do
	// need to make sure their rms is only 1/sqrt(2), rather than 1, because
	// the symmetrization adds an imaginary part. Oh, and we multiply them
	// by the spectrum as we go.
	size_t M = data->phase_screen->size1;
	size_t N = data->phase_screen->size2;
	log_trace("Allocated %u by %u matrix for phase screen", M, N);
	for (int idx=0; idx<M; idx++) {
		for (int jdx=0; jdx<N; jdx++) {
			// von Kármán spectrum amplitude
			double amp = _karman_spec(
				data->L0, data->r0,
				data->width * data->pitch,
				data->width * data->pitch,
				// divide by two since we're calculating both
				// complex and real parts next to each other
				idx/2, jdx/2
			);
			gsl_matrix_set(data->phase_screen, idx, jdx,
				gsl_ran_gaussian(data->rng, 1.0/sqrt(2)) * amp
			);
		}
	}
	// set the 0,0 element to zero (it's nonphysical)
	gsl_matrix_set(data->phase_screen, 0, 0, 0.0);
	log_trace("Generated Fourier components");
	// perform a 2D backward-fft by doing a bunch of 1D backward ffts
	for (size_t idx=0; idx<M; idx++) {
		// backward fft each row
		_backward_fft(
			data->phase_screen->data + idx*N,
			1, N
		);
	}
	for (size_t jdx=0; jdx<N; jdx++) {
		// backward fft each column
		_backward_fft(
			data->phase_screen->data + jdx,
			data->phase_screen->tda, M
		);
	}
	if (LOG_DEBUG >= log_get_level()) {
		// DEBUG: write the matrix to a file
		log_debug("Writing screen to screen.bin");
		FILE *fp = fopen("screen.bin", "wb");
		gsl_matrix_fwrite(fp, data->phase_screen);
		fflush(fp);
		fclose(fp);
	}
	return 0;
}


int vonkarman_stream_init(struct oao_device *self)
{
	self->process = &vonkarman_stream_process;
	self->close = &vonkarman_stream_close;
	self->device_data = (struct oao_vonkarman_stream_data *)calloc(
		1, sizeof(struct oao_vonkarman_stream_data)
	);
	struct oao_vonkarman_stream_data *data = self->device_data;
	// parse the params json into our data struct
	json_object_object_foreach(self->params, key, val) {
		if (!strcmp(key, "L0")) {
			data->L0 = strtod(json_object_get_string(val), 0);
			log_trace("L0 = %E", data->L0);
		} else if (!strcmp(key, "r0")) {
			data->r0 = strtod(json_object_get_string(val), 0);
			log_trace("r0 = %E", data->r0);
		} else if (!strcmp(key, "pitch")) {
			data->pitch = strtod(json_object_get_string(val), 0);
			log_trace("pitch = %E", data->pitch);
		} else if (!strcmp(key, "width")) {
			data->width = (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("width = %u", data->width);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->L0 || !data->r0 || !data->pitch || !data->width) {
		log_error("You must provide all params: L0, r0, pitch, width.");
		return -1;
	}
	// set up the rng
	data->rng = gsl_rng_alloc(gsl_rng_ranlxs2);
	gsl_rng_set(data->rng, time(0));
	// make the phase screen
	if (_generate_phase_screen(self)) {
		log_error("Failed to generate phase screen.");
		return -1;
	}
	log_trace("vonkarman_stream initialized");
	return 0;
}


int vonkarman_stream_process(struct oao_device *self, struct oao_state *state)
{
	// TODO: move along the screen. probably don't calculate wind speed,
	// just rely on a delay device. maybe take an initial position and a
	// direction as additional params
	log_trace("vonkarman_stream processed");
	return 0;
}


int vonkarman_stream_close(struct oao_device *self)
{
	json_object_object_foreach(self->params, key, _) {
		json_object_object_del(self->params, key);
	}
	free(self->params); self->params = 0;
	struct oao_vonkarman_stream_data *data = self->device_data;
	gsl_matrix_free(data->phase_screen); data->phase_screen = 0;
	free(data); self->device_data = 0;
	log_trace("vonkarman_stream closed");
	return 0;
}
