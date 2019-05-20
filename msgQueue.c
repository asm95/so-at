#include "msgQueue.h"

int create_channel(int key){
    int msg_id = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    return msg_id;              // Returns the channel ID
}

int get_channel(int key){
    int msg_id = msgget(key, S_IWUSR | S_IRUSR);
    return msg_id;
}

int delete_channel(int msg_id){
    if(msg_id < 0){             // Channel doesn't exist
        return -1;
    }

    return msgctl(msg_id, IPC_RMID, NULL);
}