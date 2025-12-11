#include "chat.h"
#include "conversation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sem_id_global;
int shm_id_global;
SharedMemory* shm_ptr_global;

int cnv_id_global;
int cnv_index_global;
int prt_index_global;



int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"Incorrect argument ammount! Should be called as:\n%s <conversation_id>\n", argv[0]);
        return 1;
    }
    cnv_id_global = atoi(argv[1]);

     // Setup semaphore και shared memory
    setUpSemaphore(&sem_id_global);
    setUpSharedMemory(&shm_id_global, &shm_ptr_global);

    cnv_index_global = joinConversation(cnv_id_global, sem_id_global, shm_ptr_global);
    if(cnv_index_global == -1){
        fprintf(stderr,"Unable to join or create a conversation with the given id!\n");
    }

    printf("Succesfully joined conversation with id:(%d) Input messages to send below.\n", cnv_id_global);

    char buffer[MAX_MESSAGE_LENGTH];
    while (1){
        printf(">");
        if (!fgets(buffer, MAX_MESSAGE_LENGTH, stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;

        
        // If TERMINATE is read, leave the conversation, detach from memory and cleanup the semaphore
        // The shared memory remains untouched
        if (strcmp(buffer, "TERMINATE") == 0) {

            leaveConversation(cnv_id_global, sem_id_global, shm_ptr_global);
            cleanUpProcess(sem_id_global, shm_ptr_global);
            printf("Conversation terminated.\n");
            exit(0);
        // If TERMINATEFULL is read , the shared memory is also cleared
        }else if(strcmp(buffer, "TERMINATEFULL") == 0) {

            leaveConversation(cnv_id_global, sem_id_global, shm_ptr_global);
            cleanUpFull(sem_id_global, shm_id_global, shm_ptr_global);
            printf("Conversation terminated. Shared memory flushed.\n");
            exit(0);
        }
        sendMessage(cnv_id_global, sem_id_global, shm_ptr_global, buffer);
    }
    cleanUpProcess(sem_id_global,shm_ptr_global);
    printf("FINISHED\n");
    return 0;
}