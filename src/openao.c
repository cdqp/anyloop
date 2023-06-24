#include <signal.h>
#include <string.h>
#include "openao.h"
#include "logging.h"
#include "config.h"
#include "device.h"

struct oao_state state = {0};
struct oao_conf conf;


void _cleanup()
{
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		struct oao_device *dev = &conf.devices[idx];
		if (dev->close) {
			dev->close(dev);
		}
		log_trace("Closed %s", dev->uri);
	}
	conf.n_devices = 0;
	free(conf.devices); conf.devices = 0;
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
	strcpy(state.header.magic, "OAO_DATA");

	// TODO: in addition to config file, parse a log level param
	// (probably want getopt?)
	char *cf = argv[1];
	if (cf) {
		conf = read_config(cf);
	} else {
		log_info("Usage: `openao openao_conf.json`");
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

	while (state.status ^ OAO_DONE) {
		for (size_t idx=0; idx<conf.n_devices; idx++) {
			struct oao_device *dev = &conf.devices[idx];
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

	log_info("OAO_DONE was set; cleaning up");
	_cleanup();
	log_info("Exiting now!");
	return 0;
}

