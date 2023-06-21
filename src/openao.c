#include "openao.h"
#include "logging.h"
#include "config.h"
#include "device.h"

struct oao_state state = {0};
struct oao_conf conf;


int main(int argc, char *argv[])
{
	char *cf = argv[1];
	if (cf) {
		conf = read_config(cf);
	} else {
		log_info("Usage: `openao openao_conf.json`");
		log_error("Please provide a config file.");
		abort();
	}

	// initialize all devices
	for (size_t idx=0; idx<conf.n_devices; idx++) {
		init_device(&conf.devices[idx]);
	}

	int done = 0;
	while (!done) {
		for (size_t idx=0; idx<conf.n_devices; idx++) {
			struct oao_device *dev = &conf.devices[idx];
			dev->process(dev, &state);
		}
		done = 1;
	}
}

