#include "chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>


// === Semaphore/Shared Memory Setup Functions ===

// Set up semaphore
int setUpSemaphore(int* sem_id){
    key_t sem_key = SEM_KEY;
    // Semaphore initialisation
    int sem_id_buffer = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id_buffer == -1){
        // Semaphore already exists

        // Check if semget instead failed due to actual error (without IPC_EXCL this time)
        sem_id_buffer = semget(sem_key, 1, IPC_CREAT | 0666);
        if (sem_id_buffer == -1){
            // Failed to initialise semaphore
            return 1;
        }
    }else{
        // Semaphore didn't exist before, give it the correct value
        if (semctl(sem_id_buffer, 0, SETVAL, 1) == -1){
            // Failed to set up semaphore
            return 1;
        }
    }
    // Return value
    *(sem_id) = sem_id_buffer;
    return 0;
}

// Set up shared memory
int setUpSharedMemory(int* shm_id, SharedMemory** shm_ptr){
    key_t shm_key = SHM_KEY;
    // Initialise shared memory
    int shm_id_buffer = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id_buffer == -1){
        // Failed to aquire shared memory
        return 1;
    }

    // Starting values for the struct
    struct shmid_ds shm_stat;
    if (shmctl(shm_id_buffer, IPC_STAT, &shm_stat) == -1) {
        // Failed to determine if shared memory is already initiated
        return 1;
    }

    // If this is the first time, initiate the values of the shared memory
    int is_first_time = 0;
    if (shm_stat.shm_nattch == 0) {
        is_first_time = 1;
    }

    // Attach to shared memory
    SharedMemory *shm_ptr_buffer = (SharedMemory *)shmat(shm_id_buffer, NULL, 0);
    if (shm_ptr_buffer == (SharedMemory *)-1){
        // Failed to attach to shared memory
        return 1;
    }
    
    if (is_first_time){
        shm_ptr_buffer->numConversations = 0;
    }

    // Return values
    *(shm_id) = shm_id_buffer;
    *(shm_ptr) = shm_ptr_buffer;
    return 0;
}

// If this is the last process, clean up shared memory and semaphore
// Else just detach from shared memory
int cleanUp(int sem_id, int shm_id, SharedMemory* shm_ptr){

    struct shmid_ds shm_stat;
    shmctl(shm_id, IPC_STAT, &shm_stat);
    if (shm_stat.shm_nattch <= 1){
        // Detach
        shmdt(shm_ptr); 
        // Remove shared memory           
        shmctl(shm_id, IPC_RMID, NULL); 
        // Remove semaphore (if given one)
        if (sem_id >= 0) semctl(sem_id, 0, IPC_RMID);  

        return 1;
    }else{ 
        // Just detach
        shmdt(shm_ptr);

        return 0; 
    }
}


// == Semaphore Actions ===

// Use when entering critical section
int lock(int sem_id){
    struct sembuf op = {0, -1, 0};
    if (semop(sem_id, &op, 1) == -1){
        // Failed to lock semaphore
        return 1;
    }
    return 0;
}

// Use when exiting critical section
int unlock(int sem_id){
    struct sembuf op = {0, 1, 0};
    if (semop(sem_id, &op, 1) == -1){
        // Failed to unlock semaphore
        return 1;
    }
    return 0;
}