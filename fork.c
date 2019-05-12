#include <stdio.h>
#include <stdlib.h> // for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "topology.h"
#include "msg/msg.h"


void print_path(int *arr, int sz){
    for(int i=0; i<sz; i++){
        printf("%X", arr[i]);
        if (i < sz-1){
            printf(" -> ");
        }
    }printf("\n");
}

#define NRO_PROC 5

// it's a global variable because signal handlers doesn't provide any parameters
int pid_vec[NRO_PROC];
int pid_id, real_pid, channel_id, do_exit = 0;
int job_id = 0;
int child_state = 0;


void shutdown(){
    // shutdown steps
    // breaks topology but kills the current executing processes
    printf("(%3s) Shutdown process initiated\n", "M");
    int status;
    for (int id=1; id < NRO_PROC; id++){
        // don't forget to do a waitpid on fork cus your process will turn into a zombie
        // and zombies don't release their line in the process table!
        // you can do a waitpid after the process exited
        kill(pid_vec[id], SIGINT);
        waitpid(pid_vec[id], &status, 0);
    }
}

void exit_handler_parent(int sig_id){
    // will shutdown all programs and print the report
    shutdown();
    delete_channel(channel_id);
    do_exit = 1;
}

void exit_handler_child(int sig_id){
    printf("(C%2d) Exiting child\n", pid_id);
    do_exit = 1;
}

int route_walk(msg_packet *p){
    // returns next node
    int next_node_idx = p->routing_idx;
    if (next_node_idx >= 16){
        return -1; // routing reached it's destination
    }
    int next_node = p->routing_path[next_node_idx];
    p->routing_idx += 1;
    p->type = next_node + 1;

    return next_node;
}

void route_print(msg_packet *p){
    int idx = p->routing_idx;
    for(; idx < 16; idx++){
        printf("%d ", p->routing_path[idx]);
    }
    printf("\n");
}

void exec_program(){
    int pid = fork();
    if (pid == 0){
        // child will sleep for a while
        int sleep_time = 5;
        printf("(I) Hello! I'm sleeping for %d seconds\n", sleep_time);
        sleep(sleep_time);
        exit(0);
    } else {
        // node will now listen to the queue and wait for it's child termination
        child_state = 1;
    }
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
            exec_program();
        }
    }
}

// we need to finish that thing
#include "jobs.c"
job_node *job_l;

int finished_c = -2;

void alarm_handler_parent(int sigid){
    // happens when a program will execute now after delay
    finished_c = -1;
}

void spawn_program(msg_packet *p, to_proxy *topo){
    p->ac = AC_SP; // packet will ask for nodes to spawn a program
    int dest_node;
    for (int node_idx=1; node_idx < NRO_PROC; node_idx++){
        p->routing_idx = topology_search(topo, node_idx, p->routing_path, 16);
        dest_node = route_walk(p);
        if (dest_node <= 0){
            printf("(%3s) Error on routing. Not a valid destination (%d)\n", "M", dest_node);
            continue;
        }
        printf("(%3s) Sending packet to %d via %d\n", "M", node_idx, dest_node);
        msgsnd(channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // block until sent
    }
}

void add_program(msg_packet *p, to_proxy *topo, int *finished_c){
    // very strict function, other values will have no effect under the scheduler
    // i.e. bad packets are discarded

    int list_sz = get_jl_sz(job_l);

    if (list_sz < 0){
        printf("(%3s) Job list seems to be corruped. Please panic!\n", "(MW)");
        return;
    }

    if (list_sz == 0){
        // alarm is set when nobody is on the list
        int seconds = p->delay;
        if (seconds < 0){
            printf("(%3s) Bad packet received\n", "(MW)");
            // back packet
            return;
        }
        alarm(seconds);
    }

    printf("(%3s) Enqueuing job %d...\n", "M", job_id);
    job_l = insert_jl(job_l, create_job(p->delay, p->prog_name));
}

void check_job_queue(to_proxy *topo){
    // assuming all nodes have finished
    // check if there's any program to be executed (TBE)
    // if there is, then take it out from the list and broadcast to all nodes
    // this should be run when a program has finished
    int list_sz = get_jl_sz(job_l);

    if (list_sz <= 0){
        return;
    }
    job_node *el = pop_front(&job_l);
    // forgot that they talked french
    msg_packet p;
    strncpy(p.prog_name, el->prog_name, MAX_PROG_NAME-1);
    p.prog_name[MAX_PROG_NAME-1] = '\0';
    p.delay = el->delay; // todo.warning: makes no sense if delay is really used
    spawn_program(&p, topo);
    free(el); // el turns into an unsued node, it must be free from memory
    finished_c = 0;

    printf("(%3s) There are %d jobs waiting to be executed.\n", "M", list_sz - 1);
}

void process_packet_master(msg_packet *p, to_proxy *topo, int *finished_c){
    switch(p->ac){
        case AC_FP:
            printf("(%3s) Program finished from NID: 0x%02x\n", "M", p->pid_id);
            *finished_c += 1;
            break;
        case AC_NP:
            printf("(%3s) Received to execute new program '%s' in %d seconds\n", "M", p->prog_name, p->delay);
            add_program(p, topo, finished_c);
            
        default: break;
    }
}

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

void parent_manager(to_proxy *topo, int *pid_vec){
    printf("(%3s) Parent process (PID: %d)\n", "M", pid_vec[0]);

    job_l = new_jl();

    // check if communication channel exists
    if (channel_id <= 0){
        printf("(%3s) Failed to create channel %d. Exiting...\n", "M", MQ_ID);
        shutdown();
        return;
    }

    // will calculate routes for all nodes
    topology_init(topo, 0); // parent ID is always 0

    msg_packet p;
    p.delay = 5;

    // order to execute command to each node

    printf("(%3s) Waiting for messages on %d\n", "M", channel_id);
    int rcv_ok;
    while(! do_exit){
        // printf("(%3s) Value of finished_c is %d...\n", "M", finished_c);
        if (finished_c >= 0){
            // means we're waiting for processes to finish
            if (finished_c == NRO_PROC-1){
                // if everyone finished, then we reset the counter
                printf("(%3s) Waiting for new processes\n", "M");
                finished_c = -2;
            }
        } else
        if (finished_c == -1){
            // time to check the job queue for new processes
            check_job_queue(topo);
        }
        // printf("(%3s) Checking for new messages...\n", "M");
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0);
        if (rcv_ok > 0){ // did we read some bytes?
            process_packet_master(&p, topo, &finished_c);
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

void test_spawn_processes(to_types scheduler_topo){
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
        signal(SIGALRM, alarm_handler_parent);
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

    test_spawn_processes(cli_topology);
    return 0;
}