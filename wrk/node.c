#include <stdio.h> // for printf
#include <string.h> // for strncmp
#include <stdlib.h> // for exit

#include <signal.h> // for signal, SIGINT
#include <unistd.h> // for getpid, usleep
#include <sys/ipc.h> // for IPC_NOWAIT
#include <sys/wait.h> // for wait, WNOHANG
#include <sys/msg.h> // for msgsnd, msgrcv

#include "../msg/msg.h" // for msg_packet
#include "node.h"

// private global variable that manages child state
static int g_child_state = 0;
static int g_real_pid = -1;
static int g_do_exit = 0;
static int g_channel_id = -1;
static int g_pid_id = -1;

int get_real_pid(){
    if (g_real_pid == -1){
        // have you ever heard about the singleton pattern?
        // issue #1: we should be using pit_t from sys/types.h, but yeah, just an
        g_real_pid = getpid();
    }
    return g_real_pid;
}

int terminate_child(int term_status){
    g_do_exit = term_status;
}

void exec_program_mock(){
    int pid = fork();
    if (pid == 0){
        // child will sleep for a while
        int sleep_time = 3;
        printf("(I) Hello! I'm sleeping for %d seconds\n", sleep_time);
        sleep(sleep_time);
        exit(0);
    } else {
        // node will now listen to the queue and wait for it's child termination
        g_channel_id = 1;
    }
}

void exec_program_from_file(msg_packet *p){
    int pid = fork();
    if (pid == 0){
        // child will replace it's in memory image
        char prog_name_buffer[MAX_PROG_NAME + 2];
        // append './' to the name of the program is important
        // because the shell like behaviour of the execlp will search
        // in the current directory
        if (strncmp(p->prog_name, "./", 2) != 0 && p->prog_name[0] != '/'){
            sprintf(prog_name_buffer, "./%s", p->prog_name);
        }
        execlp(prog_name_buffer, p->prog_name, (char *)NULL);
        printf("(C%2d) Could not execute '%s'\n", g_pid_id, p->prog_name);
        exit(0); // case in error
    }
}

void exec_program(msg_packet *p){
    // exec_program_mock()
    exec_program_from_file(p);
    // node will now listen to the queue and wait for it's child termination
    g_child_state = 1;
}

void process_packet(msg_packet *p, to_proxy *topo){
    int next_node = route_walk(p);
    if (next_node >= 0){
        printf("(C%2d) Packet is for %d\n", g_pid_id, next_node);
        msgsnd(g_channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
    } else
    if (next_node == -1){
        printf("(C%2d) Packet is for me!\n", g_pid_id);
        if (p->ac == AC_SP){
            printf("(C%2d) Message for spawn program named '%s'\n", g_pid_id, p->prog_name);
            exec_program(p);
        }
    }
}

void child_state_wait_msg(to_proxy *topo){
    int rcv_ok;
    msg_packet p;

    while(! g_do_exit && g_child_state == 0){
        printf("(C%2d) Listining to slot 0x%02x on channel %d...\n", g_pid_id, g_pid_id+1, g_channel_id);
        rcv_ok = msgrcv(g_channel_id, &p, sizeof(msg_packet) - sizeof(long), g_pid_id+1, 0); // block
        if (rcv_ok > 0){
            process_packet(&p, topo);
        }
    }
}

void send_prog_finished(msg_packet *p, to_proxy *topo){
    int next_node;

    // search for route
    p->routing_idx = topology_search(topo, 0x0, p->routing_path, 16); // search for master
    p->ac = AC_FP;
    p->pid_id = g_pid_id;
    next_node = route_walk(p); // check which node should I send first
    printf("(C%2d) Sending AC_FP to master via %d\n", g_pid_id, next_node);
    msgsnd(g_channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
}

void child_state_prog_running(to_proxy *topo){
    int rcv_ok = -1;
    msg_packet p;
    const int sleep_intl = 100 * 1000; // in miliseconds

    while (! g_do_exit){
        rcv_ok = msgrcv(g_channel_id, &p, sizeof(msg_packet) - sizeof(long), g_pid_id+1, IPC_NOWAIT | 0); // do not block
        if (rcv_ok > 0){
            process_packet(&p, topo);
        }
        usleep(sleep_intl);
        waitpid(-1, &rcv_ok, WNOHANG | 0);
        if (rcv_ok == 0){
            send_prog_finished(&p, topo);
            g_child_state = 0; break;
        }
    }
}

void child_manager(to_proxy *topo, int pid_id){
    g_pid_id = pid_id;

    printf("(C%2d) Initiated process (PID: %d)\n", pid_id, get_real_pid());
    topology_init(topo, pid_id);

    int rcv_ok = -1;
    g_channel_id = open_channel();
    while (! g_do_exit){
        switch(g_child_state){
            case 1:
                child_state_prog_running(topo); break;
            case 0:
            default:
                child_state_wait_msg(topo); break;
        }
    }
}