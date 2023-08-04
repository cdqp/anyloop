#include <pthread.h>
#include <string.h>

#include "anyloop.h"
#include "logging.h"
#include "thread_pool.h"
#include "center_of_mass.h"
#include "xalloc.h"


// TODO: optimize? for example, if the matrix is contiguous and 8x8 in size, the
// compiler can do some really nice optimizations here. but for variable sizes,
// and non-contiguous matrices ... it gets a little harder.
// With -O3, on my old laptop, I timed that this could do 10 million 8x8
// matrices in 4.302 s, whereas the naive solution with contiguous memory took
// 2.447 s. So even with this naive non-contiguous solution, we still get at
// worst 0.4 μs per matrix, which is ... not the worst. That's 43 μs for a 10x10
// grid of submatrices, with no multithreading. GCC is quite intelligent.
/** Center of mass task for gsl_matrix_uchar.
* Will write (y,x) coords (in that order) of the center of mass of the uchar
* matrix at src to dst[0] and dst[1]. */
void com_mat_uchar(gsl_matrix_uchar *src, double *dst)
{
	double y = 0.0, x = 0.0, s = 0.0;
	// take weighted average
	for (size_t i=0; i < src->size1; i++) {
		for (size_t j=0; j < src->size2; j++) {
			unsigned char el = src->data[i*src->tda + j];
			y += i*el;
			x += j*el;
			s += el;
		}
	}
	// set final values, scaling from 0:size-1 (given by y/s or x/s) to -1:1
	dst[0] = -1.0 + 2*y/(s*(src->size1-1));
	dst[1] = -1.0 + 2*x/(s*(src->size2-1));
}


// This is nice and fast, but doesn't work for us, because we want to find the
// com of a slice of non-contiguous memory....
// /** Hardcoded 8x8 center of mass task so the compiler can optimize it.
// * Will look at 64 uchars at src, and write the output as two doubles in dst
// * in (y,x) order. */
// void com_8x8(unsigned char *src, double *dst)
// {
// 	double y = 0.0, x = 0.0, s = 0.0;
// 	for (char i = 0; i < 8; i++) {
// 		for (char j = 0; j < 8; j++) {
// 			y += i * src->data[8*i+j];
// 			x += j * src->data[8*i+j];
// 			s += src->data[8*i+j];
// 		}
// 	}
// 	dst[0] = y / s;
// 	dst[1] = x / s;
// }


int center_of_mass_init(struct aylp_device *self)
{
	int err;
	self->process = &center_of_mass_process;
	self->close = &center_of_mass_close;
	self->device_data = xcalloc(1, sizeof(struct aylp_center_of_mass_data));
	struct aylp_center_of_mass_data *data = self->device_data;

	// default params
	data->region_height = 0;
	data->region_width = 0;
	data->thread_count = 1;
	// parse parameters
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "region_height")) {
			data->region_height = json_object_get_uint64(val);
			log_trace("region_height = %llu", data->region_height);
		} else if (!strcmp(key, "region_width")) {
			data->region_width = json_object_get_uint64(val);
			log_trace("region_width = %llu", data->region_width);
		} else if (!strcmp(key, "thread_count")) {
			data->thread_count = json_object_get_uint64(val);
			if (data->thread_count == 0) {
				log_error("Correcting 0 threads to 1 thread");
				data->thread_count = 1;
			}
			log_trace("thread_count = %llu", data->thread_count);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	if (!data->region_height || !data->region_width) {
		log_error("You must provide nonzero region_height and "
			"region_width params"
		);
		return -1;
	}

	// start threads
	data->threads = xmalloc(data->thread_count * sizeof(pthread_t));
	for (size_t t = 0; t < data->thread_count; t++) {
		err = pthread_create(&data->threads[t],
			0, task_runner, &data->queue
		);
		if (err) {
			log_error("Couldn't create pthread: %s", strerror(err));
			return -1;
		}
	}
	log_info("Started %llu threads", data->thread_count);

	// set types and units
	self->type_in = AYLP_T_MATRIX_UCHAR;
	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_VECTOR;
	self->units_out = AYLP_U_MINMAX;
	return 0;
}


int center_of_mass_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_center_of_mass_data *data = self->device_data;
	size_t max_y = state->matrix_uchar->size1;
	size_t max_x = state->matrix_uchar->size2;
	size_t y_subap_count = max_y / data->region_height;
	size_t x_subap_count = max_x / data->region_width;
	// TODO: we assume one task per subaperture, but this might not be ideal
	// for all subaperture counts and sizes. It may be that the overhead
	// associated with processing one task can be comparable to the cost of
	// calculating on one subaperture, and we should be creating tasks that
	// process more than one subaperture at a time.
	size_t n_tasks = y_subap_count * x_subap_count;
	// It's unfortunately quite ugly that we malloc here, but it's the
	// simplest fast solution I can think of to the issue of not knowing the
	// size of state->matrix_uchar when init() is run. Remember that `data`
	// is `calloc`ed so we are guaranteed to malloc on first process.
	if (data->n_tasks < n_tasks) {
		// we *could* realloc instead of this free/malloc combo, but I
		// expect free/malloc to be faster since we don't care about
		// keeping the old memory (https://stackoverflow.com/a/39562813)
		xfree(data->tasks);
		data->tasks = xmalloc(n_tasks * sizeof(struct aylp_task));
		data->n_tasks = n_tasks;
		// allocate the matrices we use for sources too
		data->subaps = xmalloc(n_tasks * sizeof(gsl_matrix_uchar));
	}
	// allocate the com vector if needed
	if (!data->com || data->com->size < n_tasks*2) {
		xfree_type(gsl_vector, data->com);
		data->com = xmalloc_type(gsl_vector, n_tasks*2);
	}

	// start assigning tasks
	size_t t = 0;
	for (size_t i=0; i < y_subap_count; i++) {
		for (size_t j=0; j < x_subap_count; j++) {
			// set source data
			data->subaps[t] = gsl_matrix_uchar_submatrix(
				state->matrix_uchar,
				i * data->region_height,
				j * data->region_width,
				data->region_height,
				data->region_width
			).matrix;
			// set task
			data->tasks[t] = (struct aylp_task){
				.func = (void(*)(void*,void*))com_mat_uchar,
				.src = &data->subaps[t],
				.dst = (void *)(data->com->data+2*t),
				.next_task = 0
			};
			task_enqueue(&data->queue, &data->tasks[t]);
			t += 1;
		}
	}
	// wait for threads to finish
	while (data->queue.tasks_processing) {
		sched_yield();	// (is there a better function to call?)
	}
	// zero-copy update of pipeline state
	state->vector = data->com;
	// housekeeping on the header
	state->header.type = self->type_out;
	state->header.units = self->units_out;
	state->header.log_dim.y = data->com->size;
	state->header.log_dim.x = 1;
	return 0;
}


int center_of_mass_close(struct aylp_device *self)
{
	struct aylp_center_of_mass_data *data = self->device_data;
	shut_queue(&data->queue);
	for (size_t t = 0; t < data->thread_count; t++) {
		pthread_join(data->threads[t], 0);
	}
	xfree(data->threads);
	xfree(data->tasks);
	xfree(data);
	return 0;
}

