// id of message key
#define MQ_ID 0x8349

typedef struct {
    long type;
    char prog_name[20];
    unsigned int delay;
} msg_packet;

int open_channel();
int create_channel();
int delete_channel(int msg_id);

int send_packet(int msg_id, unsigned int delay);
int recv_packet(int msg_id, msg_packet *p);