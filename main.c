#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sha1.h"
#include "wqueue.h"

#define ITERATIONS (1024 * 64)

static void
printer(int id, void *arg)
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
    char line[SHA1_DIGEST_SIZE * 2 + 1];
    for (int i = 0; i < SHA1_DIGEST_SIZE; i++)
        sprintf(line + i * 2, "%02x", digest[i]);
    printf("%s: %s (thread %d)\n", (char *)arg, line, id);
}

int
main(void)
{
    wqueue *queue = wqueue_create(6, sysconf(_SC_NPROCESSORS_ONLN));
    struct {
        char msg[64];
    } jobs[400];
    for (int i = 0; i < 400; i++) {
        sprintf(jobs[i].msg, "hello%06d", i);
        wqueue_add(queue, printer, &jobs[i].msg);
    }
    wqueue_wait(queue);
    wqueue_free(queue);
    return 0;
}
