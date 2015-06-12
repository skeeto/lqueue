# C11 + Pthreads Atomic Bounded Work Queue

This is a small library providing a single-writer, multiple-reader
lock-free queue (lqueue) using C11's `stdatomic.h` features. This
queue wrapped in a multi-threaded work queue (wqueue) using POSIX
threads and semaphores. Jobs in the form of function pointers /
argument tuples are submitted to the queue and completed by the
queue's threads, blocking as necessary when the queue is full.

~~~c
struct sha1_job {
    char message[64];
    char hash[SHA1_DIGEST_SIZE * 2 + 1];
};

static void
sha1_worker(int thread_id, void *arg)
{
    /* ... compute hashes ... */
}

void
sha1_compute_all(void)
{
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    wqueue *queue = wqueue_create(50, nthreads);
    struct sha1_job jobs[400];
    for (unsigned i = 0; i < countof(jobs); i++) {
        sprintf(jobs[i].message, "hello%06d", i);
        wqueue_add(queue, sha1_worker, &jobs[i]);
    }
    wqueue_wait(queue); // wait for full job completion
    /* ... process results and/or queue more jobs ... */
    wqueue_free(queue);
}
~~~
