#include <stdlib.h>
#include <errno.h>
#include "lqueue.h"

lqueue *
lqueue_create(int exponent)
{
    unsigned long size = 1UL << exponent;
    lqueue *q = malloc(sizeof(*q) + sizeof(q->elements[0]) * size);
    q->mask = size - 1;
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
        return ENOMEM;
    atomic_store(&q->elements[tail], v);
    atomic_store(&q->tail, ((tail + 1) & q->mask));
    return 0;
}

void *
lqueue_poll(lqueue *q)
{
    unsigned long head = atomic_load(&q->head);
    unsigned long tail = atomic_load(&q->tail);
    unsigned long next_head;
    void *element;
    do {
        if (head == tail)
            return NULL;
        element = atomic_load(&q->elements[head]);
        next_head = (head + 1) & q->mask;
    } while (!atomic_compare_exchange_weak(&q->head, &head, next_head));
    return element;
}
