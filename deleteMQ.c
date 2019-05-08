#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg/msg.h"

int main(int argc, char *argv[]){
    int mqid = open_channel();

    printf("%d\n", mqid);

    msgctl(mqid, IPC_RMID, NULL);

    exit(1);
}