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

typedef struct fTree {
    pid_t id;                                       // ID of the process

    int   lenght;                                   // Number of connections
    pid_t *connects;                                // Array with the connections

    struct fTree *left;                             // Left node
    struct fTree *right;                            // Right node
} fTree;

typedef struct hyperTorus{
    pid_t id;                                       // ID of the process

    int   length;                                   // Number of connections
    pid_t *connects;                                // Array with the connections

    struct hyperTorus *next;                        // Next process
} hyperTorus;

void definesTree(fTree **_tree, int _parent, int *_node, int _level);
void readTree(fTree *_tree);
pid_t* get_fTreeConnection(fTree *_tree);

void definesHyper(hyperTorus **_hyper, int _id);
void readHyper(hyperTorus *ht);
pid_t* get_hyperConnection(hyperTorus *_hyper);

void definesTorus(hyperTorus **_torus, int _id);
void readTorus(hyperTorus *_torus);
pid_t* get_torusConnection(hyperTorus *_hyper);    

void delayed_scheduler(); // Ongoing