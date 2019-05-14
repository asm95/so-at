#include "escalonador.h"

#define HYPER "-h"
#define TORUS "-t"
#define FAT   "-f"

// #define _GNU_SOURCE

/*
 * This is an implementation of a Delayed Scheduler that uses the following data structures:
 * - Hypercube;
 * - Torus; and
 * - Fat-Tree.
 * 
 * The user can choose the type of structure that the processes will use to execute their code.
 * To select any of the available structures, use one of the following commands:
 * 
 * > scheduler.out -h    - For the hypercube;
 * > scheduler.out -t    - For the torus;
 * > scheduler.out -f    - For the fat-tree.
 * 
 * Just for experiment purposes, this implementation will execute an algorithm that finds valid
 * paths in a pre built graph.
 * Fifteen to sixteen processes will be generated in order to execute the algorithm for various
 * nodes on the graph. The scheduler will run on the background and the program will only show
 * the results of the generated processes.
 * 
 * Finally, the scheduler must be executed in the background. In order to do so, you must execute
 * the following command:
 * 
 * > scheduler.out -h | -t | -f &
 * >> The | symbol means OR
 */

char *option;
execq *eq;

void shutdown(){
    int status;
    int msgsmid = get_channel(MQ_SM);
    int msgsdid = get_channel(MQ_SD);

    printf("\nClosing Scheduler-Managers channel...\n");
    delete_channel(msgsmid);
    
    printf("\nClosing Scheduler-Delayed channel...\n");
    delete_channel(msgsdid);
    
    if(strcmp(option, FAT) == 0)
        for(int i = 0; i < FATCHILDS; i++){
            kill(getpid() + (i+1), SIGKILL);
            wait(&status);                                              // Waits for all the children
        }
    else
        for(int i = 0; i < CHILDS; i++){
            kill(getpid() + (i+1), SIGKILL);
            wait(&status);                                              // Waits for all the children
        }
        
    exit(0);
}

void new_schedule(){
    int recebido;
    int msgsdid = get_channel(MQ_SD);
    msg_packet p;

    recebido = msgrcv(msgsdid, &p, sizeof(msg_packet)-sizeof(long), 0x1, 0);

    insertProcess(&eq, p.name, p.delay);
    system("clear");
    printf("Job #\tProgram\t\tDelay\n");
    printf("-----------------------------\n");
    listProcesses(eq);                                                  // Prints the process queue
    printf("\n\n");
}

void send_pid(){
    int enviado;
    int msgsdid = get_channel(MQ_SD);
    pid_packet ppkg;

    ppkg.type = 0x1;
    ppkg.pid  = getpid();

    enviado = msgsnd(msgsdid, &ppkg, sizeof(pid_packet)-sizeof(long), 0);
}

void delayed_scheduler(void *connections, int _managers){
    // Under Construction
    int msgsmid, msgsdid, enviado, recebido, count;
    long type;
    msg_packet p, *q;
    pid_packet *ppkg;
    manq *_freep, *f1; 
    manq *_ready, *r1;

    signal(SIGINT, shutdown);                                           // Defines a SIGINT treatment
    signal(SIGUSR1, send_pid);                                          // Defines a SIGUSR1 treatment
    signal(SIGUSR2, new_schedule);                                      // Defines a SIGUSR2 treatment

    // printf("Attempting to create message queues...\n");

    msgsdid = create_channel(MQ_SD);                                    // Tries to open a Scheduler-Delayed queue
    if(msgsdid != -1){
        // printf("Schduler-Delayed channel was created! Channel ID: %d\n", msgsdid);
        ppkg = malloc(sizeof(pid_packet));
        ppkg->type = 0x1;
        ppkg->pid  = getpid();

        enviado = msgsnd(msgsdid, ppkg, sizeof(pid_packet)-sizeof(long), 0);
    }

    msgsmid = create_channel(MQ_SM);                                    // Tries to open a Scheduler-Manager queue
    // if(msgsmid != -1)
        // printf("Schduler-Manager channel was created! Channel ID: %d\n", msgsmid);

    createQueue(&eq);                                                   // Creates the queue of the programs to be executed
    createManQ(&_freep);                                                // Creates the queue for processes free to execute
    createManQ(&_ready);                                                // Creates the queue for processes ready for execution

    if(msgsmid >= 0){                                                   // If message queue is open
        for(int i = 0; i < _managers; i++)
            insertManQ(&_freep, i);

        while(1){
            if(eq != NULL){
                while(_freep != NULL){
                    f1 = removeManQ(&_freep);

                    q = malloc(sizeof(msg_packet));
                    q->type  = 0x2;
                    strcpy(q->name, eq->name);
                    q->delay = eq->delay;
                    q->_mdst = f1->_id;
                    q->_id = 0;
                    q->ready = 0;
                    q->exec  = 0;
                    q->finished = 0;

                    enviado = msgsnd(msgsmid, q, sizeof(msg_packet), 0);
                    free(f1);
                    free(q);
                }

                count = 0;
                while(count < _managers){
                    recebido = msgrcv(msgsmid, &p, sizeof(msg_packet)-sizeof(long), 18, 0);
                    if(recebido != -1){
                        insertManQSorted(&_ready, p._id);
                        printf("Process #%d is ready to execute!\n", p._id);
                        count++;
                    }
                }

                while(_ready != NULL){
                    r1 = removeManQ(&_ready);

                    q = malloc(sizeof(msg_packet));
                    q->type  = 0x2;
                    q->delay = 0;
                    q->_mdst = r1->_id;
                    q->_id = 0;
                    q->ready = 1;
                    q->exec  = 1;
                    q->finished = 0;

                    printf("Sending execution order to %d\n", q->_mdst);
                    enviado = msgsnd(msgsmid, q, sizeof(msg_packet)-sizeof(long), 0);
                    free(r1);
                    free(q);
                }

                count = 0;
                while(1){
                    recebido = msgrcv(msgsmid, &p, sizeof(msg_packet)-sizeof(long), 18, 0);
                    if(recebido != -1){
                        insertManQ(&_freep, p._id);
                        count++;
                    }
                    if(count == _managers){
                        removeProcess(&eq);
                        break;
                    }
                }
            } else
                pause();
        }
    }
    else
        printf("Error on creating a new channel...\n");
}

