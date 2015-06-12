#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include "lqueue.h"

struct lqueue {
    _Atomic unsigned long head;
    _Atomic unsigned long tail;
    unsigned long mask;
    size_t element_size;
    char buffer[];
};

lqueue *
lqueue_create(int exponent, size_t element_size)
{
    unsigned long size = 1UL << exponent;
    lqueue *q = malloc(sizeof(*q) + element_size * size);
    q->mask = size - 1;
    q->element_size = element_size;
    q->head = ATOMIC_VAR_INIT(0);
    q->tail = ATOMIC_VAR_INIT(0);
    return q;
}

void
lqueue_free(lqueue *q)
{
    free(q);
}

int
lqueue_offer(lqueue *q, void *v)
{
    unsigned long head = atomic_load(&q->head);
    unsigned long tail = atomic_load(&q->tail);
    if (((tail + 1) & q->mask) == head)
        return 1;
    memcpy(q->buffer + q->element_size * tail, v, q->element_size);
    atomic_store(&q->tail, ((tail + 1) & q->mask));
    return 0;
}

int
lqueue_poll(lqueue *q, void *v)
{
    unsigned long head = atomic_load(&q->head);
    unsigned long tail = atomic_load(&q->tail);
    unsigned long next_head;
    do {
        if (head == tail)
            return 1;
        memcpy(v, q->buffer + q->element_size * head, q->element_size);
        next_head = (head + 1) & q->mask;
    } while (!atomic_compare_exchange_weak(&q->head, &head, next_head));
    return 0;
}
