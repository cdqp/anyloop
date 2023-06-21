#include "openao.h"

// populate devices uris and their parameters
struct oao_conf read_config(const char *file)
{
	struct oao_conf ret;

	log_info("Opening config file \"%s\"", file);
	FILE *f = fopen(file, "r");
	if (!f) {
		log_error("Could not open config file.");
		abort();
	}
	fseek(f, 0L, SEEK_END);
	size_t sz = ftell(f);
	rewind(f);

	char *buf = calloc(sz, 1);
	size_t n_read = fread(buf, 1, sz, f);
	if (n_read != sz || n_read <= 0) {
		log_error("Could not read config file.");
		abort();
	}

	struct json_object *jobj = json_tokener_parse(buf);
	if (!jobj) {
		log_error("Failed to parse config file.");
		abort();
	}
	json_object_object_foreach(jobj, key, val) {
		printf("key: \"%s\"\n", key);
		// TODO
	}


	log_info("Config file parsed");
	return ret;
}

