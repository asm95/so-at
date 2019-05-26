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

void alarm_handler_parent(int sigid){
    // happens when a program will execute now after delay
    finished_c = -1;
}

char check_spawn_delay(unsigned int sch_t){
    // If you want to know what time_t really is, your journey will be a hell.
    // At the end C stantard says it's an arithmetic type, but do not define
    // its size, precision of whatever - so it up to the SO devs to make their mess.
    // With some reasearch I know it's kind of 64-bit in size
    // This program will break in 2038, but nobody cares.
    unsigned int now, alarm_time;
    double real_delay; // see the difftime for more information
    char alarm_was_set = 0;

    now = time(NULL);
    real_delay = (double)now - (double)sch_t;

    // then we'll figure out whether if we have or not to configure one
    if (real_delay >= 0){
        // we need to execute the program now
        finished_c = -1;
    } else {
        // we're still on time, set an alarm
        // we set the alarm just after the element is on the list
        alarm_time = (unsigned int)(-real_delay);
        signal(SIGALRM, alarm_handler_parent);
        alarm(alarm_time);
        alarm_was_set = 1;
    }

    return alarm_was_set;
}

void check_job_queue(to_proxy *topo){
    job_node *el = job_l;
    if (el == NULL){
        finished_c = -2;
        return;
    }
    check_spawn_delay(el->sch_t);
}

void add_program(msg_packet *p, to_proxy *topo, int *finished_c, int current_job_id){
    // very strict function, other values will have no effect under the scheduler
    // i.e. bad packets are discarded

    int list_sz = get_jl_sz(job_l);

    if (list_sz < 0){
        printf("(%3s) Job list seems to be corruped. Please panic!\n", "(MW)");
        return;
    }

    if (p->delay < 0){
        printf("(%3s) Bad packet received. Reason: delay < 0\n", "M");
        return;
    }

    unsigned int sch_t;
    sch_t = p->req_t + p->delay;

    printf("(%3s) Enqueuing job %d...\n", "M", current_job_id);
    printf("(%3s) Scheduled time is: %u\n", "M", sch_t);
    job_node *el = create_job(sch_t, p->delay, p->prog_name);
    el->job_id = job_id;
    job_id += 1;
    job_l = insert_jl(job_l, el);
    
    if (*finished_c == -2){
        // means no one set an alarm, thus no program is executing
        check_spawn_delay(sch_t);
    }
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

void dispatch_program(to_proxy *topo, int jobs_exec_ord){
    int list_sz = get_jl_sz(job_l);
    if (list_sz <= 0){
        return;
    }

    job_node *el = pop_front_jl(&job_l);
    list_sz += -1;
    msg_packet p;
    strncpy(p.prog_name, el->prog_name, MAX_PROG_NAME-1);
    p.prog_name[MAX_PROG_NAME-1] = '\0';
    spawn_program(&p, topo);
    el->exc_t = time(NULL);
    el->exc_ord = jobs_exec_ord;
    jobs_exec_ord += 1;
    push_front_jl(&job_done_l, el);
    finished_c = 0;

    printf("(%3s) There are %d jobs waiting to be executed.\n", "M", list_sz);
}

void process_packet_master(msg_packet *p, to_proxy *topo, int *finished_c){
    switch(p->ac){
        case AC_FP:
            printf("(%3s) Program finished from NID: 0x%02x\n", "M", p->pid_id);
            *finished_c += 1;
            break;
        case AC_NP:
            printf("(%3s) Received to execute new program '%s' in %d seconds\n", "M", p->prog_name, p->delay);
            add_program(p, topo, finished_c, job_id);
            
        default: break;
    }
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
            dispatch_program(topo, g_exec_ord);
        }
        // printf("(%3s) Checking for new messages...\n", "M");
        rcv_ok = msgrcv(channel_id, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0);
        if (rcv_ok > 0){ // did we read some bytes?
            process_packet_master(&p, topo, &finished_c);
        }
    }
}