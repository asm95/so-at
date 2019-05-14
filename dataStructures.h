#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHILDS 16
#define FATCHILDS 15

typedef struct execq {
    char *name;
    int  delay;

    struct execq *prox;
} execq;

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

void    createQueue(execq **queue);
void    insertProcess(execq **queue, char *_name, int _delay);
execq* removeProcess(execq **queue);
void    listProcesses(execq *queue);

void createFTree(fTree **_tree);
void definesTree(fTree **_tree, int _parent, int *_node, int _level);
void readTree(fTree *_tree);
pid_t* get_fTreeConnection(fTree *_tree);

void createHyperTorus(hyperTorus **_ht);
void definesHyper(hyperTorus **_hyper, int _id);
void definesTorus(hyperTorus **_torus, int _id);
void readHyperTorus(hyperTorus *ht);
pid_t* get_htConnection(hyperTorus *_hyper);

void readConnections(pid_t *connections);

void createManQ(manq **_manq);
void insertManQ(manq **_manq, int _id);
manq* removeManQ(manq **_manq);