#include <stdlib.h> // for NULL

#include "../common.h"
#include "../globals.h"
#include "../topology.h"
#include "../msg/msg.h"
#include "jobs.h"


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

    printf("(%3s) Waiting for messages on %d\n", "M", channel_id);
    int rcv_ok;
    while(! do_exit){
        // printf("(%3s) Value of finished_c is %d...\n", "M", finished_c);
        if (finished_c >= 0){
            // means we're waiting for children nodes to finish
            if (finished_c == NRO_PROC-1){
                // if everyone finished, then we reset the counter
                printf("(%3s) Waiting for new processes\n", "M");
                remove_program();
                // checks the list for the next job in the queue
                check_job_queue(topo);
            }
        }
        if (finished_c == -1){
            // spawns the program imediatelly
            dispatch_program(topo);
        }
        // printf("(%3s) Checking for new messages...\n", "M");
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0);
        if (rcv_ok > 0){ // did we read some bytes?
            process_packet_master(&p, topo, &finished_c);
        }
    }
}