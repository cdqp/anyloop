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
	}
	conf.n_devices = 0;
	free(conf.devices); conf.devices = 0;
}


int main(int argc, char *argv[])
{
	char *cf = argv[1];
	if (cf) {
		conf = read_config(cf);
	} else {
		log_info("Usage: `openao openao_conf.json`");
		log_error("Please provide a config file.");
		return 1;
	}

	// initialize all devices
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		if (init_device(&conf.devices[idx])) {
			log_error("Could not initialize %s. Exiting.",
				conf.devices[idx].uri
			);
			_cleanup();
			return 1;
		}
	}

	int done = 0;
	while (!done) {
		for (size_t idx=0; idx<conf.n_devices; idx++) {
			struct oao_device *dev = &conf.devices[idx];
			if (dev->process) {
				dev->process(dev, &state);
			}
		}
		done = 1;
	}

	// TODO: sigint handling
	_cleanup();
	log_info("Exiting. Goodbye!");
	return 0;
}

