#ifndef AYLP_ANYLOOP_H_
#define AYLP_ANYLOOP_H_

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>

#include "logging.h"

// So as to not shoot ourselves in the foot when compiling for different
// platforms, we need to specify that certain types are the right size.
#if __STDC_VERSION__ > 201710L
	#ifndef __STDC_IEC_60559_BFP__
	_Static_assert(0, "we require IEEE-754 binary floats");
	#endif
#else
	#ifndef __STDC_IEC_559__
	_Static_assert(0, "we require IEC559 floats");
	#endif
#endif
_Static_assert(sizeof(unsigned char) == 1,
	"we need unsigned char to be 8-bit for gsl");
_Static_assert(sizeof(size_t) == 8, "we need size_t to be 64-bit for gsl");

#define UNUSED(x)	(void)(x)
#define LIKELY(x)	__builtin_expect((x),1)
#define UNLIKELY(x)	__builtin_expect((x),0)


/** Flags pertaining to loop status. */
typedef uint16_t aylp_status;
enum {
	/** Signals that we are done with the loop. */
	AYLP_DONE	= 1 << 0,
	// add more as necessary
};


/** Enum for type of data in aylp_state.
 * Devices must specify their input and output types after they are initialized.
 * Null is a special type that means "unaltered" when set as a device output.
 */
typedef uint8_t aylp_type;
enum {
	/** Indicates that there is no data in the pipeline yet. */
	AYLP_T_NONE	= 1 << 0,
	/** For gsl_block. */
	AYLP_T_BLOCK	= 1 << 1,
	/** For gsl_vector. */
	AYLP_T_VECTOR	= 1 << 2,
	/** For gsl_matrix. */
	AYLP_T_MATRIX	= 1 << 3,
	/** For gsl_block_uchar. */
	AYLP_T_BYTES	= 1 << 4,
	/** Used to signal compatibility with any (gsl) type.
	* Devices must also set AYLP_U_ANY to be compatible with any aylp_type.
	*/
	AYLP_T_ANY	= 0xFF,
	// add more as necessary
};


/** Enum for type of data in aylp_state.
 * Devices must specify their input and output types after they are initialized.
 * Null is a special type that means "unaltered" when set as a device output.
 */
typedef uint8_t aylp_units;
enum {
	/** Indicates that there is no data in the pipeline yet. */
	AYLP_U_NONE	= 1 << 0,
	/** Signals that the units are in radians. */
	AYLP_U_RAD	= 1 << 1,
	/** Signals that the units are in [-1,+1]. */
	AYLP_U_MINMAX	= 1 << 2,
	/** Used to signal compatibility with any units. */
	AYLP_U_ANY	= 0xFF,
	// add more as necessary
};


/** Little-endian representation of "AYLP" as a magic number. */
#define AYLP_MAGIC 0x504C5941

/**
 * Status of loop and header for saved data files.
 * Saved data files will use this as a header (see file_sink_process()), and
 * it's also good to have the rest of this info during the loop, so devices and
 * get and set the status of the overall system.
 */
struct aylp_header {
	/** Magic number to verify from another end (for endianness, etc.) */
	uint32_t magic;

	/** Status flags and type of data. */
	aylp_status status;	// uint16_t
	aylp_type type;		// uint8_t
	aylp_units units;	// uint8_t

	/** Logical dimensions (like the size of a matrix).
	* For example, commands to a DM might usually be seen as vectors, but
	* in reality have some logical x,y dimensions, which can be important.
	*/
	struct {
		uint64_t y;
		uint64_t x;
	} log_dim;

	/** Physical distance between successive logical rows and columns. */
	struct {
		double y;
		double x;
	} pitch;
}__attribute__((packed));


/** State of the system.
 * This is to be written to or read from by devices, and contains all the
 * important data that needs to be passed from one device to another in the
 * pipeline.
 */
struct aylp_state {
	/** Header */
	struct aylp_header header;

	/** Pointer to data in the pipeline.
	* The type should correspond to the current value of header.type.
	* Typechecking will be done on initialization. For example, a wavefront
	* sensor might write a vector of phases to this, and a deformable
	* mirror might read a vector command from it. The (undocumented)
	* alloc_from_block methods in gsl are undoubtedly useful for
	* conversions here:
	* https://git.savannah.gnu.org/cgit/gsl.git/tree/matrix/init_source.c
	*/
	union {
		gsl_block *block;
		gsl_vector *vector;
		gsl_matrix *matrix;
		gsl_block_uchar *bytes;
	};
};


/** Device struct.
 * How this is interpreted is up to the specific device. Devices are expected to
 * attach their process() and close() functions upon initialization, if such
 * functions are needed.
 */
struct aylp_device {
	/** URI(-like) name for device.
	* Most devices will start with the "anyloop" scheme. */
	char *uri;

	/** Input and output types/units accepted and produced by the device.
	* Each device's input type and units must independently logical-and to a
	* nonzero value with the last non-null output type before it in the
	* pipeline. If a device comes first in the pipeline, and it has an input
	* type or units other than AYLP_T_ANY or AYLP_U_ANY, it must be
	* compatible with the output type and units of the last device in the
	* pipeline as well as AYLP_T_NONE|AYLP_U_NONE.
	*/
	aylp_type type_in;
	aylp_units units_in;
	aylp_type type_out;
	aylp_units units_out;

	/** Device parameters.
	* This is taken directly from the json configuration file. For example,
	* a valid parameter json object might look like
	* `"params":{"ip":"127.0.0.1","port":"64730"}`, and then the
	* `json_object` that `params` points to would look like
	* `{"ip":"127.0.0.1","port":"64730"}` by the time the device is
	* initialized.
	*/
	struct json_object *params;

	/** Pointer to log_status structure.
	* Built-in devices should ignore this. Plug-in devices should log_init
	* with it. */
	log_status_t *log_status;

	/** Initializer function.
	* Devices are initialized in pipeline order at program startup. */
	int (*init)(struct aylp_device *self);

	/** Processing function.
	* Called once every loop when it's the device's turn. */
	int (*process)(struct aylp_device *self, struct aylp_state *state);

	/** Destructor function.
	* Devices are closed in pipeline order when the program exits. */
	int (*close)(struct aylp_device *self);

	/** Optional pointer to allow devices to store private data. */
	void *device_data;
};


/** Main config struct.
 * Carries information on the configuration of the pipeline.
 */
struct aylp_conf {
	/** Number of devices. */
	size_t n_devices;

	/** Array of aylp_device objects. */
	struct aylp_device *devices;
};


#endif	// include guard

