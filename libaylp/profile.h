#ifndef AYLP_PROFILE_H_
#define AYLP_PROFILE_H_

#include <stdlib.h>
#include <time.h>

#include "anyloop.h"

struct device_profile {
	// time statistics in milliseconds
	double max;
	double min;
	double mean;
	size_t sample_count;
};

struct profile {
	struct aylp_conf *conf;
	struct device_profile *device_profiles;

	// transient per-device profiling state
	struct timespec start_time;
	ssize_t current_device_id;
};

struct profile profile_new(struct aylp_conf *conf);
void profile_free(struct profile *p);

void profile_begin_for_device(struct profile *p, size_t device_id);
void profile_end_for_device(struct profile *p, size_t device_id);

void profile_summary(struct profile *p);

#endif
