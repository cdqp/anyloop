#ifndef AYLP_ANYLOOP_H_
#define AYLP_ANYLOOP_H_

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>

// Stop clang from complaining about json_object_object_foreach()
#ifdef __clang__
	#pragma clang diagnostic ignored \
		"-Wgnu-statement-expression-from-macro-expansion"
#endif

// C23 deprecated _Static_assert in favor of static_assert
#ifndef static_assert
#define static_assert _Static_assert
#endif

// So as to not shoot ourselves in the foot when compiling for different
// platforms, we need to specify that certain types are the right size.
#if __STDC_VERSION__ > 201710L
	#ifndef __STDC_IEC_60559_BFP__
	static_assert(0, "we require IEEE-754 binary floats");
	#endif
#else
	#ifndef __STDC_IEC_559__
	static_assert(0, "we require iec559 floats");
	#endif
#endif
static_assert(sizeof(unsigned char) == 1,
	"we need unsigned char to be 8-bit for gsl");
static_assert(sizeof(unsigned short) == 2,
	"we need unsigned short to be 16-bit for gsl");
static_assert(sizeof(size_t) == 8, "we need size_t to be 64-bit for gsl");

#define UNUSED(x)	(void)(x)
#define LIKELY(x)	__builtin_expect((x),1)
#define UNLIKELY(x)	__builtin_expect((x),0)


/** Flags pertaining to loop status. */
typedef uint8_t aylp_status;
enum {
	/** Signals that we are done with the loop. */
	AYLP_DONE	= 1 << 0,
	// add more as necessary
};


/** Enum for type of data in aylp_state.
 * After being initialized, devices must specify the types they are capable of
 * inputting and the types they may output. Null is a special type that means
 * "unaltered" when set as a device output.
 */
typedef uint8_t aylp_type;
enum {
	/** As device output, indicates that type is unchanged from before. */
	AYLP_T_UNCHANGED	= 0,
	/** Indicates that there is no data in the pipeline yet. */
	AYLP_T_NONE		= 1 << 0,
	/** For gsl_block. */
	AYLP_T_BLOCK		= 1 << 1,
	/** For gsl_vector. */
	AYLP_T_VECTOR		= 1 << 2,
	/** For gsl_matrix. */
	AYLP_T_MATRIX		= 1 << 3,
	/** For gsl_block_uchar. */
	AYLP_T_BLOCK_UCHAR	= 1 << 4,
	/** For gsl_matrix_uchar. */
	AYLP_T_MATRIX_UCHAR	= 1 << 5,
	/** For gsl_matrix_ushort. */
	AYLP_T_MATRIX_USHORT	= 1 << 6,
	/** Used to signal compatibility with any (gsl) type.
	* Devices must also set AYLP_U_ANY to be compatible with any aylp_type.
	*/
	AYLP_T_ANY	= 0xFF,
	// add more as necessary
};
// https://en.wikipedia.org/wiki/X_macro
#define FOR_AYLP_TYPES(DO) \
	DO(AYLP_T_UNCHANGED, unchanged) \
	DO(AYLP_T_NONE, none) \
	DO(AYLP_T_BLOCK, block) \
	DO(AYLP_T_VECTOR, vector) \
	DO(AYLP_T_MATRIX, matrix) \
	DO(AYLP_T_BLOCK_UCHAR, block_uchar) \
	DO(AYLP_T_MATRIX_UCHAR, matrix_uchar) \
	DO(AYLP_T_ANY, any)


/** Enum for units of data in aylp_state.
 * After being initialized, devices must specify the units they are capable of
 * inputting and the units they may output.
 */
