#include "conversation.h"
#include <unistd.h>
#include <string.h>


// === Search Functions ===

// Find and return the index of the particpant repressenting the currect process
int findParticipantIndex(Conversation* cnv_ptr){
    for (int i = 0; i < cnv_ptr->numParticipants; i++){
        if (cnv_ptr->participants[i].participantId == getpid()){
            return i;
        }
    }
    return -1;
}

// Find and return the index of the convertation with the given if it exists in the SharedMemoryGiven
int findConversationIndex(int cnv_id, SharedMemory* shm_ptr){
    for (int i = 0; i < shm_ptr->numConversations; ++i){
        if (shm_ptr->conversations[i].conversationId == cnv_id){
            return i;
        }
    }
    return -1;
}


// === Conversation Participation Functions ===

// Join the conversation with the given id, or create one if one does not exist
int joinConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr){
    lock(sem_id);

    int cnv_index = findConversationIndex(cnv_id, shm_ptr);
    // If no conversation with the given id exists, create one if possible
    if (cnv_index == -1){
        cnv_index = shm_ptr->numConversations;

        // If no more conversation can exist, return with failure, else initialise a new one
        if (cnv_index >= MAX_CONVERSATIONS){

            unlock(sem_id);
            return 1;
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
        return 1;
    }
    

    // The new participant will be located at the last spot (the unupdated numParticipants since the table participants[numParticipants] is zero indexed)
    int prp_index = cnv_ptr->numParticipants;

    // Initialise the values of the new participant
    cnv_ptr->participants[prp_index].participantId = getpid();

    // Update numParticipants
    cnv_ptr->numParticipants++;

    unlock(sem_id);
    return 0;
}

// Leave the conversation with the given id
int leaveConversation(int cnv_id, int sem_id, SharedMemory* shm_ptr){
    lock(sem_id);

    int cnv_index = findConversationIndex(cnv_id, shm_ptr);
    // If no conversation with the given id exists, return with failure
    if (cnv_index == -1){

        unlock(sem_id);
        return -1;
    }

    // Search for the current process in participants
    Conversation* cnv_ptr = &(shm_ptr->conversations[cnv_index]);
    int ptr_index = findParticipantIndex(cnv_ptr);

    cnv_ptr->participants[ptr_index].participantId = -1;

    // Check if conversation has no participants
    int cnv_removal_flag = 0;
    for (int i = 0 ; i < cnv_ptr->numParticipants; i++){
        if (cnv_ptr->participants[i].participantId != -1) cnv_removal_flag = 1;
    }
    // If conversation is empty, remove it
    if(cnv_removal_flag && (cnv_ptr->numMessages == 0)){
        // Shift left conversations
        for(int i = cnv_index; i < shm_ptr->numConversations - 1; i++){
            shm_ptr->conversations[i] = shm_ptr->conversations[i+1];
        }
        // Update numConversations
        shm_ptr->numConversations--;
    }

    unlock(sem_id);
    return ptr_index;
}

// Send a message to the conversation with the given id
int sendMessage(int cnv_id, int sem_id, SharedMemory* shm_ptr, const char* text){
    lock(sem_id);

    // Find the conversation where the message should be sent
    int cnv_index = findConversationIndex(cnv_id, shm_ptr);
    // If there isn't a conversation with that id return with failure
    // Shouldn't happen in conv_main
    if (cnv_index == -1){
        unlock(sem_id);
        return 1;
    }
    Conversation* cnv_ptr = &(shm_ptr->conversations[cnv_index]);

    // If no more messages can be sent, return with failure
    if (cnv_ptr->numMessages >= MAX_MESSAGES){
        unlock(sem_id);
        return 1;
    }
    // The new message will be located at the last spot (the unupdated numMessages since the table messages[numMessages] is zero indexed)
    int msg_index = cnv_ptr->numMessages;
    Message* msg_ptr = &(cnv_ptr->messages[msg_index]);
    
    // Initialise the values of the new message
    msg_ptr->messageId = msg_index;
    msg_ptr->senderId = getpid();
    strncpy(msg_ptr->text, text, MAX_MESSAGE_LENGTH - 1);
    msg_ptr->text[MAX_MESSAGE_LENGTH - 1] = '\0';
    // Pass through entire stucture to make unread
    for (int i=0; i < MAX_PARTICIPANTS; i++){
        msg_ptr->hasBeenRead[i] = 0;
    }
    
    // Update numMessages
    cnv_ptr->numMessages++;

    unlock(sem_id);
    return 0;
}


// === Message Removal Functions ===

// Returns if a message needs to be removed (has been read by everyone)
int messageRemovalCheck(Conversation* cnv_ptr, int msg_index){
    int removal_flag = 1;
    // Check if this messsage has been read by everyone
    for (int i = 0; i < cnv_ptr->numParticipants; ++i){
        if ((cnv_ptr->participants[i].participantId != -1)&&(cnv_ptr->messages[msg_index].hasBeenRead[i] == 0)){
            removal_flag = 0;
            break;
        } 
    }
    return removal_flag;
}

// Removes the message with the given index from the given conversation
void messageRemoval(Conversation* cnv_ptr, int msg_index){
    // Shift messages to the left and update numMessages
    for (int i = msg_index; i < cnv_ptr->numMessages - 1; i++){
         cnv_ptr->messages[i] = cnv_ptr->messages[i+1];
    }

    // Update numMessages, msg_index
    cnv_ptr->numMessages--;
}