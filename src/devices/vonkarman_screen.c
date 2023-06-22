#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <json-c/json.h>
#include "openao.h"
#include "logging.h"
#include "vonkarman_screen.h"


/* von Kármán spectrum
 * see: https://doi.org/10.1364/AO.43.004527
 * and https://doi.org/10.1117/12.279029
 * L0 is outer scale length [m]
 * r0 is Fried parameter [m]
 * Gx and Gy are width and height of phase screen [m]
 * x and y are dimensionless (indices)
 * output is dimensionless (1/px)
 * Ivy's original MATLAB code:
 * karmanSpec = @(L0,r0,Gx,Gy,x,y) (0.15132*(Gx*Gy)^(-1/2)*r0^(-5/6) ...
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


/* generate the von Kármán phase screen by first weighting the spectrum with
 * Hermitian unit gaussian noise, then taking the inverse Fourier transform of
 * that 2D spectrum (again, see https://doi.org/10.1364/AO.43.004527)
 */
void _generate_phase_screen(struct oao_device *self)
{
	struct oao_vonkarman_screen_data *data = self->device_data;
	data->phase_screen = gsl_matrix_alloc(data->width, data->width);
	/* we fill a matrix with random phases. kind of. we don't need the
	 * phases to be complex, because the ifft later will make them
	 * conjugate-symmetric. however, we do need to make sure their rms is
	 * only 1/sqrt(2), rather than 1, because the symmetrization adds an
	 * imaginary part.
	 */
	for (size_t idx=0; idx<data->phase_screen->size1; idx++) {
		for (size_t jdx=0; jdx<data->phase_screen->size2; jdx++) {
			gsl_matrix_set(data->phase_screen, idx, jdx,
				gsl_ran_gaussian(data->rng, 1.0/sqrt(2))
			);
		}
	}
	// TODO: multiply by spectrum
	// TODO: ifft
}


int vonkarman_screen_init(struct oao_device *self)
{
	self->process = &vonkarman_screen_process;
	self->close = &vonkarman_screen_close;
	self->device_data = (struct oao_vonkarman_screen_data *)calloc(
		1, sizeof(struct oao_vonkarman_screen_data)
	); 
	struct oao_vonkarman_screen_data *data = self->device_data;
	// parse the params json into our data struct
	json_object_object_foreach(self->params, key, val) {
		if (!strcmp(key, "Cn2")) {
			data->Cn2 = strtod(json_object_get_string(val), 0);
			log_trace("Cn2 = %E", data->Cn2);
		} else if (!strcmp(key, "L")) {
			data->L= strtod(json_object_get_string(val), 0);
			log_trace("L = %E", data->L);
		} else if (!strcmp(key, "pitch")) {
			data->pitch = strtod(json_object_get_string(val), 0);
			log_trace("pitch = %E", data->pitch);
		} else if (!strcmp(key, "width")) {
			data->width = (size_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("width = %u", data->width);
		} else {
			log_warn("Unkown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->Cn2 || !data->L || !data->pitch || !data->width) {
		log_error("You must provide all params: Cn2, L, pitch, width.");
		return -1;
	}
	// set up the rng
	data->rng = gsl_rng_alloc(gsl_rng_ranlxs2);
	gsl_rng_set(data->rng, time(0));
	// make the phase screen
	_generate_phase_screen(self);
	log_trace("vonkarman_screen initialized");
	return 0;
}


int vonkarman_screen_process(struct oao_device *self, struct oao_state *state)
{
	log_trace("%E", _karman_spec(2.0,0.02,50.0,50.0,0,1));
	log_trace("vonkarman_screen processed");
	return 0;
}


int vonkarman_screen_close(struct oao_device *self)
{
	json_object_object_foreach(self->params, key, _) {
		json_object_object_del(self->params, key);
	}
	free(self->params); self->params = 0;
	struct oao_vonkarman_screen_data *data = self->device_data;
	gsl_matrix_free(data->phase_screen); data->phase_screen = 0;
	free(data); self->device_data = 0;
	log_trace("vonkarman_screen closed");
	return 0;
}

