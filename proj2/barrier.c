#include "barrier.h"

void b_join(barrier_t *barrier) {
    sem_wait(barrier->mutex);
    barrier->shm[0]++;
    if (*barrier->shm == barrier->size) {
        for (int i = 0; i < barrier->size-1; i++) {
            sem_post(barrier->sem);
        }
        barrier->shm[0] = 0;
        sem_post(barrier->mutex);
    } else {
        sem_post(barrier->mutex);
        sem_wait(barrier->sem);
    }
}

void b_open(barrier_t *barrier, int size) {
    barrier->sem = sem_open(semBARRIER, O_RDWR);
    barrier->mutex = sem_open(semBARRIERMUTEX, O_RDWR);
    barrier->size = size;

    int shmID;
    shmID = shm_open(shmBARRIER, O_RDWR, S_IRUSR | S_IWUSR);
    barrier->shm = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
}

void b_close(barrier_t *barrier)  {
    sem_close(barrier->sem);
    sem_close(barrier->mutex);
    munmap(barrier->shm, sizeof(int));
}

void b_init(barrier_t *barrier) {
    int shmID;
    shmID = shm_open(shmBARRIER, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, sizeof(int));
    barrier->shm = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    barrier->shm[0] = 0;
    munmap(barrier->shm, sizeof(int));

    barrier->mutex = sem_open(semBARRIERMUTEX, O_CREAT, 0600, 1);
    barrier->sem = sem_open(semBARRIER, O_CREAT, 0600, 0);
    sem_close(barrier->mutex);
    sem_close(barrier->sem);
}

void b_unlink()  {
    shm_unlink(shmBARRIER);
    sem_unlink(semBARRIERMUTEX);
    sem_unlink(semBARRIER);
}

