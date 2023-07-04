#include <errno.h>
#include <signal.h>
#include <gsl/gsl_block.h>
#include "anyloop.h"
#include "logging.h"
#include "config.h"
#include "device.h"

struct aylp_state state = {0};
struct aylp_conf conf;


void _cleanup(void)
{
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		struct aylp_device *dev = &conf.devices[idx];
		if (dev->close) {
			dev->close(dev);
		}
		log_trace("Closed %s", dev->uri);
	}
	conf.n_devices = 0;
	free(conf.devices); conf.devices = 0;
	gsl_block_free(state.block);
}


void handle_signal(int sig)
{
	if (sig == SIGINT) {
		log_info("Caught SIGINT; cleaning up");
		_cleanup();
		log_info("Exiting now!");
		exit(0);
	}
}


int main(int argc, char *argv[])
{
	if (signal(SIGINT, handle_signal) == SIG_ERR) {
		log_fatal("Failed to attach signal handler to SIGINT.");
		return 1;
	}

	// copy magic number to header
	state.header.magic = AYLPDATA_MAGIC;
	// allocate the block
	state.block = gsl_block_alloc(0);

	// TODO: in addition to config file, parse a log level param
	// (probably want getopt?)
	if (argc > 1) {
		conf = read_config(argv[1]);
	} else {
		log_info("Usage: `anyloop path_to_conf.json`");
		log_fatal("Please provide a config file.");
		return 1;
	}

	// initialize all devices
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		if (!conf.devices[idx].uri) {
			log_fatal("Device %d as no uri; config issue?", idx);
			log_info("(note that comments are not allowed as top-"
				"level entries in the pipeline array)"
			);
			return 1;
		}
		if (init_device(&conf.devices[idx])) {
			log_fatal("Could not initialize %s.",
				conf.devices[idx].uri
			);
			_cleanup();
			return 1;
		} else {
			log_info("Initialized %s.", conf.devices[idx].uri);
		}
	}

	while (state.status ^ AYLP_DONE) {
		for (size_t idx=0; idx<conf.n_devices; idx++) {
			struct aylp_device *dev = &conf.devices[idx];
			if (dev->process) {
				dev->process(dev, &state);
				// this logging call could be too much overhead
				// even when log level is above trace, but I
				// doubt it
				log_trace("Processed %s", dev->uri);
			} else {
				log_trace("Not processing %s", dev->uri);
			}
		}
	}

	log_info("AYLP_DONE was set; cleaning up");
	_cleanup();
	log_info("Exiting now!");
	return 0;
}

