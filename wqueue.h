#ifndef WQUEUE_H
#define WQUEUE_H

#include <semaphore.h>
#include <pthread.h>
#include "lqueue.h"

typedef struct wqueue wqueue;

wqueue *wqueue_create(int nthreads);
void    wqueue_free(wqueue *);
void    wqueue_add(wqueue *, void (*)(int, void *), void *);
void    wqueue_wait(wqueue *);

#endif
