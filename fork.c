#include <stdio.h>  // for printf
#include <unistd.h> // for getpid, fork
#include <signal.h> // for signal, SIGINT
#include <string.h> // for strcmp

#include "common.h"
#include "globals.h"

#include "util/log.h"
#include "topology.h"
#include "msg/msg.h"
#include "sch/jobs.h"
#include "sch/master.h"
#include "wrk/node.h"

#define CLI_SW_HYPER "-h"
#define CLI_SW_TORUS "-t"
#define CLI_SW_FAT   "-f"

void print_path(int *arr, int sz){
    for(int i=0; i<sz; i++){
        printf("%X", arr[i]);
        if (i < sz-1){
            printf(" -> ");
        }
    }printf("\n");
}

// global variables are acutally defined here
// this mess all started when signal handlers didn't have arguments
int pid_vec[NRO_PROC];
int pid_id, real_pid, channel_id, do_exit = 0;
int job_id = 0;
int child_state = 0;
int g_exec_ord = 0;
int finished_c = -2;

job_node *job_l, *job_done_l = NULL;

void route_print(msg_packet *p){
    int idx = p->routing_idx;
    for(; idx < 16; idx++){
        printf("%d ", p->routing_path[idx]);
    }
    printf("\n");
}

void exit_handler_parent(int sig_id){
    // will shutdown all programs and print the report
    shutdown(pid_vec);
    delete_channel(channel_id);
    delete_jl(job_l);
    do_exit = 1;
}

void exit_handler_child(int sig_id){
    printf("(C%2d) Exiting child\n", pid_id);
    terminate_child(1);
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
    
    log_init();
    log_set_enabled(LOG_GRP_DEFAULT, LOG_GRP_ENABLED);

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
        signal(SIGINT, exit_handler_child);
        child_manager(tp, pid_id);
    } else {
        pid_vec[0] = getpid();
        signal(SIGINT, exit_handler_parent);
        parent_manager(tp, pid_vec);
        close_channel();
    }

    topology_clear(tp);
}


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