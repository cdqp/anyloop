#ifndef _OPENAO_H
#define _OPENAO_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>

// flags that can be set to tell devices different things
enum oao_status {
	OAO_DONE = 1 << 0,	// done with loop
	// add more as necessary
};

// type of data written to block
// this is useful for a few reasons; for example, a DM might want to be able to
// take a command directly (after having gone through whatever processing device
// upstream), while also being able to just take raw phases (e.g. from a von
// Kármán stream) and scale them according to the stroke depth
enum oao_blocktype {
	OAO_PHASES = 1 << 0,	// a screen of phases in radians
	OAO_COMMAND = 1 << 1,	// a command to send to a device (range ±1)
	// add more as necessary
};

// information about data in block
struct oao_blockinfo {
	// type of data written to block
	enum oao_blocktype type;
	// logical dimensions (e.g. of a matrix)
	// for example, commands to a DM might usually be seen as vectors, but
	// in reality have some logical x,y dimensions, which can be important
	struct {
		size_t y;
		size_t x;
	} log_dim;
	// physical distance between successive logical rows and columns
	struct {
		double y;
		double x;
	} pitch;
};

// state of the system (written to or read from by devices)
struct oao_state {
	// gsl_block of data in the pipeline:
	// (for example, a wavefront sensor might write a vector of phases to
	// this, and a deformable mirror might read a vector command from it)
	// if we want this in vector or matrix form, use the alloc_from_block
	// methods in gsl:
	// https://git.savannah.gnu.org/cgit/gsl.git/tree/matrix/init_source.c
	gsl_block block;
	// type of data in block
	struct oao_blockinfo info;
	// a status enum, in case devices need to selectively act differently
	enum oao_status status;
	// add more parameters as needed
};

// device struct---how this is interpreted is up to the device module
struct oao_device {
	// uri for device (e.g. ip, serial, etc.)
	// some custom devices will start with the "openao" scheme
	char *uri;
	// params from json
	struct json_object *params;
	// initializer function
	int (*init)(struct oao_device *self);
	// function that's called once every loop when it's the device's turn
	int (*process)(struct oao_device *self, struct oao_state *state);
	// destructor function
	int (*close)(struct oao_device *self);
	// optional pointer to allow devices to store their parameters and such
	void *device_data;
};

// main config struct
struct oao_conf {
	// number of devices
	size_t n_devices;
	// array of oao_device objects
	struct oao_device *devices;
};

#include "device.h"

#endif

