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

#define NRO_PROC 3

// it's a global variable because signal handlers doesn't provide any parameters
int pid_vec[NRO_PROC];
int pid_id, channel_id;

void shutdown(){
    // shutdown steps
    // breaks topology but kills the current executing processes
    printf("(I) Shutdown process initiated\n");
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
}

void exit_handler_child(int sig_id){
    printf("(I) Exiting child (PID: %d)\n", getpid());
}

void child_manager(to_proxy *topo){
    printf("(I) Initiated process (PID: %d)\n", getpid());

    int rcv_ok = -1;
    channel_id = open_channel();
    msg_packet p;
    signal(SIGINT, exit_handler_child);
    printf("(I) Listining to slot 0x%02x on channel %d (PID: %d)...\n", pid_id, channel_id, getpid());
    rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), pid_id, 0); // block
    if (rcv_ok > 0){
        printf("(I) Received msg '%s' at (PID: %d)\n", p.prog_name, getpid());
    }
}

void parent_manager(to_proxy *topo, int *pid_vec){
    printf("(I) Parent process (PID: %d)\n", pid_vec[0]);

    int *path, path_sz;
    printf("(I) Path from 7 to 9 using HyperCube is: ");
    path = topology_query(topo, 0x7, 0x9, &path_sz);
    print_path(path, path_sz);

    msg_packet p;
    p.delay = 5;
    char *msg_set[3] = {"", "hello, joe", "hello, bear"};
    for (int node_idx=1; node_idx < 3; node_idx++){
        p.type = node_idx;
        strcpy(p.prog_name, msg_set[node_idx]);
        printf("(I) Sending message '%s' to 0x%02x via channel %d\n", p.prog_name, node_idx, channel_id);
        msgsnd(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0); // block until sent
    }

    printf("(I) Waiting for messages on %d (PID: %d)\n", channel_id, pid_vec[0]);
    recv_packet(channel_id, &p);
}

void test_spawn_processes(){
    printf("(I) Gerenciador de Processos\n");
    printf("(I) Número de processos a serem criados: %d\n", NRO_PROC-1);

    to_proxy *tp = topology_create(HYPER_C);
    channel_id = create_channel();

    int pid;
    // 0 is the master node itself
    for (int id = 1; id < NRO_PROC; id++){
        pid = fork();
        if (pid == 0){
            // child will break the for to avoid creating more processes
            pid_id = id; // each child will have it's own internal ID
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
    }

    topology_clear(tp);
}

int main(){
    test_spawn_processes();
    return 0;
}