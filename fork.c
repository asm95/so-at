#include <stdio.h>
#include <stdlib.h> // for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


#include "topology.h"
#include "msg/tests.h"

void print_path(int *arr, int sz){
    for(int i=0; i<sz; i++){
        printf("%X", arr[i]);
        if (i < sz-1){
            printf(" -> ");
        }
    }printf("\n");
}

void child_manager(to_proxy *topo){
    printf("(I) Initiated %d process\n", getpid());

    int *path, path_sz;
    printf("(I) Path from 0 to A using HyperCube is: ");
    path = topology_query(topo, 0x0, 0xa, &path_sz);
    print_path(path, path_sz);
}

#define NRO_PROC 3

void parent_manager(to_proxy *topo, int *pid_vec){
    printf("(I) Parent process (PID: %d)\n", pid_vec[0]);

    int *path, path_sz;
    printf("(I) Path from 7 to 9 using HyperCube is: ");
    path = topology_query(topo, 0x7, 0x9, &path_sz);
    print_path(path, path_sz);

    // shutdown process
    // breaks topology but kills the current executing processes
    int status;
    for (int id=1; id < NRO_PROC; id++){
        // don't forget to do a waitpid on fork cus your process will turn into a zombie
        // and zombies don't release their line in the process table!
        // you can do a waitpid after the process exited
        waitpid(pid_vec[id], &status, 0);
    }
}

void test_spawn_processes(){
    printf("(I) Gerenciador de Processos\n");
    printf("(I) Número de processos a serem criados: %d\n", NRO_PROC-1);

    to_proxy *tp = topology_create(HYPER_C);
    

    int pid_vec[NRO_PROC];

    int pid;
    // 0 is the master node itself
    for (int id = 1; id < NRO_PROC; id++){
        pid = fork();
        if (pid == 0){
            // child will break the for to avoid creating more processes
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
        parent_manager(tp, pid_vec);
    }

    topology_clear(tp);
}

int main(){
    test_spawn_processes();
    return 0;
}