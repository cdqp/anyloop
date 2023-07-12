#include "profile.h"

#include <string.h>
#include <assert.h>
#include <float.h>

#include "anyloop.h"
#include "logging.h"
#include "xalloc.h"

struct profile profile_new(struct aylp_conf *conf)
{
	struct profile p;
	p.conf = conf;
	p.current_device_id = -1;

	p.device_profiles = xcalloc(conf->n_devices,
		sizeof(struct device_profile)
	);
	for (size_t i=0; i<conf->n_devices; i++) {
		p.device_profiles[i].min = DBL_MAX;
	}

	return p;
}

void profile_free(struct profile *p)
{
	xfree(p->device_profiles);
}

void profile_begin_for_device(struct profile *p, size_t device_id)
{
	p->current_device_id = device_id;
	if (clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1) {
		log_error("Failed to get clock time for profiling: %s",
			strerror(errno)
		);
		exit(EXIT_FAILURE);
	}
}

void profile_end_for_device(struct profile *p, size_t device_id)
{
	assert(p->current_device_id == (ssize_t) device_id);

	struct timespec end_time;
	if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1) {
		log_error("Failed to get clock time for profiling: %s",
			strerror(errno)
		);
		exit(EXIT_FAILURE);
	}

	// calculate duration
	time_t duration_sec = end_time.tv_sec - p->start_time.tv_sec;
	long duration_nsec = end_time.tv_nsec - p->start_time.tv_nsec;

	// milliseconds
	double duration = duration_sec*1000.f + (duration_nsec/1000000.f);

	// update statistics
	struct device_profile *dp = &p->device_profiles[device_id];
	dp->sample_count++;

	if (duration < dp->min) dp->min = duration;
	if (duration > dp->max) dp->max = duration;

	// welford's online algorithm is used to perform an online mean and
	// variance calculation
	dp->mean += (duration - dp->mean)/dp->sample_count;

	// we are no longer profiling a device
	p->current_device_id = -1;
}

void profile_summary(struct profile *p)
{
	size_t max_uri_len = 0;
	for (size_t i=0; i<p->conf->n_devices; i++) {
		size_t uri_len = strlen(p->conf->devices[i].uri);
		if (uri_len > max_uri_len) max_uri_len = uri_len;
	}

	log_info("%*s | %11s | %11s | %11s",
		(int) max_uri_len + 1, "URI", "mean", "min",
		"max"
	);

	for (size_t i=0; i<p->conf->n_devices; i++) {
		struct device_profile *dp = &p->device_profiles[i];
		log_info("%-*s | %8.4f ms | %8.4f ms | %8.4f ms",
			max_uri_len + 1, p->conf->devices[i].uri, dp->mean,
			dp->min, dp->max);
	}
}
