#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include <sys/stat.h> // includes permission bits

#include <string.h>

#include "common.h"
#include "msg.h"


int open_channel(){
    int msg_id;

    msg_id = msgget(MQ_ID, S_IWUSR);
    return msg_id;
}

int create_channel(){
    int msg_id;

    msg_id = msgget(MQ_ID, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (msg_id == -1){
        if (errno == EEXIST){
            return -2;
        }
    }
    return msg_id;
}

int delete_channel(int msg_id){
    if (msg_id < 0){
        return -1;
    }
    return msgctl(msg_id, IPC_RMID, NULL);
}

int send_packet(int msg_id, uint delay){
    msg_packet p;
    p.type = 0x1;
    p.delay = delay;
    strcpy(p.prog_name, "hello_world");

    int status;
    status = msgsnd(msg_id, &p, sizeof(msg_packet), 0);
    return status;
}

int recv_packet(int msg_id, msg_packet *p){
    return msgrcv(msg_id, p, sizeof(msg_packet), 0x1, 0);
}