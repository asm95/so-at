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
    do_exit = 1;
}

void exit_handler_child(int sig_id){
    printf("(I) Exiting child (PID: %d)\n", getpid());
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

void process_packet(msg_packet *p, to_proxy *topo){
    int next_node = route_walk(p);
    if (next_node >= 0){
        printf("(I) Packet is for %d (PID: %d)\n", next_node, real_pid);
        msgsnd(channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
    } else
    if (next_node == -1){
        printf("(I) Packet is for me! (PID: %d)\n", real_pid);
        if (p->ac == AC_SP){
            printf("(I) Message for spawn program named '%s'\n", p->prog_name);
            sleep(2);
            // search for route
            p->routing_idx = topology_search(topo, 0x0, p->routing_path, 16); // search for master
            p->ac = AC_FP;
            p->pid_id = pid_id;
            next_node = route_walk(p); // check which node should I send first
            printf("(I) Sending AC_FP to master via %d (PID: %d)\n", next_node, real_pid);
            msgsnd(channel_id, p, sizeof(msg_packet) - sizeof(long), 0); // forward packet
        }
    }
}

void process_packet_master(msg_packet *p, to_proxy *topo){
    switch(p->ac){
        case AC_FP:
            printf("(M) Program finished from NID: 0x%02x\n", p->pid_id);
            break;
        default: break;
    }
}

void child_manager(to_proxy *topo){
    printf("(I) Initiated process (PID: %d)\n", real_pid);
    topology_init(topo, pid_id);

    int rcv_ok = -1;
    channel_id = open_channel();
    msg_packet p;
    signal(SIGINT, exit_handler_child);
    while(! do_exit){
        printf("(I) Listining to slot 0x%02x on channel %d (PID: %d)...\n", pid_id+1, channel_id, real_pid);
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), pid_id+1, 0); // block
        if (rcv_ok > 0){
            process_packet(&p, topo);
        }
    }
}

void parent_manager(to_proxy *topo, int *pid_vec){
    printf("(I) Parent process (PID: %d)\n", pid_vec[0]);

    // check if communication channel exists
    if (channel_id <= 0){
        printf("(I) Failed to create chanel %d. Exiting...\n", MQ_ID);
        shutdown();
        return;
    }

    // will calculate routes for all nodes
    topology_init(topo, 0); // parent ID is always 0

    msg_packet p;
    p.delay = 5;

    // order to execute command to each node
    strcpy(p.prog_name, "hello");
    p.ac = AC_SP; // packet will ask for nodes to spawn a program
    int dest_node;
    for (int node_idx=1; node_idx < NRO_PROC; node_idx++){
        p.routing_idx = topology_search(topo, node_idx, p.routing_path, 16);
        dest_node = route_walk(&p);
        if (dest_node <= 0){
            printf("(I) Error on routing. Not a valid destination (%d)\n", dest_node);
            continue;
        }
        printf("(I) Sending packet to %d via %d\n", node_idx, dest_node);
        msgsnd(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0); // block until sent
    }

    printf("(I) Waiting for messages on %d (PID: %d)\n", channel_id, pid_vec[0]);
    int rcv_ok;
    while(! do_exit){
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0);
        if (rcv_ok > 0){
            process_packet_master(&p, topo);
        }
    }
}

void close_channel(){
    int oid = open_channel();
    int sts = delete_channel(oid);
    if (sts == 0){
        printf("(I) Removed channel %d sucessfully!\n", oid);
    }
}

void test_spawn_processes(){
    printf("(I) Gerenciador de Processos\n");
    printf("(I) Número de processos a serem criados: %d\n", NRO_PROC-1);

    to_proxy *tp = topology_create(FAT_TREE);
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

int main(){
    //close_channel(); exit(0);

    test_spawn_processes();
    return 0;
}