#include <stdio.h>          // Includes the Stardard I/O Library
#include <stdlib.h>         // Includes the Standard C Library
#include <unistd.h>         // Includes the POSIX operating system API
#include <sys/syscall.h>    // Includes the System Calls numbers
#include <sys/types.h>      // Includes the System Primitive Data Types
#include <sys/wait.h>       // Includes the System Wait definitions
#include <sys/mman.h>       // Includes BSD Memory Management Library
#include <sys/stat.h>       // Includes POSIX File Characteristics Library
#include <signal.h>         // Includes the Signal definitions
#include <string.h>         // Includes the String Library
#include <fcntl.h>

#include "msg/msg.h"
#include "processQueue.h"

void create_nodes(char *option);
void create_fat_tree(pid_t _parent, int _level);
void create_hypercube(pid_t _master);
void create_torus(pid_t _master);                       
void delayed_scheduler(); // Ongoing