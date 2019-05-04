#include <stdio.h>
#include <stdlib.h> // for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include "topology.h"
#include "msg.h"

void print_path(int *arr, int sz){
    for(int i=0; i<sz; i++){
        printf("%X", arr[i]);
        if (i < sz-1){
            printf(" -> ");
        }
    }printf("\n");
}

void child_manager(to_proxy *topo){
    printf("(I) Child process\n");

    int *path, path_sz;
    printf("(I) Path from 0 to A using HyperCube is: ");
    path = topology_query(topo, 0x0, 0xa, &path_sz);
    print_path(path, path_sz);
}

void parent_manager(to_proxy *topo){
    printf("(I) Parent process\n");

    int *path, path_sz;
    printf("(I) Path from 7 to 9 using HyperCube is: ");
    path = topology_query(topo, 0x7, 0x9, &path_sz);
    print_path(path, path_sz);
}

void test_messages(){
    int pid = fork();
    int status;
    int cid;
    if (pid == 0){
        printf("Waiting for channel...\n");
        sleep(5);
        cid = open_channel();
        if (cid == -1){
            printf("Failed to get channel. Exiting...\n");
            exit(0);
        }
        printf("OK: got channel.\n");
        printf("Sending message...\n");
        if (send_packet(cid, 5) == 0){
            printf("OK: Sent message\n");
        }
    } else {
        cid = create_channel();
        if (cid != -1){
            msg_packet p;
            printf("Waiting for messages...\n");
            if (recv_packet(cid, &p) != -1){
                printf("Got message: program %s, delay %u\n", p.prog_name, p.delay);
            }
        } else if (cid == -2){
            printf("(W) Message key already exists\n");
        }
        printf("cid was %d\n", cid);
        waitpid(pid, &status, 0);
        if (cid != -1){
            delete_channel(cid);
        }
    }
}

void test_channel(){
    int cid = create_channel();
    int oid = open_channel();
    printf("cid, oid was %d,%d\n", cid, oid);
    int stat = delete_channel(cid);
    printf("stat %d\n", stat);
}

void test_spawn_processes(){
    printf("(I) Gerenciador de Processos\n");

    to_proxy *tp = topology_create(HYPER_C);
    topology_clear(tp);

    int pid = fork();
    int status;
    if (pid == 0){
        child_manager(tp);
    } else {
        parent_manager(tp);

        // don't forget to do a waitpid on fork cus your process will turn into a zombie
        // and zombies don't release their line in the process table!
        // you can do a waitpid after the process exited

        waitpid(pid, &status, 0);
    }
}

int main(){
    test_messages();

    return 0;
}