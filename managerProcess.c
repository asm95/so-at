#include "managerProcess.h"

void manager_exit(){
    exit(0);                                                                                    // Terminates the manager
}

void manager_process(int _id, pid_t *connections, char *option){
    int msqid, aux, aux2, _fork, _status;
    msg_packet p, *q;
    time_t begin, end;

    signal(SIGQUIT, manager_exit);                                                              // Sets the routine when SIGQUIT is received

    msqid  = get_channel(MQ_SM);
    if(msqid < 0){
        printf("Error while getting the message qeue...\n");
        exit(0);
    }

    while(1){
        msgrcv(msqid, &p, sizeof(msg_packet)-sizeof(long), 0x2+_id, 0);                         // Executes a blocked receive, waits for some msg to arrive

        if(p._mdst != _id){                                                                     // If the message received isn't for this manager, redirects!
            if(p._mdst >= 0){                                                                   // Checks if the direction is Scheduler->Managers
                if(strcmp(option, "-t") == 0){                                                  // Torus calculus of the route
                    aux = p._mdst%4;                                                            // Checks the destination column on the Torus structure

                    aux2 = 0;
                    for(int i = 1; i < connections[0]+1; i++){                                  // Checks if the destination is on the same column as any of the connections
                        if(aux == (connections[i]%4))
                            aux2 = connections[i];
                        if(p._mdst == connections[i]){                                          // Checks if the destination is one of the connections
                            aux2 = connections[i];
                            break;
                        }
                    }
                    if(aux2 == 0){                                                              // If both check fail, sends the message to the node on the right
                        aux2 = _id+1;
                        if(aux2%4 == 0)                                                         // If the node is the last of the row, sends to the first of its row
                            aux2 = aux2/4;
                    }

                    aux = aux2;
                } else {                                                                        // Hypercube and Fat Tree calculus of the route
                    aux = 0;
                    for(int i = 1; i < connections[0]+1; i++){                                  // Checks if the nth-connection is less than the destination and different of -1
                        if(connections[i] < p._mdst && connections[i] != -1)
                            aux = connections[i];
                        if(p._mdst == connections[i]){                                          // Checks if the nth-connections is the destination
                            aux = connections[i];
                            break;
                        }
                    }
                }

                q = malloc(sizeof(msg_packet));                                                 // Allocates the message
                q->type = 0x2+aux;                                                              // Rewrites the message type to the type of the next node on the route
                strcpy(q->name, p.name);                                                        // Copies the program name
                q->delay = p.delay;                                                             // Copies the delay
                q->_mdst = p._mdst;                                                             // Copies the destination
                q->finish= p.finish;                                                            // Copies the finish flag

                // if(_id == 5)
                    // printf("[%d]  Redirecionando para %d\n", _id, aux);                         // Debug only!

                msgsnd(msqid, q, sizeof(msg_packet) - sizeof(long), 0);                         // Sends the message
                free(q);                                                                        // Frees the data of the message
            } else {                                                                            // The direction is Managers->Scheduler
                if(strcmp(option, "-t") == 0){                                                  // Torus calculus of the route
                    aux = 99;

                    for(int i = 1; i < connections[0]+1; i++){                                  // Checks if the nth-connection is on the same column as the node 0 and if the connection ID is less than its own
                        if(connections[i]%4 == 0 && connections[i] < _id)
                            aux = connections[i];
                        if(connections[i] == p._mdst){                                          // Checks if the nth-connection is the destination
                            aux = connections[i];
                            break;
                        }
                    }

                    if(aux == 99){                                                              // If both checks fail, sends the message to the node on the left
                        aux = _id - 1;
                    }

                    if(_id == 0)                                                                // If node is 0, sends to the scheduler
                        aux = -1;
                } else {                                                                        // Hypercube and Fat Tree calculus of the route
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){                                  // Checks if the nth-connection is less than "aux"
                        if(connections[i] < aux)
                            aux = connections[i];
                        if(connections[i] == p._mdst){                                          // Checks if the nth-connection is the destination
                            aux = connections[i];
                            break;
                        }
                    }
                }

                q = malloc(sizeof(msg_packet));                                                 // Allocates the message
                q->type  = 0x2+aux;                                                             // Rewrites the message type to the type of the next node on the route
                strcpy(q->name, p.name);                                                        // Copies the program name
                q->_mdst = p._mdst;                                                             // Copies the destination
                q->_id   = p._id;                                                               // Copies the ID
                q->finish= p.finish;                                                            // Copies the finish flag
                q->pid   = p.pid;                                                               // Copies the PID of the manager child
                q->begin = p.begin;                                                             // Copies the beginning of the childs execution time
                q->end   = p.end;                                                               // Copies the ending of the childs execution time

                // if(_id != 0)
                //     printf("[%d]  Redirecionando para %d\n", _id, aux);                         // Debug only!
                
                msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);                           // Sends the message
                free(q);                                                                        // Frees the data of the message
            }
        } else {                                                                                // If the message received is for this manager
            if(p.finish == 0){
                begin = time(NULL);                                                             // Starts the clock
                _fork = fork();                                                                 // Creates the child that will execute the program
                
                if(_fork == 0){                                                                 // Child tries to execute, if it fails, exits
                    if(execl(p.name, p.name, NULL) == -1)
                        exit(1);
                } else { 
                    wait(&_status);                                                             // Father waits for the end of the child
                    if(WEXITSTATUS(_status) == 1)                                               // If error on child execution, prints it
                        printf("Error on executing the program %s...\n", p.name);
                    end = time(NULL);                                                           // Stops the clock

                    q = malloc(sizeof(msg_packet));                                             // Allocates the message
                
                    aux = 99;
                    for(int i = 1; i < connections[0]+1; i++){                                  // Chooses to the send the message to the lowest ID connection
                        if(connections[i] < aux)
                            aux = connections[i];
                    }

                    q->type  = 0x2+aux;                                                         // Sets the type to the first node on the route
                    strcpy(q->name, p.name);                                                    // Copies the program name
                    q->_mdst = -1;                                                              // Writes the destination
                    q->_id   = _id;                                                             // Writes the manager ID
                    q->finish= 0;                                                               // Writes the finish flag
                    q->pid   = _fork;                                                           // Writes the child PID
                    q->begin = begin;                                                           // Writes the time that the execution begun
                    q->end   = end;                                                             // Writes the time that the execution ended

                    // printf("[%d]  Enviando resposta para %d\n", _id, aux);                   // Debug only!
                    
                    msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);                       // Sends the message
                    free(q);                                                                    // Frees the data of the message
                }
            } else {
                free(connections);                                                              // Frees the connections data
                exit(0);
            }
        }
    }
}