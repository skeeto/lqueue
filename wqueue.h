#ifndef WQUEUE_H
#define WQUEUE_H

typedef struct wqueue wqueue;

wqueue *wqueue_create(int exponent, int nthreads);
void    wqueue_free(wqueue *);
void    wqueue_add(wqueue *, void (*)(int, void *), void *);
void    wqueue_wait(wqueue *);

#endif
