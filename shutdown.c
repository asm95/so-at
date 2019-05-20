#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>

#include "msgQueue.h"

int main(int argc, char *argv[]){
    int msgqid, recebido;
    pid_packet p;

    msgqid = get_channel(MQ_SD);
    if(msgqid >= 0){
        printf("Shuting down the scheduler...\n");

        recebido = msgrcv(msgqid, &p, sizeof(pid_packet)-sizeof(long), 0x2, 0);
        if(recebido == 0)
            kill(p.pid, SIGINT);
        else
            printf("Error while receiving message...\n");
    } else {
        printf("Error on getting the message queue...\n");
    }

    exit(0);
}