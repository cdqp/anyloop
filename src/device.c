#include <dlfcn.h>
#include <libgen.h>
#include <string.h>
#include "anyloop.h"
#include "logging.h"
#include "device.h"


int init_device(struct aylp_device *dev)
{
	int device_found = 0;

	if (!strncmp(dev->uri, "anyloop", sizeof("anyloop")-1)) {
		static const size_t imax = sizeof(init_map)/sizeof(init_map[0]);
		for (size_t idx=0; idx<imax; idx++) {
			if (!strcmp(init_map[idx].uri, dev->uri)) {
				device_found = 1;
				dev->init = init_map[idx].init_fun;
			}
		}
	} else if (!strncmp(dev->uri, "file", sizeof("file")-1)) {
		char *path = dev->uri + sizeof("file:")-1;
		void *plug_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
		if (!plug_handle) {
			log_error(
				"Could not open plug-in device: %s", dlerror()
			);
			return -1;
		}
		device_found = 1;
		char *bn = strdup(basename(path));
		char *dot = strchr(bn, '.');
		if (dot) {
			*dot = 0;	// strip file extension
		}
		// weird cast for compiler warning
		*(void **) (&dev->init) = dlsym(
			plug_handle, strcat(bn, "_init")
		);
		free(bn); bn = 0;
	} else {
		log_error("Device has unsupported URI scheme.");
		return -1;
	}

	if (!device_found) {
		log_error("Could not find device %s", dev->uri);
	}
	if (dev->init) {
		log_info("Initializing %s", dev->uri);
		return dev->init(dev);
	} else {
		log_error("Init function for %s was null", dev->uri);
		return -1;
	}
}

