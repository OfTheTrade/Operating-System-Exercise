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

// Used by the recieving thread to order the main thread to terminate 
volatile int termination_order = 0;

void* recieveMessages(void* arg){

    while(!termination_order){
        usleep(300000);
        lock(sem_id_global);

        // Find the current position of the conversation and participant struct in shared memory
        int cnv_index = findConversationIndex(cnv_id_global, shm_ptr_global);
        if (cnv_index == -1){
            termination_order = 1;
            perror("Conversation no longer exists");
            unlock(sem_id_global);
            break;
        }
        Conversation* cnv_ptr = &(shm_ptr_global->conversations[cnv_index]);

        int prt_index = findParticipantIndex(cnv_ptr);
        if (prt_index == -1){
            termination_order = 1;
            perror("Participant no longer exists");
            unlock(sem_id_global);
            break;
        }

        for (int i = 0; i < cnv_ptr->numMessages; ){
            // Will be used to see if a message shoud be removed (has been read by everyone)
            int msg_removal_flag = 0;

            Message* msg_ptr = &(cnv_ptr->messages[i]);
            
            if (msg_ptr->hasBeenRead[prt_index] == 0){
                printf("[PID %d]: %s\n", msg_ptr->senderId, msg_ptr->text);
                msg_ptr->hasBeenRead[prt_index]=1;
                // Check if the message has now been read by everyone
                // If it has, remove it, updating msg_ptr since the messages have now been shifted to the left
                msg_removal_flag = messageRemovalCheck(cnv_ptr, i);
            }
            // If TERMINATE is read, leave the conversation, detach from memory and cleanup the semaphore
            // The shared memory remains untouched
            if (strcmp(msg_ptr->text, "TERMINATE") == 0) {

                // Do not try to remove the "TERMINATE" string.
                // This allows processes that join after it is sent but before the conversation is flushed to not get stuck
                termination_order = 1;
 
                printf("Conversation pending termination / Please press <ENTER> ...\n");
                unlock(sem_id_global);
                break;
            }
            if (msg_removal_flag){
                messageRemoval(cnv_ptr, i);
            }else{
                i++;
            }
        }
        unlock(sem_id_global);
    }
    return NULL;

}

int main(int argc, char** argv){
    // Normal execution
    if (argc != 2){
        fprintf(stderr,"Incorrect argument ammount! Should be called as:\n%s <conversation_id>\n", argv[0]);
        return 1;
    }
    // Completely cleans shared memory and semaphore
    // Do not use while other processes are using that space 
    // For testing
    if (strcmp(argv[1], "FLUSH") == 0){
        setUpSemaphore(&sem_id_global);
        setUpSharedMemory(&shm_id_global, &shm_ptr_global);
        cleanUpFull(sem_id_global, shm_id_global, shm_ptr_global);
        printf("Flushing shared memory and semaphore!\n");
        return 0;
    }
    cnv_id_global = atoi(argv[1]);

     // Setup semaphores και shared memory
    setUpSemaphore(&sem_id_global);
    setUpSharedMemory(&shm_id_global, &shm_ptr_global);

    // Join the conversation
    int cnv_index = joinConversation(cnv_id_global, sem_id_global, shm_ptr_global);
    if(cnv_index == -1){
        fprintf(stderr,"Unable to join or create a conversation with the given id!\n");
    }

    printf("Succesfully joined conversation with id:(%d) Input messages to send below.\n", cnv_id_global);

    // Start receiving thread
    pthread_t rcv_thread;
    int thread_flag = pthread_create(&rcv_thread, NULL, recieveMessages, NULL);
    if (thread_flag != 0){
        cleanUpProcess(sem_id_global, shm_id_global, shm_ptr_global);
        fprintf(stderr,"Unable to obtain thread!\n");
        exit(1);
    }

    char buffer[MAX_MESSAGE_LENGTH];
    while (1){
        // Read the message and send if successful
        if (!fgets(buffer, MAX_MESSAGE_LENGTH, stdin)) break;
        printf("\033[1A");  
        printf("\033[2K");  

        // Check if TERMINATE has been read by the reading thread
        if (termination_order == 1){

            pthread_join(rcv_thread, NULL);
            leaveConversation(cnv_id_global, sem_id_global, shm_ptr_global);
            printf("Exited conversation.\n");

            if (cleanUpProcess(sem_id_global, shm_id_global, shm_ptr_global)) printf("Last proccess alive. Flushing shared memory.\n");
            return 0;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        // A new message is being output
        sendMessage(cnv_id_global, sem_id_global, shm_ptr_global, buffer);
        
    }
    // Should not happen
    fprintf(stderr,"This should never be reached.\n");
    return 1;
}