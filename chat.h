#ifndef CHAT_H
#define CHAT_H

#include <sys/types.h>

#define MAX_PARTICIPANTS 10
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_CONVERSATIONS 10

typedef struct{
    pid_t participantId;
    int hasRead;
} Participant;

typedef struct{
    int conversationId;
    Participant participants[MAX_PARTICIPANTS];
    int numParticipants;
} Conversation;

// All that defines a message in shared memory
typedef struct{
    int messageId;
    pid_t senderId;
    char text[MAX_MESSAGE_LENGTH];
    int hasBeenRead[MAX_PARTICIPANTS];
} Message;

// All that will be stored in shared memory
typedef struct{
    Conversation conversations[MAX_CONVERSATIONS];
    Message messages[MAX_MESSAGES];
    int numConversations;
    int numMessages;
} SharedMemory;

void setUpSemaphore(int* sem_id);
void setUpSharedMemory(int* shm_id, SharedMemory** shm_ptr);
void cleanUp(int sem_id, int shm_id, SharedMemory* shm_ptr);
void lock(int sem_id);
void unlock(int sem_id);

#endif