typedef uint8_t aylp_units;
enum {
	/** As device output, indicates that units are unchanged from before. */
	AYLP_U_UNCHANGED	= 0,
	/** Indicates that there is no data in the pipeline yet. */
	AYLP_U_NONE		= 1 << 0,
	/** Signals that the units are natural numbers.
	* For example, pixel values. */
	AYLP_U_COUNTS		= 1 << 1,
	/** Signals that the units are in [-1,+1]. */
	AYLP_U_MINMAX		= 1 << 2,
	/** Signals that the units are in radians. */
	AYLP_U_RAD		= 1 << 3,
	/** Signals that the units are in volts. */
	AYLP_U_V		= 1 << 4,
	/** Used to signal compatibility with any units. */
	AYLP_U_ANY		= 0xFF,
	// add more as necessary
};
#define FOR_AYLP_UNITS(DO) \
	DO(AYLP_U_UNCHANGED, unchanged) \
	DO(AYLP_U_NONE, none) \
	DO(AYLP_U_COUNTS, counts) \
	DO(AYLP_U_MINMAX, minmax) \
	DO(AYLP_U_RAD, rad) \
	DO(AYLP_U_V, V) \
	DO(AYLP_U_ANY, any)


/** Little-endian representation of "AYLP" as a magic number. */
#define AYLP_MAGIC 0x504C5941

/** Schema version (null means unstable). */
#define AYLP_SCHEMA_VERSION 0

/**
 * Status of loop and header for saved data files.
 * Saved data files will use this as a header (see file_sink_proc()), and it's
 * also good to have the rest of this info during the loop, so devices and get
 * and set the status of the overall system.
 */
struct aylp_header {
	/** Magic number to verify from another end (for endianness, etc.) */
	uint32_t magic;

	/** Schema version, status flags, and type of data. */
	uint8_t version;
	aylp_status status;	// uint8_t
	aylp_type type;		// uint8_t
	aylp_units units;	// uint8_t

	/** Logical dimensions (like the size of a matrix).
	* For example, commands to a DM might usually be seen as vectors, but
	* in reality have some logical x,y dimensions, which can be important.
	*/
	struct __attribute__((packed)) {
		uint64_t y;
		uint64_t x;
	} log_dim;

	/** Physical distance between successive logical rows and columns.
	* (In units of meters.)
	*/
	struct __attribute__((packed)) {
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
		gsl_block_uchar *block_uchar;
		gsl_matrix_uchar *matrix_uchar;
	};
};


/** Device struct.
 * How this is interpreted is up to the specific device. Devices are expected to
 * attach their proc() and fini() functions upon initialization, if such
 * functions are needed.
 */
struct aylp_device {
	/** URI(-like) name for device.
	* Most devices will start with the "anyloop" scheme. */
	char *uri;

	/** Input and output types/units accepted and produced by the device.
	* For each device, its input type and units must contain all the bits
	* set in the last non-null output type before it in the pipeline. That
	* is, a device is compatible with the pipeline if it is capable of
	* accepting all the possible output types and units before it in the
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

	/** Initializer function.
	* Devices are initialized in pipeline order at program startup. */
	int (*init)(struct aylp_device *self);

	/** Processing function.
	* Called once every loop when it's the device's turn. */
	int (*proc)(struct aylp_device *self, struct aylp_state *state);

	/** Destructor function.
	* Devices are closed in pipeline order when the program exits. */
	int (*fini)(struct aylp_device *self);

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


/** Convert a string to a pure type. Case insensitive.
 * @param type_name: the string holding the type name (e.g. "vector").
 */
aylp_type aylp_type_from_string(const char *type_name);


/** Convert a pure type to its name as a string. Returns lowercase.
 * @param type: the type to convert (e.g. AYLP_T_VECTOR).
 */
const char *aylp_type_to_string(aylp_type type);


/** Convert a string to the pure units it represents. Case insensitive.
 * @param units_name: the string holding the units (e.g. "rad").
 */
aylp_units aylp_units_from_string(const char *units_name);


/** Convert pure units to their name as a string. Mixed case (e.g. rad, V).
 * @param units: the units to convert (e.g. AYLP_U_RAD).
 */
const char *aylp_units_to_string(aylp_units units);


#endif	// include guard

