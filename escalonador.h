#include <stdio.h>          // Includes the Stardard I/O Library
#include <stdlib.h>         // Includes the Standard C Library
#include <unistd.h>         // Includes the POSIX operating system API
#include <sys/syscall.h>    // Includes the System Calls numbers
#include <sys/types.h>      // Includes the System Primitive Data Types
#include <sys/wait.h>       // Includes the System Wait definitions
#include <sys/stat.h>       // Includes POSIX File Characteristics Library
#include <signal.h>         // Includes the Signal definitions
#include <string.h>         // Includes the String Library
#include <sys/shm.h>

#ifdef _MSG_QUEUE_H
    #include "msgQueue/msgQueue.h"
#endif
#include "dataStructures.h"
#include "managerProcess.h"

void delayed_scheduler(int managers);
void shutdown();
void new_schedule();
void send_pid();
void update_alarm();