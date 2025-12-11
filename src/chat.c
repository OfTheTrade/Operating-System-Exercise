#include "chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SEM_KEY 0x1234
#define SHM_KEY 0x5678

void setUpSemaphore(int* sem_id){
    key_t sem_key = SEM_KEY;
    // Semaphore initialisation
    int sem_id_buffer = semget(sem_key, 1, IPC_CREAT | 0666);
    if (sem_id_buffer == -1){
        perror("Failed to initialise semaphore");
        exit(1);
    }
    // Give it the correct value
    if (semctl(sem_id_buffer, 0, SETVAL, 1) == -1){
        perror("Failed to set up semaphore");
        exit(1);
    }
    // Return value
    *(sem_id) = sem_id_buffer;
}
void setUpSharedMemory(int* shm_id, SharedMemory** shm_ptr){
    key_t shm_key = SHM_KEY;
    // Initialise shared memory
    int shm_id_buffer = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id_buffer == -1){
        perror("Failed to aquire shared memory");
        exit(1);
    }
    // Mold shared memory according to the used struct
    SharedMemory *shm_ptr_buffer = (SharedMemory *)shmat(shm_id_buffer, NULL, 0);
    if (shm_ptr_buffer == (SharedMemory *)-1){
        perror("Failed to attach to shared memory");
        exit(1);
    }
    // Starting values for the struct
    shm_ptr_buffer->numConversations = 0;
    shm_ptr_buffer->numMessages = 0;

    // Return values
    *(shm_id) = shm_id_buffer;
    *(shm_ptr) = shm_ptr_buffer;
}

void cleanUp(int sem_id, int shm_id, SharedMemory* shm_ptr){
    // Detach
    shmdt(shm_ptr); 
    // Remove shared memory           
    shmctl(shm_id, IPC_RMID, NULL); 
    // Remove semaphore
    semctl(sem_id, 0, IPC_RMID);  
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
