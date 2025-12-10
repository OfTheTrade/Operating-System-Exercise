#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int sem_id;

struct sharedMemory{
    int flag;
};

void lock(){
    struct sembuf op = {0, -1, 0};
    if (semop(sem_id, &op, 1) == -1){
        exit(1);
    }
}

void unlock(){
    struct sembuf op = {0, 1, 0};
    if (semop(sem_id, &op, 1) == -1){
        exit(1);
    }
}

int main(){
    // Semaphore
    key_t sem_key = 0x1234;
    sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
    if (sem_id == -1){
        exit(1);
    }
    if (semctl(sem_id, 0, SETVAL, 1) == -1){
        exit(1);
    }

    // Shared memory
    key_t shm_key = 0x5678;
    int shm_id = shmget(shm_key, sizeof(struct sharedMemory), IPC_CREAT | 0666);
    if (shm_id == -1){
        exit(1);
    }
    struct sharedMemory *shm_ptr = (struct sharedMemory *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (struct sharedMemory *)-1){
        exit(1);
    }
    lock();
    printf("Inside critical section\n");
    shm_ptr->flag = 42;
    printf("(%d)\n", shm_ptr->flag);
    unlock();
    printf("Outside critical section\n");

    // Cleanup
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
}