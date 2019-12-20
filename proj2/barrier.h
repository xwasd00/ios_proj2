#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>

#define shmSIZE sizeof(int)
#define shmBARRIER "/xsovam00-shmBARRIER"
#define semBARRIERMUTEX "/xsovam00-semBARRIERMUTEX"
#define semBARRIER "/xsovam00-semBARRIER"

//barrier
typedef struct {
    sem_t *sem;
    sem_t *mutex;
    int *shm;
    int size;
} barrier_t;

void b_join(barrier_t *barrier);

void b_open(barrier_t *barrier, int size);

void b_close(barrier_t *barrier);

void b_init(barrier_t *barrier);

void b_unlink();
