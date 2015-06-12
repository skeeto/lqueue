/**
 * C11 lock-free bounded queue. Supports one writer and multiple
 * readers. To simplify memory management queue users, data offered to
 * the queue are copied into the queue's buffers and copied back out
 * on retrieval.
 *
 * Queue functions return non-zero if the queue is full/empty.
 */
#ifndef LQUEUE_H
#define LQUEUE_H

#include <stddef.h>

typedef struct lqueue lqueue;

lqueue *lqueue_create(unsigned min_size, size_t element_size);
void    lqueue_free(lqueue *);
int     lqueue_offer(lqueue *, void *);
int     lqueue_poll(lqueue *, void *);

#endif
