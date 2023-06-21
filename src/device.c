#include "openao.h"
#include "logging.h"
#include "device.h"


void init_device(struct oao_device *dev)
{
	for (size_t idx=0; idx<(sizeof(init_map)/sizeof(init_map[0])); idx++) {
		if (!strcmp(init_map[idx].uri, dev->uri)) {
			dev->init = init_map[idx].init_fun;
		}
	}
	if (dev->init) {
		log_info("Initializing %s", dev->uri);
		dev->init(dev);
	}
	return;
}

