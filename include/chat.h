#ifndef CHAT_H
#define CHAT_H

#include <sys/types.h>

#define MAX_PARTICIPANTS 10
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_CONVERSATIONS 10

// All that defines a participant in a conversation
typedef struct{
    pid_t participantId;
} Participant;

// All that defines a message in a conversation
typedef struct{
    int messageId;
    pid_t senderId;
    char text[MAX_MESSAGE_LENGTH];
    int hasBeenRead[MAX_PARTICIPANTS];
} Message;

// All that defines a conversation in shared memory
typedef struct{
    int conversationId;
    Participant participants[MAX_PARTICIPANTS];
    int numParticipants;
    Message messages[MAX_MESSAGES];
    int numMessages;
} Conversation;

// All that will be stored in shared memory
typedef struct{
    Conversation conversations[MAX_CONVERSATIONS];
    int numConversations;
} SharedMemory;


// === Semaphore/Shared Memory Setup Functions ===

// Set up semaphore
void setUpSemaphore(int* sem_id);

// Set up shared memory
void setUpSharedMemory(int* shm_id, SharedMemory** shm_ptr);

// Cleanup semaphore and shared memory
void cleanUpFull(int sem_id, int shm_id, SharedMemory* shm_ptr);

// Detach from shared memory
void cleanUpProcess(int sem_id, SharedMemory* shm_ptr);

// == Semaphore Actions ===

// Use when entering critical section
void lock(int sem_id);

// Use when exiting critical section
void unlock(int sem_id);

#endif