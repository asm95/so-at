#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h> // for access syscall

#include "msg/msg.h"

#define APP_NAME "at"

void cli_main(int delay, char *prog_name){
    int oid;

    oid = open_channel();
    if (oid <= 0){
        printf("(E) Could not communicate with scheduler. It is running?\n");
        return;
    }

    msg_packet p;
    p.ac = AC_NP;
    p.type = 0x1; // send to master node 
    p.delay = delay;
    p.req_t = time(NULL);
    strcpy(p.prog_name, prog_name);
    send_msg(oid, &p);
}

int cli_check_prog_name(char *prog_name){
    size_t name_len = strlen(prog_name);
    if (name_len > MAX_PROG_NAME-1){
        printf("(E) Invalid <prog_name> (%s). Max length is %d\n", prog_name, MAX_PROG_NAME-1);
        return 0;
    }
    if( access( prog_name, R_OK|X_OK ) != -1 ) {
        return 1;
    }
    printf("(E) Invalid <prog_name> (%s). Make sure it exists and you have 'r-x' permissions\n", prog_name);
    return 0;
}

int cli_process_args(int argc, char *argv[], int *usr_delay, char **prog_name){
    if (argc < 3){
        printf("(W) Usage: %s <delay> <prog_name>\n", APP_NAME);
        return -1;
    }

    int delay = atoi(argv[1]);
    if (delay == 0){
        printf("(E) <delay> should be grater than zero.\n");
        return -2;
    }

    if (! cli_check_prog_name(argv[2])){
        return -2;
    }

    *usr_delay = delay;
    *prog_name = argv[2];
}

int main (int argc, char *argv[]){
    int sts;
    
    int delay; char *prog_name;
    sts = cli_process_args(argc, argv, &delay, &prog_name);

    switch(sts){
        case -1:
            exit(0); break;
        case -2:
            printf("(I) Parameters are invalid. Please consult the manual.\n");
            exit(1); break;
        default: break;
    }

    cli_main(delay, prog_name);

    return 0;
}