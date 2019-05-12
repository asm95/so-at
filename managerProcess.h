#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef _SYS_LIBS_H
    #include <sys/signal.h>
    #include <sys/types.h>
#endif

#ifndef _MSG_QUEUE_H
    #include "msgQueue/msgQueue.h"
#endif

void manager_exit();
void manager_process(int _id, pid_t *connections);