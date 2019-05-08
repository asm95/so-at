#include "msg.h"

int create_channel(){
    int msg_id = msgget(MQ_ID, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    printf("%d\n", msg_id);

    if(msg_id == -1){                   // Fails to create the channel
        if(errno == EACCES){
            printf("A message queue exists for key, but the calling process does not have permission to access the queue, and does not have the CAP_IPC_OWNER capability.\n");
            return -1;
        } else if(errno == EEXIST){     // Channel already exists
            printf("A message queue exists for key and msgflg specified both IPC_CREAT and IPC_EXCL.\n");
            return -2;
        } else if(errno == ENOENT){
            printf("No message queue exists for key and msgflg did not specify IPC_CREAT.\n");
            return -3;
        } else if(errno == ENOMEM){
            printf("A message queue has to be created but the system does not have enough memory for the new data structure.\n");
            return -4;
        } else {
            printf("A message queue has to be created but the system limit for the maximum number of message queues (MSGMNI) would be exceeded.\n");
            return -5;
        }
    }
    
    return msg_id;              // Returns the channel ID
}

int open_channel(){
    int msg_id = msgget(MQ_ID, S_IWUSR);

    return msg_id;
}

int delete_channel(int msg_id){
    if(msg_id < 0){             // Channel doesn't exist
        return -1;
    }

    return msgctl(msg_id, IPC_RMID, NULL);
}