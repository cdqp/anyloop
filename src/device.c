#include "openao.h"
#include "logging.h"
#include "device.h"


int init_device(struct oao_device *dev)
{
	int device_found = 0;
	for (size_t idx=0; idx<(sizeof(init_map)/sizeof(init_map[0])); idx++) {
		if (!strcmp(init_map[idx].uri, dev->uri)) {
			device_found = 1;
			dev->init = init_map[idx].init_fun;
		}
	}
	if (!device_found) {
		log_error("Could not find device %s", dev->uri);
	}
	if (dev->init) {
		log_info("Initializing %s", dev->uri);
		int ret = dev->init(dev);
		return ret;
	} else {
		// assume device doesn't need initialization
		log_info("Not initializing %s", dev->uri);
		return 0;
	}
}

