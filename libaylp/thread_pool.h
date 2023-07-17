#ifndef AYLP_DEVICES_THREAD_POOL_H_
#define AYLP_DEVICES_THREAD_POOL_H_

#include <pthread.h>

/** Task object to be put in queues. Essentially forms a singly linked list. */
struct aylp_task {
	void (*func)(void *src, void *dst);	// the function to call
	void *src;				// where to read input from
	void *dst;				// where to put output
	struct aylp_task *next_task;		// older task on queue, if any
};

/** Queue object for thread pool.
* The managing process should use task_enqueue and task_dequeue to add and
* remove tasks to this queue, and should use is_empty to decide if all tasks are
* done in the queue. The process should also call shut_queue to make all threads
* waiting on the queue to exit. */
struct aylp_queue {
	pthread_mutex_t mutex;	// prevent race conditions
	pthread_cond_t ready;	// whether there is a task ready
	struct aylp_task *oldest_task;	// for dequeue
	struct aylp_task *newest_task;	// for enqueue
	int exit;	// true when threads should exit
};

/** Add a new task to the queue. This call blocks. Returns error code. */
void task_enqueue(struct aylp_queue *queue, struct aylp_task *task);

/** Grab the oldest task (if any), or wait for a new one if there are no tasks.
* Put the grabbed task into the task pointer. This call blocks. Returns error
* code. */
struct aylp_task *task_dequeue(struct aylp_queue *queue);

/** Start running tasks on the queue. Return when queue->exit is set. */
void *task_runner(void *queue);

/** Check if queue is empty. */
static inline int is_empty(struct aylp_queue *queue) {
	return !queue->oldest_task;
}

/** Shut down queue, telling threads to exit. */
static inline void shut_queue(struct aylp_queue *queue) {
	queue->exit = 1;
	pthread_cond_broadcast(&queue->ready);
}

#endif

