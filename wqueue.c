#include <stdlib.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
#include "lqueue.h"
#include "wqueue.h"

#define POISON NULL

struct wqueue {
    lqueue *lqueue;
    sem_t count;
    int nthreads;
    struct wqueue_data {
        pthread_t thread;
        int id;
        struct wqueue *q;
        sem_t pause;
        sem_t complete;
    } threads[];
};

struct job {
    void (*f)(int, void *);
    void *arg;
};

static void
pause(int id, void *arg)
{
    wqueue *q = arg;
    sem_post(&q->threads[id - 1].complete);
    sem_wait(&q->threads[id - 1].pause);
}

static void *
worker(void *arg)
{
    struct wqueue_data *data = arg;
    for (;;) {
        sem_wait(&data->q->count);
        struct job job;
        if (0 == lqueue_poll(data->q->lqueue, &job)) {
            if (job.f == POISON)
                break;
            job.f(data->id, job.arg);
        }
    }
    return NULL;
}

wqueue *
wqueue_create(unsigned min_size, int nthreads)
{
    assert(nthreads > 0);
    assert((unsigned)nthreads < min_size);
    nthreads--;
    wqueue *q = malloc(sizeof(*q) + sizeof(q->threads[0]) * nthreads);
    q->lqueue = lqueue_create(min_size, sizeof(struct job));
    sem_init(&q->count, 0, 0);
    q->nthreads = nthreads;
    for (int i = 0; i < nthreads; i++) {
        q->threads[i].q = q;
        q->threads[i].id = i + 1;
        sem_init(&q->threads[i].pause, 0, 0);
        sem_init(&q->threads[i].complete, 0, 0);
        pthread_create(&q->threads[i].thread, NULL, worker, &q->threads[i]);
    }
    return q;
}

void
wqueue_free(wqueue *q)
{
    wqueue_wait(q);
    for (int i = 0; i < q->nthreads; i++)
        wqueue_add(q, POISON, NULL);
    for (int i = 0; i < q->nthreads; i++) {
        pthread_join(q->threads[i].thread, NULL);
        sem_destroy(&q->threads[i].pause);
        sem_destroy(&q->threads[i].complete);
    }
    lqueue_free(q->lqueue);
    sem_destroy(&q->count);
    free(q);
}

void
wqueue_add(wqueue *q, void (*f)(int, void *), void *v)
{
    struct job job = {f, v};
    while (lqueue_offer(q->lqueue, &job) != 0) {
        /* Help finish jobs until a spot opens in the queue. */
        struct job job;
        if (0 == lqueue_poll(q->lqueue, &job))
            job.f(0, job.arg);
    }
    sem_post(&q->count);
}

void
wqueue_wait(wqueue *q)
{
    /* Help finish running jobs. */
    struct job job;
    while (0 == lqueue_poll(q->lqueue, &job))
        job.f(0, job.arg);
    /* Ask all threads to pause. */
    for (int i = 0; i < q->nthreads; i++)
        wqueue_add(q, pause, q);
    /* Wait for all threads to complete. */
    for (int i = 0; i < q->nthreads; i++)
        sem_wait(&q->threads[i].complete);
    /* Unpause threads. */
    for (int i = 0; i < q->nthreads; i++)
        sem_post(&q->threads[i].pause);
}
