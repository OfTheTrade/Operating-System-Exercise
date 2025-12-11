#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "chat.h"

// === Conversation Actions ===

// Find and return the index of a convertation with the given id in SharedMemoryGiven
int findConversationIndex(int cnv_id, SharedMemory* shm_ptr);

// Join the conversation with the given id, or create one if one does not exist
int joinConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr);

// Leave the conversation with the given id
int leaveConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr);

// Send a message to the conversation with the given id
int sendMessage(int cnv_id, int sem_id, SharedMemory* shm_ptr, const char* text);

#endif