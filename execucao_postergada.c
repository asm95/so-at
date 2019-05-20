#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/signal.h>

#include "msgQueue.h"

int main(int argc, char *argv[]){
    int msg_id = get_channel(MQ_SD);
    int status;
    msg_packet p;
    pid_packet ppkg;

    if(msg_id >= 0){
        msgrcv(msg_id, &ppkg, sizeof(pid_packet)-sizeof(long), 0x1, 0);
        kill(ppkg.pid, SIGUSR1);

        p.type = 0x1;
        strcpy(p.name, argv[1]);
        p.delay = atoi(argv[2]);

        status = msgsnd(msg_id, &p, sizeof(msg_packet)-sizeof(long), 0);
        if(status == 0){
            printf("Message successfully sent!\n");
            kill(ppkg.pid, SIGUSR2);
        }
        else{
            printf("Error while sending message...\n");
            exit(1);
        }
    } else {
        printf("Theres no open channel...\n");
        exit(2);
    }

    exit(0);
}