#ifndef LQUEUE_H
#define LQUEUE_H

#include <stdatomic.h>

typedef struct {
    _Atomic unsigned long head;
    _Atomic unsigned long tail;
    unsigned long mask;
    _Atomic void *elements[];
} lqueue;

lqueue *lqueue_create(int exponent);
void    lqueue_free(lqueue *);
int     lqueue_offer(lqueue *, void *);
void   *lqueue_poll(lqueue *);

#endif
