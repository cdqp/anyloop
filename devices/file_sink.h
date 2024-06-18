#ifndef AYLP_DEVICES_FILE_SINK_H_
#define AYLP_DEVICES_FILE_SINK_H_

#include <stdio.h>
#include "anyloop.h"

/** aylp:file_sink
 *
 * types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`
 *
 * This device writes the current pipeline state to an AYLP file. See
 * [filetype.md](../doc/filetype.md) for documentation on the AYLP file format.
 *
 * Parameters:
 *   - `filename` (string) (required)
 *     - The filename to write the pipeline data to.
 *   - `flush` (boolean) (optional)
 *     - Whether or not to flush the output every iteration. Setting this may
 *       slightly hinder performance, but will ensure that anything reading the
 *       file is not waiting for a buffered write.
 */

struct aylp_file_sink_data {
	// file to sink data to
	FILE *fp;
	// whether or not to flush the file every process()
	json_bool flush;
	// gsl_block_uchar that we will copy pointer to data to
	gsl_block_uchar bytes;
};

// initialize file sink device
int file_sink_init(struct aylp_device *self);

// process file sink device once per loop
int file_sink_process(struct aylp_device *self, struct aylp_state *state);

// close file sink device when loop exits
int file_sink_close(struct aylp_device *self);

#endif

