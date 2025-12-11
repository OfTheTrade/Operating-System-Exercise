#include "chat.h"
#include "conversation.h"
#include <stdio.h>
#include <stdlib.h>

int sem_id_global;
int shm_id_global;
SharedMemory* shm_ptr_global;

int conversation_id_global;

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"Incorrect argument ammount! Should be called as:\n%s <conversation_id>\n", argv[0]);
        return 1;
    }
    conversation_id_global = atoi(argv[1]);

    cleanUp(sem_id_global,shm_id_global,shm_ptr_global);
    printf("FINISHED\n");
    return 0;
}