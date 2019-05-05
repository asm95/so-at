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
    test_msg_queue();
    return 0;
}