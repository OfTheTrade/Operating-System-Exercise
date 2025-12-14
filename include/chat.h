#ifndef CHAT_H
#define CHAT_H

#include <sys/types.h>

#define MAX_PARTICIPANTS 10
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_CONVERSATIONS 10

#define SEM_KEY 0x1234
#define SHM_KEY 0x5678

// === Shared Memory Structs ===

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
int setUpSemaphore(int* sem_id);

// Set up shared memory
int setUpSharedMemory(int* shm_id, SharedMemory** shm_ptr);

// If this is the last process, clean up shared memory and semaphore
// Else just detach from shared memory
int cleanUp(int sem_id, int shm_id, SharedMemory* shm_ptr);


// == Semaphore Actions ===

// If semaphore is lost mid-execution
extern volatile int fatal_error;

// Use when entering critical section
int lock(int sem_id);

// Use when exiting critical section
int unlock(int sem_id);

#endif