#ifndef _AYLP_FILE_SINK_H
#define _AYLP_FILE_SINK_H

#include "anyloop.h"

struct aylp_file_sink_data {
	char *filename;
};

// initialize file sink device
int file_sink_init(struct aylp_device *self);

// process file sink device once per loop
int file_sink_process(struct aylp_device *self, struct aylp_state *state);

// close file sink device when loop exits
int file_sink_close(struct aylp_device *self);

#endif

