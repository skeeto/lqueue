#ifndef LQUEUE_H
#define LQUEUE_H

#include <stddef.h>

typedef struct lqueue lqueue;

lqueue *lqueue_create(unsigned max_size, size_t element_size);
void    lqueue_free(lqueue *);
int     lqueue_offer(lqueue *, void *);
int     lqueue_poll(lqueue *, void *);

#endif
