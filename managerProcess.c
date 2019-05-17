#include "managerProcess.h"

void manager_exit(){
    exit(0);
}

void manager_process(int _id, pid_t *connections, char *option){
    char *program;
    int  delay;
    int msqid, recebido, aux, aux2, _ndst, _fork, _status, _wait;
    int shmid, *child;
    msg_packet p, *q;
    time_t begin, end;

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
                q->exec  = p.exec;
                
                msgsnd(msqid, q, sizeof(msg_packet) - sizeof(long), 0);
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
                q->type  = 19+aux;
                strcpy(q->name, p.name);
                q->_mdst = p._mdst;
                q->_id   = p._id;
                q->exec  = p.exec;
                q->pid   = p.pid;
                q->begin = p.begin;
                q->end   = p.end;

                msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                free(q);
            }
        } else {
            if(p.exec == 1){
                _fork = fork();
                shmid = shmget(SHMID, sizeof(pid_t), IPC_CREAT);
                
                if(_fork == 0){
                    // shmid = shmget(SHMID, sizeof(pid_t), S_IWUSR);
                    
                    // child = (int*) shmat(shmid, (void*)0, 0);
                    // child = malloc(sizeof(int));

                    if(execl(p.name, p.name, NULL) == -1)
                        exit(1);
                } else {
                    begin = time(NULL);
                    _wait = wait(&_status);
                    if(WEXITSTATUS(_status) == 1)
                        printf("Error on executing the program %s...\n", p.name);
                    end = time(NULL);

                    q = malloc(sizeof(msg_packet));
                
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){
                        if(connections[i] < aux)
                            aux = connections[i];
                    }

                    q->type  = 19+aux;
                    strcpy(q->name, p.name);
                    q->_mdst = -1;
                    q->_id   = _id;
                    q->exec  = 0;
                    q->pid   = _fork;
                    q->begin = begin;
                    q->end   = end;

                    msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                    free(q);
                }
            }
        }
    }
}