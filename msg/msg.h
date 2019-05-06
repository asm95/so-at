// id of message key
#define MQ_ID 0x8349

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
    char prog_name[20];
    unsigned int delay;

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