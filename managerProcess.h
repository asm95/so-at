#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHMID 0x8349

#ifndef _SYS_LIBS_H
    #include <sys/signal.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

#ifndef _MSG_QUEUE_H
    #include "msgQueue/msgQueue.h"
#endif

void manager_exit();
void manager_process(int _id, pid_t *connections, char *option);
void start_exec();