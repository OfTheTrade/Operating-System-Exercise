#include "chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>


int setUpSemaphore(){
    key_t sem_key = 0x1234;
    // Semaphore initialisation
    int sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
    if (sem_id == -1){
        perror("Failed to initialise semaphore");
        exit(1);
    }
    // Give it the correct value
    if (semctl(sem_id, 0, SETVAL, 1) == -1){
        perror("Failed to set up semaphore");
        exit(1);
    }
    return sem_id;
}

SharedMemory* setUpSharedMemory(){
    key_t shm_key = 0x5678;
    // Initialise shared memory
    int shm_id = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id == -1){
        perror("Failed to aquire shared memory");
        exit(1);
    }
    // Mold shared memory according to the used struct
    SharedMemory *shm_ptr = (SharedMemory *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (SharedMemory *)-1){
        perror("Failed to attach to shared memory");
        exit(1);
    }
    return shm_ptr;
}

// Use when entering critical section
void lock(int sem_id){
    struct sembuf op = {0, -1, 0};
    if (semop(sem_id, &op, 1) == -1){
        perror("Failed to lock semaphore");
        exit(1);
    }
}

// Use when exiting critical section
void unlock(int sem_id){
    struct sembuf op = {0, 1, 0};
    if (semop(sem_id, &op, 1) == -1){
        perror("Failed to unlock semaphore");
        exit(1);
    }
}

int main(){
    int semaphore = setUpSemaphore();
    SharedMemory* sharedMemory = setUpSharedMemory();
    lock(semaphore);
    printf("ENTERED CRITICAL SECTION\n");
    sharedMemory->messages->text[0] = 's';
  
    printf("(%c)\n", sharedMemory->messages->text[0]);

    unlock(semaphore);
    printf("EXITED CRITICAL SECTION\n");
    return 0;
}
