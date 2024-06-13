#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "matmul.h"
#include "pretty.h"
#include "xalloc.h"


int matmul_init(struct aylp_device *self)
{
	int err;
	self->device_data = xcalloc(1, sizeof(struct aylp_matmul_data));
	struct aylp_matmul_data *data = self->device_data;

	// input filename
	const char *filename = 0;
	// so we can check if we got a type
	self->type_in = AYLP_T_NONE;

	// parse the params json into our data struct
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "filename")) {
			filename = json_object_get_string(val);
			log_trace("filename = %s", filename);
		} else if (!strcmp(key, "type")) {
			const char *s = json_object_get_string(val);
			if (!strcmp(s, "vector"))
				self->type_in = AYLP_T_VECTOR;
			else if (!strcmp(s, "matrix"))
				self->type_in = AYLP_T_MATRIX;
			else log_error("Unrecognized type: %s", s);
			log_trace("type = %s (0x%X)", s, self->type_in);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}

	// make sure we didn't miss any params
	if (!filename || self->type_in == AYLP_T_NONE) {
		log_error("You must provide the filename and type params.");
		return -1;
	}

	// open input file
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		log_error("Couldn't open file: %s", strerror(errno));
		return -1;
	}
	// grab header
	struct aylp_header head = {0};
	size_t n = fread(&head, 1, sizeof(struct aylp_header), fp);
	if (n < sizeof(struct aylp_header)) {
		log_error("Short read: %llu of %llu",
			n, sizeof(struct aylp_header)
		);
		return -1;
	}
	// check header
	if (head.magic != AYLP_MAGIC) {
		log_error("File provided is not an AYLP file.");
		return -1;
	} else if (head.version != AYLP_SCHEMA_VERSION) {
		log_error("File provided has different AYLP_SCHEMA_VERSION "
			"(we are %hhX, file is %hhX)",
			AYLP_SCHEMA_VERSION, head.version
		);
		return -1;
	} else if (head.type != AYLP_T_MATRIX) {
		log_error("Data in file is not of type AYLP_T_MATRIX.");
		return -1;
	}
	// allocate and grab data
	data->mat = gsl_matrix_alloc(head.log_dim.y, head.log_dim.x);
	err = gsl_matrix_fread(fp, data->mat);
	if (err) {
		log_error("Error in reading matrix: ", gsl_strerror(err));
		return -1;
	}
	// close input file
	fclose(fp);

	if (log_get_level() <= LOG_INFO) {
		// INFO: log matrix contents
		log_info("Read matrix of size %llux%llu:",
			data->mat->size1, data->mat->size2
		);
		pretty_matrix(data->mat);
	}

	switch (self->type_in) {
	case AYLP_T_MATRIX:
		self->process = &matmul_process_mm;
		break;
	case AYLP_T_VECTOR:
		self->process = &matmul_process_mv;
		break;
	default:
		log_error("BUG: self->type_in is wrong");
		return -1;
	}

	// set types and units
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;

	return 0;
}


// TODO: test this
int matmul_process_mm(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_matmul_data *data = self->device_data;

	if (UNLIKELY(!data->mat_res)) {
		// we have nowhere to put the result; let's allocate it
		data->mat_res = gsl_matrix_alloc(
			data->mat->size1, state->matrix->size2
		);
	} else if (UNLIKELY(data->mat_res->size2 != state->matrix->size2)) {
		// somehow the state matrix changed width >:(
		xfree_type(gsl_matrix, data->mat_res);
		data->mat_res = gsl_matrix_alloc(
			data->mat->size1, state->matrix->size2
		);
	}

	// C = αAB + βC
	int err = gsl_blas_dgemm(
		CblasNoTrans,	// no transpose
		CblasNoTrans,	// trans people are cool though
		1.0,		// α
		data->mat,	// A
		state->matrix,	// B
		0.0,		// β
		data->mat_res	// C
	);
	if (err) {
		log_error("Error during dgemm: %s", gsl_strerror(err));
		return -1;
	}

	// update pipeline state
	state->matrix = data->mat_res;
	// housekeeping on the header
	state->header.log_dim.y = data->mat_res->size1;
	state->header.log_dim.x = data->mat_res->size2;

	return 0;
}


int matmul_process_mv(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_matmul_data *data = self->device_data;

	if (UNLIKELY(!data->vec_res)) {
		// we have nowhere to put the result; let's allocate it
		data->vec_res = gsl_vector_alloc(data->mat->size1);
	}

	// y = αAx + βy
	int err = gsl_blas_dgemv(
		CblasNoTrans,	// no transpose
		1.0,		// α
		data->mat,	// A
		state->vector,	// x
		0.0,		// β
		data->vec_res	// y
	);
	if (err) {
		log_error("Error during dgemv: %s", gsl_strerror(err));
		return -1;
	}

	// update pipeline state
	state->vector = data->vec_res;
	// housekeeping on the header
	state->header.log_dim.y = data->vec_res->size;
	state->header.log_dim.x = 1;

	return 0;
}


int matmul_close(struct aylp_device *self)
{
	struct aylp_matmul_data *data = self->device_data;
	xfree_type(gsl_matrix, data->mat);
	switch (self->type_in) {
	case AYLP_T_MATRIX:
		xfree_type(gsl_matrix, data->mat_res);
		break;
	case AYLP_T_VECTOR:
		xfree_type(gsl_vector, data->vec_res);
		break;
	default:
		break;
	}
	xfree(data);
	return 0;
}

