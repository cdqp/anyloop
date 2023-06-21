#include "openao.h"
#include "logging.h"
#include "config.h"


struct oao_conf read_config(const char *file)
{
	struct oao_conf ret;

	log_info("Opening config file \"%s\"", file);
	struct json_object *jobj = json_object_from_file(file);
	if (!jobj) {
		log_error(json_util_get_last_err());
		abort();
	}

	json_object_object_foreach(jobj, tlkey, sub1) {
		// parse top-level keys
		if (!strcmp(tlkey, "pipeline")) {
			// parse devices in pipeline
			ret.n_devices = json_object_array_length(sub1);
			ret.devices = (struct oao_device *)calloc(
				ret.n_devices, sizeof(struct oao_device)
			);
			log_info("Seeing %d devices", ret.n_devices);
			for (size_t idx=0; idx<ret.n_devices; idx++) {
				json_object *sub2 = json_object_array_get_idx(
					sub1, idx
				);
				json_object_object_foreach(sub2, sub3, sub4) {
					if (!strcmp(sub3, "uri")) {
						const char *uri =
							json_object_get_string(
								sub4
							);
						ret.devices[idx].uri =
							strdup(uri);
						log_info("Found device with "
							"uri: %s",
							ret.devices[idx].uri
						);
					} else if (!strcmp(sub3, "params")) {
						log_info("Found params for "
							"uri: %s",
							ret.devices[idx].uri
						);
						// take ownership of param obj
						ret.devices[idx].params =
							json_object_get(sub4);
					} else {
						log_error("Unknown device key: "
							"\"%s\"", sub3
						);
						abort();
					}
				}
			}
		} else {
			log_error("Unknown config key: \"%s\"", tlkey);
			abort();
		}
	}

	log_info("Config file parsed");
	return ret;
}

