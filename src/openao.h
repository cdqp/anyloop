#ifndef _OPENAO_H
#define _OPENAO_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <json-c/json.h>

// state of the system (written to or read from by devices)
struct oao_state {
	// matrix of phases [rad]:
	// (for example, a wavefront sensor might write to this, and a
	// deformable mirror might read from it)
	gsl_matrix phases;
	// physical distance between successive rows and columns
	double pitch_y;
	double pitch_x;
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

