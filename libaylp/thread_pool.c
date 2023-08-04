#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "thread_pool.h"
#include "logging.h"


static void throw(int err)
{
	log_fatal("pthread error %d: %s", err, strerror(err));
	abort();
}


void task_enqueue(struct aylp_queue *queue, struct aylp_task *task)
{
	int err;
	err = pthread_mutex_lock(&queue->mutex);
	if (err) throw(err);
	// mutex grabbed
	{
		// add this task as oldest if there is no oldest task
		if (!queue->oldest_task) queue->oldest_task = task;
		// make the old newest_task point to this one as the next task,
		// as long as it's not the same as the current task
		if (queue->newest_task && queue->newest_task != task)
			queue->newest_task->next_task = task;
		// make the new newest_task this task
		queue->newest_task = task;
	}
	// release mutex
	err = pthread_mutex_unlock(&queue->mutex);
	if (err) throw(err);
	// tell threads that a task is ready
	err = pthread_cond_signal(&queue->ready);
	if (err) throw(err);
}


struct aylp_task *task_dequeue(struct aylp_queue *queue)
{
	int err;
	struct aylp_task *ret;
	err = pthread_mutex_lock(&queue->mutex);
	if (err) throw(err);
	// mutex grabbed
	{
		// if queue is empty, wait for a task
		while (!queue->oldest_task) {
			err = pthread_cond_wait(&queue->ready, &queue->mutex);
			if (err) throw(err);
			if (queue->exit) {
				err = pthread_mutex_unlock(&queue->mutex);
				if (err) throw(err);
				log_trace("Thread exiting");
				return 0;
			}
		}
		// grab oldest task and remove it from the queue
		ret = queue->oldest_task;
		queue->oldest_task = ret->next_task;
		// can technically overflow at SIZE_MAX; but is that really
		// worth checking?
		queue->tasks_processing += 1;
	}
	// release mutex
	err = pthread_mutex_unlock(&queue->mutex);
	if (err) throw(err);
	return ret;
}


void *task_runner(void *queue)
{
	struct aylp_queue *q = queue;
	while (1) {
		struct aylp_task *task = task_dequeue(q);
		if (q->exit) return 0;
		task->func(task->src, task->dst);
		q->tasks_processing -= 1;
	}
	return 0;
}

