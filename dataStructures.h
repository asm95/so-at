#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHILDS 16
#define FATCHILDS 15

typedef struct _queue {
    char *name;
    int  delay;

    struct _queue *prox;
} _queue;

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

void    createQueue(_queue **queue);
void    insertProcess(_queue **queue, char *_name, int _delay);
_queue* removeProcess(_queue **queue);
void    listProcesses(_queue *queue);

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