#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>

#define MQ_SD 0x7497            // Channel for communication Scheduler-Delayed
#define MQ_SM 0x8349            // Channel for communication Scheduler-Managers

typedef struct msg_packet {
    long type;                  // Defines the message type

    char name[256];             // Program name for the scheduler
    int  delay;                 // Delay (in seconds) for execution

    int _mdst;                  // ID of manager destination
    
    // adicionar campos!
    int _id;                    // Process ID
    int ready;                  // Flag to indicate manager ready to execute
    int exec;                   // Flag to indicate manager to execute program
    int finished;               // Flag to indicate manager finished execution
} msg_packet;

typedef struct pid_packet{
    long type;
    pid_t pid;
} pid_packet;

int create_channel(int key);
int get_channel(int key);
int delete_channel(int msg_id);

int send_packet();
// int recv_packet();