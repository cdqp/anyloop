#ifndef AYLP_DEVICES_THREAD_POOL_H_
#define AYLP_DEVICES_THREAD_POOL_H_

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

/** Task object to be put in queues. Essentially forms a singly linked list. */
struct aylp_task {
	void (*func)(void *src, void *dst);	// the function to call
	void *src;				// where to read input from
	void *dst;				// where to put output
	struct aylp_task *next_task;		// older task on queue, if any
};

/** Queue object for thread pool.
* The managing process should use task_enqueue and task_dequeue to add and
* remove tasks to this queue, and should use check tasks_processing to decide if
* all tasks are done in the queue. The process should also call shut_queue to
* make all threads waiting on the queue to exit. */
struct aylp_queue {
	// mutex for queue writes to prevent race conditions
	pthread_mutex_t mutex;
	// condition to signal that there is a task ready
	pthread_cond_t ready;
	// the oldest task in the queue, which will be processed next
	struct aylp_task *oldest_task;
	// the task most recently added to the queue (it may be completed!)
	struct aylp_task *newest_task;
	// true when threads should exit
	bool exit;
	// count of tasks that are not done; safe to read/write without mutex
	atomic_size_t tasks_processing;
};

/** Add a new task to the queue. This call blocks. */
void task_enqueue(struct aylp_queue *queue, struct aylp_task *task);

/** Grab the oldest task (if any), or wait for a new one if there are no tasks.
* This call blocks. Returns a pointer to the oldest task and removes it from the
* queue. */
struct aylp_task *task_dequeue(struct aylp_queue *queue);

/** Start running tasks on the queue.
 * Tasks_processing will be decremented each time a task is completed. Return
 * when queue->exit is set. */
void *task_runner(void *queue);

/** Shut down queue, telling threads to exit. */
static inline void shut_queue(struct aylp_queue *queue) {
	queue->exit = true;
	pthread_cond_broadcast(&queue->ready);
}

#endif

