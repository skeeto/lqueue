#include <stdlib.h>
#include <assert.h>
#include "lqueue.h"
#include "wqueue.h"

#define QUEUE_EXPONENT 8  // 256 queue slots

struct wqueue {
    lqueue *lqueue;
    int nthreads;
    sem_t offer;
    sem_t poll;
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
        sem_wait(&data->q->poll);
        struct job job;
        int r = lqueue_poll(data->q->lqueue, &job);
        assert(r == 0);
        sem_post(&data->q->offer);
        if (job.f == NULL)
            break;
        job.f(data->id, job.arg);
    }
    return NULL;
}

wqueue *
wqueue_create(int nthreads)
{
    wqueue *q = malloc(sizeof(*q) + sizeof(q->threads[0]) * nthreads);
    q->lqueue = lqueue_create(QUEUE_EXPONENT, sizeof(struct job));
    sem_init(&q->offer, 0, (1UL << QUEUE_EXPONENT) - 1);
    sem_init(&q->poll, 0, 0);
    q->nthreads = nthreads;
    for (int i = 0; i < nthreads; i++) {
        q->threads[i].q = q;
        q->threads[i].id = i;
        pthread_create(&q->threads[i].thread, NULL, worker, &q->threads[i]);
    }
    return q;
}

void
wqueue_free(wqueue *q)
{
    for (int i = 0; i < q->nthreads; i++)
        wqueue_add(q, NULL, NULL);
    for (int i = 0; i < q->nthreads; i++)
        pthread_join(q->threads[i].thread, NULL);
    lqueue_free(q->lqueue);
    sem_close(&q->offer);
    sem_close(&q->poll);
    free(q);
}

void
wqueue_add(wqueue *q, void (*f)(int, void *), void *v)
{
    struct job job = {f, v};
    sem_wait(&q->offer);
    int r = lqueue_offer(q->lqueue, &job);
    assert(r == 0);
    sem_post(&q->poll);
}

void
wqueue_wait(wqueue *q)
{
    // TODO
    (void)q;
}
