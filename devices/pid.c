#include <time.h>

#include "anyloop.h"
#include "logging.h"
#include "xalloc.h"
#include "pid.h"


int pid_init(struct aylp_device *self)
{
	int err;
	self->proc = &pid_proc;
	self->fini = &pid_fini;
	self->device_data = xcalloc(1, sizeof(struct aylp_pid_data));
	struct aylp_pid_data *data = self->device_data;

	// set defaults
	data->p = 1.0;
	data->i = 0.0;
	data->d = 0.0;
	data->clamp = 1.0;

	// parse parameters
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "type")) {
			const char *s = json_object_get_string(val);
			if (!strcmp(s, "vector"))
				data->type = AYLP_T_VECTOR;
			else if (!strcmp(s, "matrix"))
				data->type = AYLP_T_MATRIX;
			else log_error("Unrecognized type: %s", s);
			log_trace("type = %s (0x%hhX)", s, data->type);
		} else if (!strcmp(key, "units")) {
			const char *s = json_object_get_string(val);
			data->units = aylp_units_from_string(s);
			log_trace("units = %s (0x%hhX)", s, data->units);
		} else if (!strcmp(key, "p")) {
			data->p = json_object_get_double(val);
			log_trace("p = %G", data->p);
		} else if (!strcmp(key, "i")) {
			data->i = json_object_get_double(val);
			log_trace("i = %G", data->i);
		} else if (!strcmp(key, "d")) {
			data->d = json_object_get_double(val);
			log_trace("d = %G", data->d);
		} else if (!strcmp(key, "clamp")) {
			data->clamp = json_object_get_double(val);
			if (data->clamp < 0)
				data->clamp = -data->clamp;
			log_trace("clamp = ±%G", data->clamp);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->type) {
		log_error("You must provide valid type param.");
		return -1;
	}

	// allocate dummy vectors or matrices so we can skip checking if they
	// exist in proc()
	switch (data->type) {
	case AYLP_T_VECTOR:
		data->acc_v = xmalloc_type(gsl_vector, 0);
		data->pre_v = xmalloc_type(gsl_vector, 0);
		data->res_v = xmalloc_type(gsl_vector, 0);
		break;
	case AYLP_T_MATRIX:
		data->acc_m = xmalloc_type(gsl_matrix, 0, 0);
		data->pre_m = xmalloc_type(gsl_matrix, 0, 0);
		data->res_m = xmalloc_type(gsl_matrix, 0, 0);
		break;
	}

	err = clock_gettime(CLOCK_MONOTONIC, &data->tp);
	if (err) {
		log_error("Couldn't get time: %s", strerror(err));
		return -1;
	}

	// set types and units
	self->type_in = data->type;
	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_UNCHANGED;
	self->units_out = data->units;
	return 0;
}


