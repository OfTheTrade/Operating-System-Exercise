#include "chat.h"
#include "conversation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// These values, once set, will not change even when other processes run in parrarel
int sem_id_global;
int shm_id_global;
SharedMemory* shm_ptr_global;
int cnv_id_global;

void* recieveMessages(void* arg){

    while(1){
        usleep(300000);
        lock(sem_id_global);

        // Find the current position of the conversation and participant struct in shared memory
        int cnv_index = findConversationIndex(cnv_id_global, shm_ptr_global);
        if (cnv_index == -1){
            perror("Conversation no longer exists");
            exit(0);
        }
        Conversation* cnv_ptr = &(shm_ptr_global->conversations[cnv_index]);

        int prt_index = findParticipantIndex(cnv_ptr);
        if (prt_index == -1){
            perror("Participant no longer exists");
            exit(0);
        }

        for (int i = 0; i < cnv_ptr->numMessages; ++i){
            Message* msg_ptr = &(cnv_ptr->messages[i]);
            if (msg_ptr->hasBeenRead[prt_index] == 0){
                printf("[PID %d]: %s\n>", msg_ptr->senderId, msg_ptr->text);
                msg_ptr->hasBeenRead[prt_index]=1;
            }
            // If TERMINATE is read, leave the conversation, detach from memory and cleanup the semaphore
            // The shared memory remains untouched
            if (strcmp(msg_ptr->text, "TERMINATE") == 0) {
                
                unlock(sem_id_global);
                leaveConversation(cnv_id_global, sem_id_global, shm_ptr_global);
                cleanUpProcess(sem_id_global, shm_ptr_global);
 
                printf("Conversation terminated.\n");
                exit(0);
            }
        
        }
        unlock(sem_id_global);
    }

}

int main(int argc, char** argv){
    // Completely cleans shared memory and semaphore
    // Do not use while other processes are using that space
    if ((argc == 3)&&(strcmp(argv[2], "FLUSH") == 0)){
        cnv_id_global = atoi(argv[1]);
        setUpSemaphore(&sem_id_global);
        setUpSharedMemory(&shm_id_global, &shm_ptr_global);
        cleanUpFull(sem_id_global,shm_id_global,shm_ptr_global);
        return 0;
    // Normal execution
    }else if (argc != 2){
        fprintf(stderr,"Incorrect argument ammount! Should be called as:\n%s <conversation_id>\n", argv[0]);
        return 1;
    }
    cnv_id_global = atoi(argv[1]);

     // Setup semaphore και shared memory
    setUpSemaphore(&sem_id_global);
    setUpSharedMemory(&shm_id_global, &shm_ptr_global);

    int cnv_index = joinConversation(cnv_id_global, sem_id_global, shm_ptr_global);
    if(cnv_index == -1){
        fprintf(stderr,"Unable to join or create a conversation with the given id!\n");
    }

    printf("Succesfully joined conversation with id:(%d) Input messages to send below.\n", cnv_id_global);

    // Start receiving thread
    pthread_t rcv_thread;
    pthread_create(&rcv_thread, NULL, recieveMessages, NULL);
    pthread_detach(rcv_thread);

    char buffer[MAX_MESSAGE_LENGTH];
    while (1){
        printf(">");
        // Read the message and send if successful
        if (!fgets(buffer, MAX_MESSAGE_LENGTH, stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;

        sendMessage(cnv_id_global, sem_id_global, shm_ptr_global, buffer);

        // If TERMINATE is read, leave the conversation, detach from memory and cleanup the semaphore
        // The shared memory remains untouched
        if (strcmp(buffer, "TERMINATE") == 0) {

            leaveConversation(cnv_id_global, sem_id_global, shm_ptr_global);
            cleanUpProcess(sem_id_global, shm_ptr_global);
            printf("Conversation terminated.\n");
            exit(0);
        }
        
    }
    cleanUpProcess(sem_id_global,shm_ptr_global);
    printf("FINISHED\n");
    return 0;
}