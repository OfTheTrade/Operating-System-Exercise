#include "chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SEM_KEY 0x1234
#define SHM_KEY 0x5678


// === Semaphore/Shared Memory Setup Functions ===

// Set up semaphore
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

// Set up shared memory
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
    struct shmid_ds shm_stat;
    if (shmctl(shm_id_buffer, IPC_STAT, &shm_stat) == -1) {
        perror("Failed to determine if shared memory is already initiated");
        exit(1);
    }

    // If this is the first time, initiate the values of the shared memory
    if (shm_stat.shm_nattch == 0) {
        shm_ptr_buffer->numConversations = 0;
    }

    // Return values
    *(shm_id) = shm_id_buffer;
    *(shm_ptr) = shm_ptr_buffer;
}

// Cleanup semaphore and shared memory
void cleanUpFull(int sem_id, int shm_id, SharedMemory* shm_ptr){
    // Detach
    shmdt(shm_ptr); 
    // Remove shared memory           
    shmctl(shm_id, IPC_RMID, NULL); 
    // Remove semaphore
    semctl(sem_id, 0, IPC_RMID);  
}

// Cleanup semaphore and detach from shared memory
void cleanUpProcess(int sem_id, SharedMemory* shm_ptr){
    // Detach
    shmdt(shm_ptr); 
    // Remove semaphore
    semctl(sem_id, 0, IPC_RMID);  
}

// == Semaphore Actions ===

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