int pid_proc(struct aylp_device *self, struct aylp_state *state)
{
	int err;
	struct aylp_pid_data *data = self->device_data;
	struct timespec tp1;
	err = clock_gettime(CLOCK_MONOTONIC, &tp1);
	if (err) {
		log_error("Couldn't get time: %s", strerror(err));
		return -1;
	}
	double dt = tp1.tv_sec - data->tp.tv_sec;
	dt += 1E-9 * (tp1.tv_nsec - data->tp.tv_nsec);
	data->tp = tp1;
	log_trace("dt = %G s", dt);

	switch (data->type) {
	case AYLP_T_VECTOR: {
		gsl_vector *a = data->acc_v;
		gsl_vector *p = data->pre_v;
		gsl_vector *r = data->res_v;
		gsl_vector *s = state->vector;
		// check if we need to (re)initialize
		if (a->size != s->size) {
			xfree_type(gsl_vector, data->acc_v);
			data->acc_v = xcalloc_type(gsl_vector, s->size);
		}
		if (p->size != s->size) {
			xfree_type(gsl_vector, data->pre_v);
			data->pre_v = xcalloc_type(gsl_vector, s->size);
		}
		if (r->size != s->size) {
			xfree_type(gsl_vector, data->res_v);
			data->res_v = xmalloc_type(gsl_vector, s->size);
		}
		// loop over elements and apply PID control
		for (size_t j = 0; j < s->size; j++) {
			// update accumulator and clamp if needed
			a->data[j*a->stride] += dt * s->data[j*s->stride];
			if (a->data[j*a->stride] > data->clamp)
				a->data[j*a->stride] = data->clamp;
			else if (a->data[j*a->stride] < -data->clamp)
				a->data[j*a->stride] = -data->clamp;
			// apply pid params to result
			r->data[j*r->stride] =
				- data->p * s->data[j*s->stride]
				- data->i * a->data[j*a->stride]
				- data->d * (
					s->data[j*s->stride]
					- p->data[j*p->stride]
				) / dt
			;
			// clamp result if needed
			if (r->data[j*r->stride] > data->clamp)
				r->data[j*r->stride] = data->clamp;
			else if (r->data[j*r->stride] < -data->clamp)
				r->data[j*r->stride] = -data->clamp;
			// update previous
			p->data[j*p->stride] = s->data[j*s->stride];
		}
		state->vector = data->res_v;
		break;
	}

	case AYLP_T_MATRIX: {
		gsl_matrix *a = data->acc_m;
		gsl_matrix *p = data->pre_m;
		gsl_matrix *r = data->res_m;
		gsl_matrix *s = state->matrix;
		// check if we need to (re)initialize
		if (UNLIKELY(a->size1 != s->size1 || a->size2 != s->size2)) {
			xfree_type(gsl_matrix, data->acc_m);
			data->acc_m = xcalloc_type(gsl_matrix,
				s->size1, s->size2
			);
		}
		if (UNLIKELY(p->size1 != s->size1 || p->size2 != s->size2)) {
			xfree_type(gsl_matrix, data->pre_m);
			data->pre_m = xcalloc_type(gsl_matrix,
				s->size1, s->size2
			);
		}
		if (UNLIKELY(r->size1 != s->size1 || r->size2 != s->size2)) {
			xfree_type(gsl_matrix, data->res_m);
			data->res_m = xmalloc_type(gsl_matrix,
				s->size1, s->size2
			);
		}
		// loop over elements and apply PID control
		for (size_t y = 0; y < s->size1; y++) {
			for (size_t x = 0; x < s->size2; x++) {
				// update accumulator and clamp if needed
				a->data[y*a->tda+x] += dt * s->data[y*s->tda+x];
				if (a->data[y*a->tda+x] > data->clamp)
					a->data[y*a->tda+x] = data->clamp;
				else if (a->data[y*a->tda+x] < -data->clamp)
					a->data[y*a->tda+x] = -data->clamp;
				// apply pid params to result
				r->data[y*r->tda+x] =
					- data->p * s->data[y*s->tda+x]
					- data->i * a->data[y*a->tda+x]
					- data->d * (
						s->data[y*s->tda+x]
						- p->data[y*p->tda+x]
					) / dt
				;
				// clamp result if needed
				if (r->data[y*r->tda+x] > data->clamp)
					r->data[y*r->tda+x] = data->clamp;
				if (r->data[y*r->tda+x] < -data->clamp)
					r->data[y*r->tda+x] = -data->clamp;
				// update previous
				p->data[y*p->tda+x] = s->data[y*s->tda+x];
			}
		}
		state->matrix = data->res_m;
		break;
	}
	}

	return 0;
}


int pid_fini(struct aylp_device *self)
{
	struct aylp_pid_data *data = self->device_data;
	switch (data->type) {
	case AYLP_T_VECTOR:
		xfree_type(gsl_vector, data->acc_v);
		xfree_type(gsl_vector, data->pre_v);
		xfree_type(gsl_vector, data->res_v);
		break;
	case AYLP_T_MATRIX:
		xfree_type(gsl_matrix, data->acc_m);
		xfree_type(gsl_matrix, data->pre_m);
		xfree_type(gsl_matrix, data->res_m);
		break;
	}
	xfree(data);
	return 0;
}

