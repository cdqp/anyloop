#include <json-c/json.h>
#include "anyloop.h"
#include "logging.h"
#include "config.h"


struct aylp_conf read_config(const char *file)
{
	struct aylp_conf ret;

	log_info("Opening config file \"%s\"", file);
	struct json_object *jobj = json_object_from_file(file);
	if (!jobj) {
		log_fatal(json_util_get_last_err());
		exit(1);
	}

	json_object_object_foreach(jobj, tlkey, sub1) {
		// parse top-level keys
		if (tlkey[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(tlkey, "pipeline")) {
			// parse devices in pipeline
			if (!json_object_is_type(sub1, json_type_array)) {
				log_fatal("Pipeline object must be array");
				exit(1);
			}
			ret.n_devices = json_object_array_length(sub1);
			ret.devices = (struct aylp_device *)calloc(
				ret.n_devices, sizeof(struct aylp_device)
			);
			log_info("Seeing %d devices", ret.n_devices);
			for (size_t idx=0; idx<ret.n_devices; idx++) {
				json_object *sub2 = json_object_array_get_idx(
					sub1, idx
				);
				json_object_object_foreach(sub2, sub3, sub4) {
					if (sub3[0] == '_') {
						// it's a comment
					} else if (!strcmp(sub3, "uri")) {
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
						log_warn("Unknown pipeline "
							"key: \"%s\"", sub3
						);
					}
				}
			}
		} else {
			log_warn("Unknown config key: \"%s\"", tlkey);
		}
	}

	log_info("Config file parsed");

	// cleanup
	json_object_object_foreach(jobj, key, _) {
		json_object_object_del(jobj, key);
	}
	free(jobj); jobj = 0;
	return ret;
}

