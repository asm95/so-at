#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _queue {
    char *name;
    int  delay;

    struct _queue *prox;
} _queue;

void    createQueue(_queue **queue);
void    insertProcess(_queue **queue, char *_name, int _delay);
_queue* removeProcess(_queue **queue);
void    listProcesses(_queue *queue);