/**
 * Atomic, bounded work queue for one writer and multiple readers.
 * Threads are spawned internally by the work queue on creation. For
 * each job added to the queue with wqueue_add(), the pass function is
 * called with the queue-local thread id (0 - n) and the given void
 * pointer argument. The thread id can be used to access, for example,
 * custom thread-local storage.
 *
 * When the queue is full, the caller of wqueue_add() (thread 0) will
 * assist in completing jobs to open up space in the queue. Similarly,
 * in wqueue_wait(), the calling thread will assist in jobs until all
 * jobs are consumed *and* completed. Because of this, the queue
 * spawns one less thread, because the "main" queuing thread
 * participates in job completion.
 *
 * Calling wqueue_free() will block (a la wqueue_wait()) for all jobs
 * to complete before destroying the queue.
 */
#ifndef WQUEUE_H
#define WQUEUE_H

typedef struct wqueue wqueue;

wqueue *wqueue_create(unsigned min_size, int nthreads);
void    wqueue_free(wqueue *);
void    wqueue_add(wqueue *, void (*)(int, void *), void *);
void    wqueue_wait(wqueue *);

#endif
