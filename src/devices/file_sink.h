#ifndef _OPENAO_FILE_SINK_H
#define _OPENAO_FILE_SINK_H

#include "openao.h"

struct oao_file_sink_data {
	char *filename;
};

// initialize file sink device
int file_sink_init(struct oao_device *self);

// process file sink device once per loop
int file_sink_process(struct oao_device *self, struct oao_state *state);

// close file sink device when loop exits
int file_sink_close(struct oao_device *self);

#endif

