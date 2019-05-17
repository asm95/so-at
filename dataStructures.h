#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define CHILDS 16
#define FATCHILDS 15

typedef struct execq {
    int  job;
    char *name;
    int  rDelay;
    
    time_t uDelay;
    time_t sent;

    struct execq *prox;
} execq;

typedef struct execd{
    pid_t  pid;
    char*  program;
    time_t sent;
    time_t begin;
    time_t end;
    double makespan;

    struct execd *next;
} execd;

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

typedef struct manq{
    pid_t _id;
    
    struct manq *next;
} manq;

void createQueue(execq **queue);
void insertProcess(execq **queue, char *_name, int _delay);
void removeProcess(execq **queue);
void listProcesses(execq *queue);
void updateDelays(execq **queue);

void createExecD(execd **done);
void insertExecD(execd **done, pid_t pid, char* program, time_t sent, time_t begin, time_t end);
void deleteExecD(execd **done);
void listExecD(execd *done);

void createFTree(fTree **_tree);
void definesTree(fTree **_tree, int _parent, int *_node, int _level);
void readTree(fTree *_tree);
pid_t* get_fTreeConnection(fTree *_tree, int _id);

void createHyperTorus(hyperTorus **_ht);
void definesHyper(hyperTorus **_hyper, int _id);
void definesTorus(hyperTorus **_torus, int _id);
void readHyperTorus(hyperTorus *ht);
pid_t* get_htConnection(hyperTorus *_hyper, int _id);

void readHTConnections(pid_t *connections, int _id);
void readFTConnections(pid_t *connections, int _id);

void createManQ(manq **_manq);
void insertManQ(manq **_manq, int _id);
void insertManQSorted(manq **_manq, int _id);
manq* removeManQ(manq **_manq);
void readManQ(manq *_manq);