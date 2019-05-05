#include <stdio.h>
#include <stdlib.h> // for exit
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


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

// @todo.warning(tag='prototype'): inline declarations may confuse your buddies
// @todo.warning(tag='spell'): no spell checking is being used, you've been warned.

typedef struct {
    // this struct simplifies function arguments definition
    // works very well in deep nested programs

    // delay occurred to test msg queue behaviour on errors
    // (e.g. attempt to read when it not exists)
    int parent_delay;
    int child_delay;
} UnitTestData;

void test_messages(UnitTestData *d){
    /*
    This function will open a message queue and the child process will attempt to
    send a message object[1] to it's parent, who is the only one listening to the
    queue.

    This communication method is for using between at and scheduler program.

    At is a program that just read an user input and attempt sending his request
    to the scheduler process (running in the background);

    [1] it is a struct defined in msg.h
    */
    // initial fork to test two processes behaviour in various system calls
    int pid = fork();
    int status;
    int cid;
    // always important logging the PIDs so we can watch if any process turned into a zombie
    // @todo(tag='suggestion'): memory profiling would be also welcome!
    if (pid == 0){
        pid = getpid();
        printf("(I) Initiated child process with PID %d\n", pid);
        printf("(I) Waiting for channel...\n");
        sleep(d->child_delay);
        cid = open_channel();
        if (cid == -1){
            printf("(W) Failed to get channel. Exiting...\n");
            exit(0);
        }
        printf("OK: got channel.\n");
        printf("Sending message...\n");
        if (send_packet(cid, 5) == 0){
            printf("OK: Sent message\n");
        }
    } else {
        pid = getpid();
        printf("(I) Initiated parent process with pid %d\n", pid);
        sleep(d->parent_delay);
        cid = create_channel();
        if (cid > 0){
            msg_packet p;
            printf("(I) Waiting for messages...\n");
            if (recv_packet(cid, &p) == 0){
                printf("(I) Got message: program %s, delay %u\n", p.prog_name, p.delay);
            }
        } else if (cid == -2){
            printf("(W) Message key already exists\n");
        }
        waitpid(pid, &status, 0);
        if (cid != -1){
            delete_channel(cid);
        }
    }
}

void test_channel_creation(){

    /*
    trivial case: open a channel using a key and get from the key should return the same
    number
    */
    printf("(I) Attempting to create msg queue with key %d\n", MQ_ID);
    int cid = create_channel();
    printf("(I) Attempting to get the created channel...\n");
    int oid = open_channel();
    printf("(D) (CID, OID) are (%d,%d) (hint: they must match)\n", cid, oid);
    printf("(I) Deleting channel...\n");
    int stat = delete_channel(cid);
    if (stat != 0){
        printf(
        "(I) Your channel was not deleted (stat was %d).\n"
        "Now you have a zombie object in kernel space. Please panic.\n", stat
        );
        return;
    }
    printf("(I) Attempting to open channel...\n");
    oid = open_channel();
    if (oid != -1){
        printf("(E) OID shoud be -1 but it was %d\n", oid);
        return;
    }
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

void test_channel_close(){
    // clear any left open resources
    int oid = open_channel();
    if (oid > 0){
        int stat = delete_channel(oid);
        if (stat < 0){
            printf("(W) Could not close a left opened channel."
            "Kernel resource is probably leaked. Please panic!"
            );
        }
    }
}

// global variable for interrupt message
const char *g_interrupt_msg = NULL;
void dummy(int sig_id){
    if (g_interrupt_msg != NULL){
        printf(g_interrupt_msg);
    }
}

void test_msg_queue (){
    UnitTestData td;

    test_channel_creation();
    char test_cases [2] = {0,1};
    /* 
    first case when queue is created in the right order
    messages should be exchanged successfully
    */
   if (test_cases[0]){
    td.child_delay = 2;
    td.parent_delay = 0;
    test_messages(&td);
   }
    /*
    second case: parent takes too long to open the channel
    */
   if (test_cases[1]){
    printf("\n(I) The child must fail to obtain the queue ID\n");
    signal(SIGALRM, dummy);
    g_interrupt_msg = "(I) Time limit reached while waiting for messages in queue\n";
    td.parent_delay = 2;
    td.child_delay = 0;
    alarm(td.parent_delay + 3);
    test_messages(&td);
   }

   test_channel_close();

}

int main(){
    test_msg_queue();

    return 0;
}