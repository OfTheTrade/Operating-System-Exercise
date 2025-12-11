#include "conversation.h"
#include <unistd.h>

// === Conversation Actions ===

// Find out if a convertation with the given id exists in the SharedMemoryGiven
int findConversation(int cnv_id, SharedMemory* shm_ptr){
    for (int i = 0; i < shm_ptr->numConversations; ++i){
        if (shm_ptr->conversations[i].conversationId == cnv_id){
            return i;
        }
    }
    return -1;
}

// Join the conversation with the given id, or create one if one does not exist
int joinConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr){
    lock(sem_id);

    int cnv_index = findConversation(cnv_id, shm_ptr);
    // If no conversation with the given id exists, create one if possible
    if (cnv_index == -1){
        cnv_index = shm_ptr->numConversations;

        // If no more conversation can exist, return with failure, else initialise a new one
        if (cnv_index >= MAX_CONVERSATIONS){

            unlock(sem_id);
            return -1;
        }
        shm_ptr->numConversations++;

        // Initialise the values of the new conversation
        shm_ptr->conversations[cnv_index].conversationId = cnv_id;
        shm_ptr->conversations[cnv_index].numMessages = 0;
        shm_ptr->conversations[cnv_index].numParticipants = 0;
    }
    Conversation* cnv_ptr = &(shm_ptr->conversations[cnv_index]);

    // If there more Participants cannot be added, return with failure
    if (cnv_ptr->numParticipants >= MAX_PARTICIPANTS){

        unlock(sem_id);
        return -1;
    }
    
    // The new participant will be located at the last spot (the unupdated numParticipants since the table participants[numParticipants] is zero indexed)
    int prp_index = cnv_ptr->numParticipants;

    // Initialise the values of the new participant
    cnv_ptr->participants[prp_index].participantId = getpid();
    cnv_ptr->participants[prp_index].hasRead = 0;

    // Update numParticipants
    cnv_ptr->numParticipants++;

    unlock(sem_id);
    return cnv_index;
}
void sendMessage(int cnv_id);