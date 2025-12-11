#include "chat.h"
#include <stdio.h>

int main(){
    int sem_id;
    setUpSemaphore(&sem_id);
    int shm_id;
    SharedMemory* sharedMemory;
    setUpSharedMemory(&shm_id,&sharedMemory);
    lock(sem_id);
    printf("ENTERED CRITICAL SECTION\n");
    sharedMemory->messages[0].text[0] = 's';
  
    printf("(%c)\n", sharedMemory->messages[0].text[0]);

    unlock(sem_id);
    printf("EXITED CRITICAL SECTION\n");

    cleanUp(sem_id,shm_id,sharedMemory);
    return 0;
}