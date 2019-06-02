#ifndef MSG_H
#define MSG_H

// id of message key
#define MQ_ID 0x8349
#define MAX_PROG_NAME 20

typedef enum {
    AC_NOP, // do anything (undefined)
    AC_NP, // new program
    AC_SP, // spawn program
    AC_FP, // finished program
} msg_action;

typedef struct {
    long type;

    // action itself
    msg_action ac;

    // start a new program data
    char prog_name[MAX_PROG_NAME];
    unsigned int delay;
    // todo.warning: 32-bit values are susceptible to the "Year 2038" problem
    // info: we're trying to avoid including <time.h>
    unsigned int req_t; // (req)uest_(t)ime is when at was executed

    // routing data
    int routing_idx;
    int routing_path[16];

    // finshed program data
    unsigned int pid_id;

} msg_packet;

int open_channel();
int create_channel();
int delete_channel(int msg_id);

int send_packet(int msg_id, unsigned int delay);
int recv_packet(int msg_id, msg_packet *p);

// new interface that is flexible
int send_msg(int channel_id, msg_packet *p);

// node process message forwaring
int route_walk(msg_packet *p);

#endif