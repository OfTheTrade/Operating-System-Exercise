#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "chat.h"

// === Conversation Actions ===
int findConversation(int cnv_id, SharedMemory* shm_ptr);
int joinConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr);
void sendMessage(int cnv_id);

#endif