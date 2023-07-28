#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <gsl/gsl_block.h>

#include "anyloop.h"
#include "logging.h"
#include "config.h"
#include "xalloc.h"
#include "../devices/device.h"

const char *help_msg = "\nUsage: `anyloop [options] your_config_file.json`\n"
	"Options:\n"
	"-h/--help               print this help message and exit\n"
	"-l/--loglevel <level>   set log level\n"
	"-p/--profile            enable profiling\n"
	"Allowed log levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL\n"
	"Example: `anyloop -pl TRACE contrib/conf_example.json\n"
;

struct aylp_state state = {0};
struct aylp_conf conf;

static bool sigint_received = false;

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
	xfree(conf.devices);
	// we actually *don't* want to free the state block/vector/matrix/etc,
	// because it will never be the only pointer to that data, and will have
	// been freed by whatever device really owns that data.
}


void handle_signal(int sig, siginfo_t *info, void *context)
{
	UNUSED(info);
	UNUSED(context);
	if (sig == SIGINT) {
		sigint_received = true;
	}
}


// Read the argument at arg; if it matches opt_short or opt_long, return true,
// and additionally, if value is non-null, put arg+1 into *value. For example,
// if opt_short is 'o' and opt_long is "option", then both "-o foo" and
// "--option foo" will set `*value="foo"`. Also decrease chars_remaining if we
// matched an option or even set it to zero if we matched a long option.
static bool check_opt(char **arg, char opt_short, char *opt_long,
	char **value, int *chars_remaining
){
	if ((*arg)[0] != '-') return false;
	if ((*arg)[1] == '-') {
		if (strcmp(*arg+2, opt_long)) return false;
		if (chars_remaining) *chars_remaining = 0;
	} else {
		char *occ = strchr(*arg, opt_short);
		if (!occ) return false;
		*occ = '_';	// replace occurence with underscore
		if (chars_remaining) *chars_remaining -= 1;
	}
	// okay, it's a match; grab the value if needed
	if (value) *value = arg[1];
	// set the value to null so we don't try to parse it as an option
	arg[1] = 0;
	return true;
}


int main(int argc, char **argv)
{
	int err;
	// initialize logger with default level
	log_init(LOG_INFO);
	// copy magic number to header
	state.header.magic = AYLP_MAGIC;

	// check for lone -h without config file
	if (check_opt(argv+argc-1, 'h', "help", 0, 0)) {
		log_info(help_msg);
		return EXIT_SUCCESS;
	}
	// parse all but last argument
	for (int i = 1; i < argc-1; i++) {
		// if arg is null, it was handled already; keep going
		if (!argv[i]) continue;

		int remain = strlen(argv[i]);
		char *val;
		if (check_opt(argv+i, 'h', "help", 0, &remain)) {
			log_info(help_msg);
			return EXIT_SUCCESS;
		}
		if (check_opt(argv+i, 'l', "loglevel", &val, &remain)) {
			if (!val) {
				log_fatal("Expected a value for "
					"-l/--loglevel", argv[i]
				);
				return EXIT_FAILURE;
			}
			if (!log_set_level_by_name(val))
				return EXIT_FAILURE;
		}
		if (check_opt(argv+i, 'p', "profile", &val, &remain)) {
			log_warn("-p/--profile not yet implemented.");
			// TODO: @wooosh
		}
		// add more check_opt calls for new options

		// if there are chars remaining that we didn't parse and they
		// aren't just the leading '-' from short options, throw error
		if (remain && !(remain == 1 && argv[i][0] == '-')) {
			log_fatal("Unknown or repeated option(s): %s", argv[i]);
			return EXIT_FAILURE;
		}
	}

	// parse filename from last argument
	if (argc < 2 || !argv[argc-1]) {
		log_info(help_msg);
		log_fatal("Please provide a config file.");
		return EXIT_FAILURE;
	} else {
		conf = read_config(argv[argc-1]);
	}

	struct sigaction signal_handler;
	signal_handler.sa_flags = SA_SIGINFO;
	sigemptyset(&signal_handler.sa_mask);
	signal_handler.sa_sigaction = handle_signal;
	if (sigaction(SIGINT, &signal_handler, NULL) == -1) {
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
		// typecheck fails if any of the bits set in type_cur are not
		// set in type_in
		if ((d.type_in ^ type_cur) & type_cur) {
			log_fatal("Device %s with input type 0x%hX "
				"is incompatible with previous type 0x%hX",
				d.uri, d.type_in, type_cur
			);
			return EXIT_FAILURE;
		}
		log_trace("unit check: prev=0x%hX, in=0x%hX, out=0x%hX",
			units_cur, d.units_in, d.units_out
		);
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

	while (!sigint_received && state.header.status ^ AYLP_DONE) {
		for (size_t d=0; !sigint_received && d<conf.n_devices; d++) {
			struct aylp_device *dev = &conf.devices[d];
			if (dev->process) {
				err = dev->process(dev, &state);
				// errors are assumed recoverable (e.g. UDP
				// fails to send) unless the device was supposed
				// to change the type
				if (err && dev->type_out
				&& dev->type_out != dev->type_in) {
					log_fatal("%s returned error %d but "
						"was expected to change the "
						"pipeline type. Exiting.",
						dev->uri, err
					);
					cleanup();
					return EXIT_FAILURE;
				}
				// this logging call could be too much overhead
				// even when log level is above trace, but I
				// doubt it
				log_trace("Processed %s", dev->uri);
			} else {
				log_trace("Not processing %s", dev->uri);
			}
		}
	}

	if (sigint_received)
		log_info("Caught SIGINT; cleaning up");
	else
		log_info("AYLP_DONE was set; cleaning up");
	cleanup();
	log_info("Exiting now!");
	return EXIT_SUCCESS;
}

