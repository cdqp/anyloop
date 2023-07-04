#ifndef _AYLP_ANYLOOP_H
#define _AYLP_ANYLOOP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>

#define UNUSED(x)	(void)(x)
#define LIKELY(x)	__builtin_expect((x),1)
#define UNLIKELY(x)	__builtin_expect((x),0)

// require IEC559 floats
#ifndef __STDC_IEC_559__
_Static_assert(0, "we require IEC559 floats");
#endif

_Static_assert(sizeof(size_t) == 8, "we need size_t to be 64-bit for gsl");

// little-endian representation of "AYLPDATA" as a magic number
#define AYLPDATA_MAGIC 0x41544144504C5941

// flags for type of data written to block
// this is useful for a few reasons; for example, a DM might want to be able to
// take a command directly (after having gone through whatever processing device
// upstream), while also being able to just take raw phases (e.g. from a von
// Kármán stream) and scale them according to the stroke depth
#define AYLP_PHASES	1 << 0	// a screen of phases in radians
#define AYLP_COMMAND	1 << 1	// a command to send to a device (range ±1)
// add more as necessary

// flags that can be set to tell devices different things
#define AYLP_DONE	1 << 0		// done with loop
// add more as necessary

// saved data files will use this as a header (see file_sink_process())
// and it's also good to have the rest of this info for devices
struct aylp_header {
	// magic number to verify from the other end
	uint64_t magic;
	// type of data written to block
	uint64_t type;
	// logical dimensions (e.g. of a matrix)
	// for example, commands to a DM might usually be seen as vectors, but
	// in reality have some logical x,y dimensions, which can be important
	struct {
		uint64_t y;
		uint64_t x;
	} log_dim;
	// physical distance between successive logical rows and columns
	struct {
		double y;
		double x;
	} pitch;
};

// state of the system (written to or read from by devices)
struct aylp_state {
	// header that includes magic number and info on data in the block
	// the magic number is for clarity when writing to a file (or internet
	// packet), whereas the other info is for devices that want to know what
	// they're processing
	struct aylp_header header;
	// gsl_block of data in the pipeline:
	// (for example, a wavefront sensor might write a vector of phases to
	// this, and a deformable mirror might read a vector command from it)
	// if we want this in vector or matrix form, use the alloc_from_block
	// methods in gsl:
	// https://git.savannah.gnu.org/cgit/gsl.git/tree/matrix/init_source.c
	gsl_block *block;
	// status flags, in case devices need to selectively act differently
	uint64_t status;
	// add more parameters as needed
};

// device struct---how this is interpreted is up to the device module
struct aylp_device {
	// uri for device
	// most devices will start with the "anyloop" scheme
	char *uri;
	// params from json
	struct json_object *params;
	// initializer function
	int (*init)(struct aylp_device *self);
	// function that's called once every loop when it's the device's turn
	int (*process)(struct aylp_device *self, struct aylp_state *state);
	// destructor function
	int (*close)(struct aylp_device *self);
	// optional pointer to allow devices to store their parameters and such
	void *device_data;
};

// main config struct
struct aylp_conf {
	// number of devices
	size_t n_devices;
	// array of aylp_device objects
	struct aylp_device *devices;
};

#endif

