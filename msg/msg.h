#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>

#define MQ_ID 0x8349            // Channel for communication

int create_channel();
int open_channel();
int delete_channel(int msg_id);

int send_packet();
int recv_packet();

typedef struct msg_packet {
    long type;                  // Defines the message type

    char name[256];             // Program name for the scheduler
    int  delay;                 // Delay (in seconds) for execution
} msg_packet;