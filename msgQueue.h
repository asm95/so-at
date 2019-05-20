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
    int  shutdown;
    
    // Informação de comunicação
    int _mdst;                  // ID of manager destination
    int _id;
    int exec;                   // Flag to indicate manager to execute program

    // Informação de execução
    pid_t  pid;                 // PID of manager child
    time_t begin;               // Manager child was created at
    time_t end;                 // Manager child was terminated at
} msg_packet;

typedef struct pid_packet{
    long type;                  // Type of the message
    pid_t pid;                  // Content: PID of Scheduler
} pid_packet;

int create_channel(int key);
int get_channel(int key);
int delete_channel(int msg_id);