#include <stdlib.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
#include "lqueue.h"
#include "wqueue.h"

#define POISON  NULL

struct wqueue {
    lqueue *lqueue;
    int nthreads;
    sem_t count;
    struct wqueue_data {
        pthread_t thread;
        int id;
        struct wqueue *q;
    } threads[];
};

struct job {
    void (*f)(int, void *);
    void *arg;
};

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
wqueue_create(unsigned max_size, int nthreads)
{
    assert((unsigned)nthreads < max_size);
    nthreads--;
    wqueue *q = malloc(sizeof(*q) + sizeof(q->threads[0]) * nthreads);
    q->lqueue = lqueue_create(max_size, sizeof(struct job));
    sem_init(&q->count, 0, 0);
    q->nthreads = nthreads;
    for (int i = 0; i < nthreads; i++) {
        q->threads[i].q = q;
        q->threads[i].id = i + 1;
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
    for (int i = 0; i < q->nthreads; i++)
        pthread_join(q->threads[i].thread, NULL);
    lqueue_free(q->lqueue);
    sem_destroy(&q->count);
    free(q);
}

void
wqueue_add(wqueue *q, void (*f)(int, void *), void *v)
{
    struct job job = {f, v};
    while (lqueue_offer(q->lqueue, &job) != 0) {
        struct job job;
        if (0 == lqueue_poll(q->lqueue, &job))
            job.f(0, job.arg);
    }
    sem_post(&q->count);
}

void
wqueue_wait(wqueue *q)
{
    struct job job;
    while (0 == lqueue_poll(q->lqueue, &job))
        job.f(0, job.arg);
}
