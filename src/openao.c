#include "openao.h"
#include "logging.c"
#include "config.c"

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
		
	return 1;

	while (1) {
		//pre_sense_hook();
		//sense();
		//pre_modulate_hook();
		//modulate();
	}
}

