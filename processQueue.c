#include "processQueue.h"

void createQueue(_queue **queue){
    *queue = NULL;
}

void insertProcess(_queue **queue, char *_name, int _delay){
    _queue *q1, *q2;

    q1 = malloc(sizeof(_queue));
    q1->name = malloc(sizeof(char)*strlen(_name));
    strcpy(q1->name, _name);
    q1->delay = _delay;
    q1->prox  = NULL;

    if(*queue == NULL)
        *queue = q1;
    else{
        q2 = *queue;

        while(q2->prox != NULL)
            q2 = q2->prox;
        q2->prox = q1;
    }
}

_queue* removeProcess(_queue **queue){
    _queue *q1;

    if(*queue == NULL)
        return NULL;
    else{
        q1 = *queue;
        *queue = q1->prox;
        q1->prox = NULL;
    }

    return q1;
}

void listProcesses(_queue *queue){
    int job = 0;
    _queue *q1;

    if(queue == NULL)
        printf("No processes on execution queue...\n");
    else{
        q1 = queue;
        while(q1 != NULL){
            printf("%d\t%s\t%d\n", job, q1->name, q1->delay);
            job++;
            q1 = q1->prox;
        }
    }
}