#include <errno.h>
#include <signal.h>
#include <gsl/gsl_block.h>
#include <stdlib.h>
#include <unistd.h>
#include "anyloop.h"
#include "logging.h"
#include "config.h"
#include "device.h"

struct aylp_state state = {0};
struct aylp_conf conf;


static void cleanup(void)
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
	// we actually *don't* want to free the state block/vector/matrix/bytes,
	// because it will never be the only pointer to that data, and will have
	// been freed by whatever device really owns that data.
}


void handle_signal(int sig)
{
	if (sig == SIGINT) {
		log_info("Caught SIGINT; cleaning up");
		cleanup();
		log_info("Exiting now!");
		exit(0);
	}
}


int main(int argc, char **argv)
{
	UNUSED(argc);

	log_status_t log_status;
	log_status.level = LOG_INFO;
	log_status.use_color = isatty(STDERR_FILENO);
	log_init(&log_status);

	// copy magic number to header
	state.header.magic = AYLP_MAGIC;
	// allocate the block
	state.block = gsl_block_alloc(0);

	// parse options
	for (argv++; argv; argv++) {
		char *arg = *argv;
		if (arg[0] != '-' || strcmp(arg, "--") == 0)
			break;

		if (strcmp(arg, "-loglevel") == 0) {
			argv++;
			if (*argv) {
				if (!log_set_level_by_name(*argv))
				return EXIT_FAILURE;
			} else {
				log_fatal("Expected an argument to -loglevel");
				return EXIT_FAILURE;
			}
		} else {
			log_fatal("Unknown option %s", arg);
			return EXIT_FAILURE;
		}
	}

	// parse filename
	if (argv) {
		conf = read_config(*argv);
	} else {
		log_info("Usage: `anyloop [-loglevel LOG_LEVEL] [--] path_to_conf.json`");
		log_fatal("Please provide a config file.");
		return EXIT_FAILURE;
	}

	if (signal(SIGINT, handle_signal) == SIG_ERR) {
		log_fatal("Failed to attach signal handler to SIGINT.");
		return EXIT_FAILURE;
	}

	// initialize all devices
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		if (!conf.devices[idx].uri) {
			log_fatal("Device %d as no uri; config issue?", idx);
			log_info("(note that comments are not allowed as top-"
				"level entries in the pipeline array)"
			);
			return EXIT_FAILURE;
		}
		conf.devices[idx].log_status = &log_status;
		if (init_device(&conf.devices[idx])) {
			log_fatal("Could not initialize %s.",
				conf.devices[idx].uri
			);
			cleanup();
			return EXIT_FAILURE;
		} else {
			log_info("Initialized %s.", conf.devices[idx].uri);
		}
	}

	// typecheck the device pipeline
	aylp_type type_cur = AYLP_T_NONE;
	aylp_units units_cur = AYLP_U_NONE;
	// first device must be compatible with _NONE and output of last device
	type_cur |= conf.devices[conf.n_devices-1].type_out;
	units_cur |= conf.devices[conf.n_devices-1].units_out;
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		struct aylp_device d = conf.devices[idx];	// brevity
		log_trace("type check: prev=0x%hX, in=0x%hX, out=0x%hX",
			type_cur, d.type_in, d.type_out
		);
		log_trace("unit check: prev=0x%hX, in=0x%hX, out=0x%hX",
			units_cur, d.units_in, d.units_out
		);
		if (!(d.type_in & type_cur)) {
			log_fatal("Device %s with input type 0x%hX "
				"is incompatible with previous type 0x%hX",
				d.uri, d.type_in, type_cur
			);
			return EXIT_FAILURE;
		}
		if (!(d.units_in & units_cur)) {
			log_fatal("Device %s with input units 0x%hX "
				"is incompatible with previous units 0x%hX",
				d.uri, d.units_in, units_cur
			);
			return EXIT_FAILURE;
		}
		if (d.type_out)
			type_cur = d.type_out;
		if (d.units_out)
			units_cur = d.units_out;
	}

	while (state.header.status ^ AYLP_DONE) {
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
	cleanup();
	log_info("Exiting now!");
	return EXIT_SUCCESS;
}

