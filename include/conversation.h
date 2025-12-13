#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "chat.h"

// === Conversation Actions ===

// Find and return the index of the particpant repressenting the currect process
int findParticipantIndex(Conversation* cnv_ptr);

// Find and return the index of the convertation with the given if it exists in the SharedMemoryGiven
int findConversationIndex(int cnv_id, SharedMemory* shm_ptr);

// Join the conversation with the given id, or create one if one does not exist
int joinConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr);

// Leave the conversation with the given id
int leaveConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr);

// Send a message to the conversation with the given id
int sendMessage(int cnv_id, int sem_id, SharedMemory* shm_ptr, const char* text);

// Returns if a message needs to be removed (has been read by everyone)
int messageRemovalCheck(Conversation* cnv_ptr, int msg_index);

// Removes the message with the given index from the given conversation
void messageRemoval(Conversation* cnv_ptr, int msg_index);

#endif