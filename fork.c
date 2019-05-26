#include <stdio.h>
#include <stdlib.h> // for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "common.h"
#include "globals.h"

#include "topology.h"
#include "msg/msg.h"
#include "sch/jobs.h"
#include "sch/master.h"


void print_path(int *arr, int sz){
    for(int i=0; i<sz; i++){
        printf("%X", arr[i]);
        if (i < sz-1){
            printf(" -> ");
        }
    }printf("\n");
}

// it's a global variable because signal handlers doesn't provide any parameters
int pid_vec[NRO_PROC];
int pid_id, real_pid, channel_id, do_exit = 0;
int job_id = 0;
int child_state = 0;
int g_exec_ord = 0;

job_node *job_l, *job_done_l = NULL;


void exit_handler_parent(int sig_id){
    // will shutdown all programs and print the report
    shutdown(pid_vec);
    delete_channel(channel_id);
    delete_jl(job_l);
    do_exit = 1;
}

void exit_handler_child(int sig_id){
    printf("(C%2d) Exiting child\n", pid_id);
    do_exit = 1;
}


void route_print(msg_packet *p){
    int idx = p->routing_idx;
    for(; idx < 16; idx++){
        printf("%d ", p->routing_path[idx]);
    }
    printf("\n");
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
        child_state = 1;
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
        printf("(C%2d) Could not execute '%s'\n", pid_id, p->prog_name);
        exit(0); // case in error
    }
}

void exec_program(msg_packet *p){
    // exec_program_mock()
    exec_program_from_file(p);
    // node will now listen to the queue and wait for it's child termination
    child_state = 1;
}

void send_prog_finished(msg_packet *p, to_proxy *topo){
    int next_node;

    // search for route
    p->routing_idx = topology_search(topo, 0x0, p->routing_path, 16); // search for master
    p->ac = AC_FP;
    p->pid_id = pid_id;
    next_node = route_walk(p); // check which node should I send first
    printf("(C%2d) Sending AC_FP to master via %d\n", pid_id, next_node);
    msgsnd(channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
}

void process_packet(msg_packet *p, to_proxy *topo){
    int next_node = route_walk(p);
    if (next_node >= 0){
        printf("(C%2d) Packet is for %d\n", pid_id, next_node);
        msgsnd(channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
    } else
    if (next_node == -1){
        printf("(C%2d) Packet is for me!\n", pid_id);
        if (p->ac == AC_SP){
            printf("(C%2d) Message for spawn program named '%s'\n", pid_id, p->prog_name);
            exec_program(p);
        }
    }
}


int finished_c = -2;


void child_state_wait_msg(to_proxy *topo){
    int rcv_ok;
    msg_packet p;

    while(! do_exit && child_state == 0){
        printf("(C%2d) Listining to slot 0x%02x on channel %d...\n", pid_id, pid_id+1, channel_id);
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), pid_id+1, 0); // block
        if (rcv_ok > 0){
            process_packet(&p, topo);
        }
    }
}

void child_state_prog_running(to_proxy *topo){
    int rcv_ok = -1;
    msg_packet p;
    const int sleep_intl = 100 * 1000; // in miliseconds

    while (! do_exit){
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), pid_id+1, IPC_NOWAIT | 0); // do not block
        if (rcv_ok > 0){
            process_packet(&p, topo);
        }
        usleep(sleep_intl);
        waitpid(-1, &rcv_ok, WNOHANG | 0);
        if (rcv_ok == 0){
            send_prog_finished(&p, topo);
            child_state = 0; break;
        }
    }
}

void child_manager(to_proxy *topo){
    printf("(C%2d) Initiated process (PID: %d)\n", pid_id, real_pid);
    topology_init(topo, pid_id);

    int rcv_ok = -1;
    channel_id = open_channel();
    signal(SIGINT, exit_handler_child);
    while (! do_exit){
        switch(child_state){
            case 1:
                child_state_prog_running(topo); break;
            case 0:
            default:
                child_state_wait_msg(topo); break;
        }
    }
}

void close_channel(){
    int oid = open_channel();
    int sts = delete_channel(oid);
    if (sts == 0){
        printf("(%3s) Removed channel %d sucessfully!\n", "I", oid);
    }
}

void bootstrap_app(to_types scheduler_topo){
    printf("(I) Gerenciador de Processos\n");
    printf("(I) Número de processos a serem criados: %d\n", NRO_PROC-1);

    to_proxy *tp = topology_create(scheduler_topo);
    channel_id = create_channel();

    int pid;
    // 0 is the master node itself
    for (int id = 1; id < NRO_PROC; id++){
        pid = fork();
        if (pid == 0){
            // child will break the for to avoid creating more processes
            pid_id = id; // each child will have it's own internal ID
            real_pid = getpid(); // get the real PID
            break;
        } else {
            // parent will fill the pid table
            pid_vec[id] = pid;
        }
    }

    if (pid == 0){
        child_manager(tp);
    } else {
        pid_vec[0] = getpid();
        signal(SIGINT, exit_handler_parent);
        parent_manager(tp, pid_vec);
        close_channel();
    }

    topology_clear(tp);
}

#define CLI_SW_HYPER "-h"
#define CLI_SW_TORUS "-t"
#define CLI_SW_FAT   "-f"

to_types process_args(int argc, char* argv[]){
    // copycat from commit ea81598
    char *option;

    option = argv[1]; // Receives the Scheduler struct type

    /*
     * Checks if the scheduler was called with a argument.
     * If there was no argument defining the type of structure to be used, the program is finished.
     * If a argument was provided, the program needs to check if it's a valid argument. In case the
     * entered argument is invalid, the program is finished, otherwise, the program continues it's
     * execution.
     */
    if (argc < 2){
        printf("(W) Run the scheduler with one of the three options: -h, -t or -f...\n");
        return UNK_TO;
    }
    if (strcmp(CLI_SW_FAT, option) == 0){
        return FAT_TREE;
    }
    if (strcmp(CLI_SW_HYPER, option) == 0){
        return HYPER_C;
    }
    if (strcmp(CLI_SW_TORUS, option) == 0){
        return TORUS;
    }

    printf("(E) Unreconized topology %s.\n", option);

    return UNK_TO;
}

int main(int argc, char* argv[]){
    to_types cli_topology;

    cli_topology = process_args(argc, argv);
    if (cli_topology == UNK_TO){
        return 0;
    }

    bootstrap_app(cli_topology);
    return 0;
}