int main(int argc, char* argv[]){
    pid_t *connections, _parent;
    int   _struct, _fork, _id, _status, aux = 0;
    fTree *ft;
    hyperTorus *ht;

    option = argv[1];                                                   // Receives the Scheduler struct type
    _parent = getpid();                                                 // Receives the Scheduler PID

    /*
     * Checks if the scheduler was called with a argument.
     * If there was no argument defining the type of structure to be used, the program is finished.
     * If a argument was provided, the program needs to check if it's a valid argument. In case the
     * entered argument is invalid, the program is finished, otherwise, the program continues it's
     * execution.
     */
    if(option == NULL){
        printf("Run the scheduler with one of the three options: -h, -t or -f...\n");
        return 0;
    } else {
        if((strcmp(HYPER, option) != 0) && (strcmp(TORUS, option) != 0) && (strcmp(FAT, option) != 0)){
            printf("Invalid option! Try again with one of the three options: -h, -t or -f...\n");
            return 0;
        } else {
            if(strcmp(FAT, option) == 0){                               // Fat Tree structure was selected. 15 children!
                _struct = FATCHILDS;                                    // Prepares for 15 manager processes
                createFTree(&ft);
                definesTree(&ft, -1, &aux, 0);                           // Defines the Fat Tree structure
                // readTree(ft);                                           // Just for debug!!
            } else {                                                    // Hypercube or Torus was selected. 16 children!
                _struct = CHILDS;                                       // Prepares for 16 manager processes
                createHyperTorus(&ht);
                if(strcmp(option, TORUS) == 0){
                    for(int i = 0; i < CHILDS; i++)
                        definesTorus(&ht, i);                           // Defines the Torus structure
                    // readTorus(ht);                                      // Just for debug!!
                } else{
                    for(int i = 0; i < CHILDS; i++)
                        definesHyper(&ht, i);                           // Defines the Hypercube structure
                    // readHyper(ht);                                      // Just for debug!!
                }
            }
        }
    }

    for(int i = 0; i < _struct; i++){                                   // Creates the manager processes
        _fork = fork();
        if(_fork == 0)                                                  // Childs executes
            break;
        else if(_fork == -1){                                           // Error on fork
            printf("Error while creating a new process...\n");
            for(int j = 1; j <= i; j++)                                 // Loops through every process already created
                kill(getpid()+j, SIGKILL);                              // ... And kills it.
            exit(1);
        }
    }

    if(_fork == 0){
        if((strcmp(option, HYPER) == 0) || (strcmp(option, TORUS) == 0)){
            connections = get_htConnection(ht);                         // Verifies each process connections on the Hypercube/Torus structure
            // readConnections(connections);                               // Just debug!!
        } else {
            connections = get_fTreeConnection(ft);                      // Verifies each process connections on the Fat Tree structure
            // readConnections(connections);                               // Just debug!!
        }

        sleep(1);
        manager_process((getpid()-getppid()-1), connections, option);   // Manager routine
    } else{
        if((strcmp(option, HYPER) == 0) || (strcmp(option, TORUS) == 0))
            delayed_scheduler((void*)ht, _struct);                      // Calls the Delayed Scheduler Routine
        else
            delayed_scheduler((void*)ft, _struct);                      // Calls the Delayed Scheduler Routine
    }

    exit(0);
}