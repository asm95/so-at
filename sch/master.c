#include <stdio.h> // for printf, sprintf
#include <stdlib.h> // for NULL
#include <signal.h> // for kill, SIGINT
#include <time.h> // for localtime, tm

#include "../common.h"
#include "../globals.h"
#include "../topology.h"
#include "../msg/msg.h"
#include "jobs.h"

char * as_pretty_time(char *buf, long time_now){
    struct tm * time_info;
    time_info = localtime(&time_now);
    if (time_info == NULL){
        sprintf(buf, "%s", "<UNK>");
        return buf;
    }
    sprintf(buf, "%02d:%02d",
        time_info->tm_min,
        time_info->tm_sec
    );
    return buf;
}

void print_summary(job_node *job_done, job_node *job_pending){
    printf("(%3s) ~~ Resumo ~~\n", "M");
    char time_buf[4][11];
    job_node *el;
    while(1){
        el = pop_front_jl(&job_done);
        if (!el){
            break;
        }
        printf("%5sjob=%d, arquivo=%s, delay=%d, makespan=%.0f\n",
            "", el->job_id, el->prog_name, el->delay,
            (double)el->term_t - (double)el->sch_t
        );
        printf(
            "%5s\tsch_t=%s,exc_t=%s,term_t=%s\n"
            "%5s\texc_ord=%d\n",
            "", as_pretty_time((char*)&time_buf[0], el->sch_t),
                as_pretty_time((char*)&time_buf[1], el->exc_t),
                as_pretty_time((char*)&time_buf[2], el->term_t),
            "", el->exc_ord
        );
        free(el);
    }
    printf("%5s ~~ Jobs não executados ~~\n", "");
    while(1){
        el = pop_front_jl(&job_pending);
        if (!el){
            break;
        }
        printf("%5sjob=%d, arquivo=%s, delay=%d, makespan=<UNK>\n",
            "", el->job_id, el->prog_name, el->delay
        );
        free(el);
    }

}

void shutdown(int pid_vec[]){
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
    print_summary(job_done_l, job_l);
}

void on_job_finished(job_node *job_done){
    if (job_done == NULL){
        return;
    }

    job_node *el = job_done;
    el->term_t = time(NULL);
}

void parent_manager(to_proxy *topo, int *pid_vec){
    printf("(%3s) Parent process (PID: %d)\n", "M", pid_vec[0]);

    job_l = new_jl();

    // check if communication channel exists
    if (channel_id <= 0){
        printf("(%3s) Failed to create channel %d. Exiting...\n", "M", MQ_ID);
        shutdown(pid_vec);
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
                on_job_finished(job_done_l);
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