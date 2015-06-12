/**
 * Work queue demo using SHA-1 whitening.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "wqueue.h"
#include "sha1.h"

#define ITERATIONS (1024 * 256)

#define countof(a) (sizeof(a) / sizeof(0[a]))

static void
sha1_worker(int id, void *arg)
{
    uint8_t digest[SHA1_DIGEST_SIZE];
    SHA1_CTX context;
    SHA1_Init(&context);
    SHA1_Update(&context, arg, strlen(arg));
    SHA1_Final(&context, digest);
    for (int i = 0; i < ITERATIONS - 1; i++) {
        SHA1_Init(&context);
        SHA1_Update(&context, digest, SHA1_DIGEST_SIZE);
        SHA1_Final(&context, digest);
    }
    char hash[SHA1_DIGEST_SIZE * 2 + 1];
    for (int i = 0; i < SHA1_DIGEST_SIZE; i++)
        sprintf(hash + i * 2, "%02x", digest[i]);
    char line[128];
    int n = sprintf(line, "%s: %s (thread %d)\n", (char *)arg, hash, id);
    fwrite(line, n, 1, stdout);
}

int
main(void)
{
    wqueue *queue = wqueue_create(50, sysconf(_SC_NPROCESSORS_ONLN));
    struct {
        char msg[64];
    } jobs[400];
    for (unsigned i = 0; i < countof(jobs); i++) {
        sprintf(jobs[i].msg, "hello%06d", i);
        wqueue_add(queue, sha1_worker, &jobs[i].msg);
    }
    fprintf(stderr, "\x1b[91;1mWaiting ...\n\x1b[0m");
    wqueue_wait(queue);
    wqueue_free(queue);
    return 0;
}
