#include <stdio.h>
#include <stdlib.h>
#include "msg/msg.h"
#include <unistd.h>
#include <string.h>

void msgsnd_error(){
    switch(errno){
        case EACCES:
        printf("The calling process does not have write permission on the message queue, and does not have the CAP_IPC_OWNER capability.\n");
        break;
        case EAGAIN:
        printf("The message can’t be sent due to the msg_qbytes limit for the queue and IPC_NOWAIT was specified in msgflg.\n");
        break;
        case EFAULT:
        printf("The address pointed to by msgp isn’t accessible.\n");
        break;
        case EIDRM:
        printf("The message queue was removed.\n");
        break;
        case EINTR:
        printf(" 	Sleeping on a full message queue condition, the process caught a signal.\n");
        break;
        case EINVAL:
        printf("Invalid msqid value, or non-positive mtype value, or invalid msgsz value (less than 0 or greater than the system value MSGMAX).\n");
        break;
        case ENOMEM:
        printf("The system does not have enough memory to make a copy of the message pointed to by msgp.\n");
        break;
        default:
        break;
    }
}

int main(int argc, char *argv[]){
    int msg_id = open_channel();
    char *program;

    if(msg_id >= 0){
        int status;
        msg_packet p;

        p.type = 0x1;
        strcpy(p.name, argv[1]);
        p.delay = atoi(argv[2]);

        status = msgsnd(msg_id, &p, sizeof(msg_packet) - sizeof(long), 0);
        if(status == 0)
            printf("Message successfully sent!\n");
        else{
            printf("Error while sending message...\n");
            perror(strerror(errno));
            // msgsnd_error();
        }
    } else {
        printf("Theres no channel open...\n");
    }

    sleep(1);

    exit(1);
}