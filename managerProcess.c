#include "managerProcess.h"

void manager_exit(){
    exit(0);
}

void manager_process(int _id, pid_t *connections, char *option){
    // TODO
    char *program;
    int  delay;
    int msqid, recebido, enviado, aux, aux2, _ndst, _fork, _status, _wait;
    msg_packet p, *q;

    signal(SIGQUIT, manager_exit);

    msqid  = get_channel(MQ_SM);
    if(msqid < 0){
        printf("Error while getting the message qeue...\n");
        exit(0);
    }

    recebido = -1;

    while(1){
        while(1){
            recebido = msgrcv(msqid, &p, sizeof(msg_packet)-sizeof(long), 0x2+_id, IPC_NOWAIT);
            if(recebido != -1)
                break;

            recebido = msgrcv(msqid, &p, sizeof(msg_packet)-sizeof(long), 19+_id, IPC_NOWAIT);
            if(recebido != -1)
                break;
        }

        if(p._mdst != _id){
            if(p.type == 0x2 + _id){
                if(strcmp(option, "-t") == 0){
                    aux = p._mdst%4;

                    aux2 = 0;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(aux == (connections[i]%4))
                            aux2 = connections[i];
                        if(p._mdst == connections[i]){
                            aux2 = connections[i];
                            break;
                        }
                    }
                    if(aux2 == 0){
                        aux2 = _id+1;
                        if(aux2%4 == 0)
                            aux2 = aux2/4;
                    }

                    aux = aux2;
                } else {
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i] < p._mdst && connections[i] != -1)
                            aux = connections[i];
                        if(p._mdst == connections[i]){
                            aux = connections[i];
                            break;
                        }
                    }
                }

                q = malloc(sizeof(msg_packet));
                q->type = 0x2+aux;
                strcpy(q->name, p.name);
                q->delay = p.delay;
                q->_mdst = p._mdst;
                q->_id = p._id;
                q->ready = p.ready;
                q->exec  = p.exec;
                q->finished = p.finished;
                
                enviado = msgsnd(msqid, q, sizeof(msg_packet) - sizeof(long), 0);
                free(q);
            } else if(p.type == 19 + _id){
                if(strcmp(option, "-t") == 0){
                    aux = 99;

                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i]%4 == 0 && connections[i] < _id)
                            aux = connections[i];
                        if(connections[i] == p._mdst){
                            aux = connections[i];
                            break;
                        }
                    }

                    if(aux == 99){
                        aux = _id - 1;
                    }

                    if(_id == 0)
                        aux = -1;
                } else {
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i] < aux)
                            aux = connections[i];
                        if(connections[i] == p._mdst){
                            aux = connections[i];
                            break;
                        }
                    }
                }

                q = malloc(sizeof(msg_packet));
                q->type = 19+aux;
                q->_mdst = p._mdst;
                q->_id = p._id;
                q->ready = p.ready;
                q->exec  = p.exec;
                q->finished = p.finished;

                enviado = msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                free(q);
            }
        } else {
            if(p.ready == 0){
                // printf("%d Receiving program to execute!\n", _id);
                program = p.name;
                delay = p.delay;

                q = malloc(sizeof(msg_packet));
                
                if(strcmp(option, "-t") != 0){
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i] < aux)
                            aux = connections[i];
                    }
                } else {
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i]%4 == 0 && connections[i] < _id)
                            aux = connections[i];
                        if(connections[i] == -1){
                            aux = connections[i];
                            break;
                        }
                    }

                    if(aux == 99){
                        aux = _id - 1;
                    }
                }

                q->type  = 19+aux;
                q->_mdst = -1;
                q->_id   = _id;
                q->ready = 1;
                q->exec  = p.exec;
                q->finished = p.finished;

                enviado = msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                free(q);
            }
            if(p.exec == 1){
                printf("%d: Order of execution received!\n", _id);
                _fork = fork();
                
                if(_fork == 0){
                    if(execl(program, program, NULL) == -1)
                        exit(1);
                } else {
                    _wait = wait(&_status);
                    if(WEXITSTATUS(_status) == 1)
                        printf("Error on executing the program %s...\n", program);

                    q = malloc(sizeof(msg_packet));
                
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i] < aux)
                            aux = connections[i];
                    }

                    q->type  = 19+aux;
                    q->_mdst = -1;
                    q->_id   = _id;
                    q->ready = 1;
                    q->exec  = 1;
                    q->finished = 1;

                    enviado = msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                    free(q);
                }
            }
        }
    }
}