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

void shutdown(){
    int status;
    int qid = get_channel();

    printf("\nClosing channel...\n");
    delete_channel(qid);
    
    if(strcmp(option, FAT) == 0)
        for(int i = 0; i < FATCHILDS; i++){
            kill(getpid() + (i+1), SIGQUIT);
            wait(&status);                                              // Waits for all the children
        }
    else
        for(int i = 0; i < CHILDS; i++){
            kill(getpid() + (i+1), SIGQUIT);
            wait(&status);                                              // Waits for all the children
        }
        
    exit(0);
}

void delayed_scheduler(void *connections, int _managers){
    // Under Construction
    int qid, enviado, recebido, count;
    long type;
    msg_packet p, *q;
    _queue *queue;
    hyperTorus *ht;
    fTree *ft;

    signal(SIGINT, shutdown);                                           // Defines a signal treatment

    // if(strcmp(option, HYPER) == 0 || strcmp(option, TORUS) == 0){       // Gets the Hypercube or Torus structure
    //     ht = (hyperTorus*)connections;
    //     // readHyperTorus(ht);                                             // Debug only!
    // }
    // else{                                                               // Gets the Fat Tree structure
    //     ft = (fTree*)connections;
    //     // readTree(ft);                                                   // Debug only!
    // }

    printf("Attempting to create a msg queue...\n");
    qid = create_channel();                                             // Tries to open a message queue

    if(qid >= 0){                                                       // If message queue is open
        printf("Channel was created! Channel ID: %d\n", qid);
        createQueue(&queue);                                            // Creates a process queue for delayed execution

        while(1){
            msgrcv(qid, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0); // Receives the programs to be executed
            insertProcess(&queue, p.name, p.delay);                     // Inserts the program to be executed on the queue

            printf("Job #\tProgram\t\tDelay\n");
            printf("-----------------------------\n");
            listProcesses(queue);                                       // Prints the process queue
            
            for(int i = 0; i < _managers; i++){
                q = malloc(sizeof(msg_packet));
                q->type  = 0x2;
                strcpy(q->name, p.name);
                q->delay = p.delay;
                q->_mdst = i;
                q->ready = 0;
                q->finished = 0;

                enviado = msgsnd(qid, q, sizeof(msg_packet), 0);

                free(q);
            }

            count = 0;
            while(count < _managers){
                recebido = msgrcv(qid, &p, sizeof(msg_packet)-sizeof(long), 18, IPC_NOWAIT);
                if(recebido != -1){
                    printf("Processo #%d is ready to execute!\n", p._id);
                    count++;
                }
            }
            // Receber aviso de "ready" (montar fila de escalonamento FIFO)

            // Enviar ordem de execução!

            // Receber aviso de conclusão!

            // Repetir!
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