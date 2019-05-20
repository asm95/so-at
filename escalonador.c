#include "escalonador.h"

//! Define o argumento para estrutura Hypercube
#define HYPER "-h"
//! Define o argumento para estrutura Torus
#define TORUS "-t"
//! Define o argumento para estrutura Fat Tree
#define FAT   "-f"

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

//! Variável que guarda a opção de estrutura do escalonador
char *option;
//! Fila de jobs pra execução
execq *eq;
//! Fila de jobs que já foram executados
execd *ed;
//! Variável que guarda o tempo - em segundos - para disparar o alarme
int  _alarm;
//! Variável que guarda o ID da fila de mensagens entre escalonador e gerentes
int  msgsmid;
//! Variável que guarda a quantidade de gerentes para o escalonamento 
int _managers;
//! Fila de gerentes prontos para executar
manq *_ready;

void shutdown(){
    int status;
    int msgsmid = get_channel(MQ_SM);
    int msgsdid = get_channel(MQ_SD);

    printf("\nClosing Scheduler-Managers channel...\n");
    delete_channel(msgsmid);
    
    printf("\nClosing Scheduler-Delayed channel...\n");
    delete_channel(msgsdid);

    system("clear");

    if(eq != NULL){
        printf("The following won't execute:\n");
        listProcesses(eq);
        printf("\n");
        while(eq != NULL)
            removeProcess(&eq);
    }

    printf("Summary of execution:\n\n");
    listExecD(ed);
    deleteExecD(&ed);
    
    for(int i = 0; i < _managers; i++){
        kill(getpid() + (i+1), SIGQUIT);
        wait(&status);                                                  // Waits for all the children
    }
        
    exit(0);
}

void new_schedule(){
    int msgsdid = get_channel(MQ_SD);
    msg_packet p;

    msgrcv(msgsdid, &p, sizeof(msg_packet)-sizeof(long), 0x1, 0);
    printf("Program: %s\n", p.name);
    printf("Delay: %d\n", p.delay);

    if(eq == NULL){
        _alarm = p.delay;
        alarm(_alarm);
    }
    
    insertProcess(&eq, p.name, p.delay);
    updateDelays(&eq);
    printf("\n");
    listProcesses(eq);                                                  // Prints the process queue
    printf("\n\n");
    if(p.delay < _alarm){
        _alarm = p.delay;
        alarm(_alarm);
    }
    if(p.delay == 0)
        kill(getpid(), SIGALRM);
}

void send_pid(){
    int msgsdid = get_channel(MQ_SD);
    pid_packet ppkg;

    ppkg.type = 0x1;
    ppkg.pid  = getpid();

    msgsnd(msgsdid, &ppkg, sizeof(pid_packet)-sizeof(long), 0);
}

void execute_job(){
    int recebido, count;
    manq *r1;
    msg_packet p, *q;
    time_t begin, end;
    double makespan;

    count = 0;
    while(_ready != NULL){
        r1 = removeManQ(&_ready);

        q = malloc(sizeof(msg_packet));
        q->type  = 0x2;
        strcpy(q->name, eq->name);
        q->_mdst = r1->_id;
        q->_id   = r1->_id;
        // q->exec  = 1;

        msgsnd(msgsmid, q, sizeof(msg_packet)-sizeof(long), 0);
        free(r1);
        free(q);
    }

    while(1){
        recebido = msgrcv(msgsmid, &p, sizeof(msg_packet)-sizeof(long), 18, 0);

        insertExecD(&ed, p.pid, p.name, eq->sent, p.begin, p.end);

        if(count == 0)
            begin = time(NULL);
        if(recebido != -1){
            insertManQ(&_ready, p._id);
            count++;
        }
        if(count == _managers){
            end = time(NULL);
            makespan = difftime(end, begin);
            printf("\njob=%d,\tprogram=%s,\tdelay=%d,\tmakespan=%.0lf segundos\n\n", eq->job, eq->name, eq->rDelay, makespan);
            removeProcess(&eq);

            break;
        }
    }

    updateDelays(&eq);
    listProcesses(eq);
    printf("\n");
    if(eq != NULL){
        if(eq->uDelay != 0)
            alarm(eq->uDelay);
        if(eq->uDelay == 0)
            kill(getpid(), SIGALRM);
    }
}

void delayed_scheduler(int managers){
    _managers = managers;

    signal(SIGINT, shutdown);                                           // Defines a SIGINT treatment
    signal(SIGUSR1, send_pid);                                          // Defines a SIGUSR1 treatment
    signal(SIGUSR2, new_schedule);                                      // Defines a SIGUSR2 treatment
    signal(SIGALRM, execute_job);

    createQueue(&eq);                                                   // Creates the queue of the programs to be executed
    createExecD(&ed);
    createManQ(&_ready);                                                // Creates the queue for processes ready for execution

    msgsmid = get_channel(MQ_SM);
    if(msgsmid >= 0){                                                   // If message queue is open
        for(int i = 0; i < _managers; i++)                              // Initially all the managers are ready to execute
            insertManQ(&_ready, i);

        while(1){}                                                      // Executes a busy waiting for new jobs
    }
    else{
        printf("Error on creating a new channel...\n");
        shutdown();
    }
}

int main(int argc, char* argv[]){
    pid_t *connections, *pids;
    int   _struct, _fork, _id = 0 , aux = 0;
    int msgsdid, msgsmid;
    fTree *ft;
    hyperTorus *ht;
    pid_packet *ppkg;

    option = argv[1];                                                   // Receives the Scheduler struct type

    /*
     * Checks if the scheduler was called with a argument.
     * If there was no argument defining the type of structure to be used, the program is finished.
     * If a argument was provided, the program needs to check if it's a valid argument. In case the
     * entered argument is invalid, the program is finished, otherwise, the program continues it's
     * execution.
     */
    if(option == NULL){
        printf("Run the scheduler with one of the three options: -h, -t or -f...\n");
        exit(0);
    } else {
        if((strcmp(HYPER, option) != 0) && (strcmp(TORUS, option) != 0) && (strcmp(FAT, option) != 0)){
            printf("Invalid option! Try again with one of the three options: -h, -t or -f...\n");
            exit(0);
        } else {
            printf("Attempting to create message queue...\n");

            msgsdid = create_channel(MQ_SD);                            // Tries to open a Scheduler-Delayed queue
            if(msgsdid >= 0){
                printf("Scheduler-Delayed channel was created! Channel ID: %d\n", msgsdid);
                ppkg = malloc(sizeof(pid_packet));
                ppkg->type = 0x1;
                ppkg->pid  = getpid();
                msgsnd(msgsdid, ppkg, sizeof(pid_packet)-sizeof(long), 0);

                ppkg->type = 0x2;
                msgsnd(msgsdid, ppkg, sizeof(pid_packet)-sizeof(long), 0);
            }

            if(msgsdid < 0){
                printf("Error while creating the queues. Terminating execution...\n");
                exit(0);
            }

            msgsmid = create_channel(MQ_SM);
            if(msgsmid >= 0)
                printf("Scheduler-Manager channel was created! Channel ID: %d\n", msgsmid);
            
            if(msgsmid < 0 || msgsdid < 0){
                printf("Error while creating the queues. Terminating execution...\n");
                exit(0);
            }
            
            if(strcmp(FAT, option) == 0){                               // Fat Tree structure was selected. 15 children!
                _struct = FATCHILDS;                                    // Prepares for 15 manager processes
                createFTree(&ft);
                definesTree(&ft, -1, &aux, 0);                          // Defines the Fat Tree structure
                // readTree(ft);                                           // Just for debug!!
            } else {                                                    // Hypercube or Torus was selected. 16 children!
                _struct = CHILDS;                                       // Prepares for 16 manager processes
                createHyperTorus(&ht);
                if(strcmp(option, TORUS) == 0){
                    for(int i = 0; i < CHILDS; i++)
                        definesTorus(&ht, i);                           // Defines the Torus structure
                    // readHyperTorus(ht);                                 // Just for debug!!
                } else{
                    for(int i = 0; i < CHILDS; i++)
                        definesHyper(&ht, i);                           // Defines the Hypercube structure
                    // readHyperTorus(ht);                                 // Just for debug!!
                }
            }
        }
    }

    pids = malloc(sizeof(pid_t)*_struct);
    for(int i = 0; i < _struct; i++){                                   // Creates the manager processes
        _fork = fork();
        if(_fork == 0)                                                  // Childs executes
            break;
        else if(_fork > 0)
            _id++;                                                      // Increments the count for the next manager
        else {                                                          // Error on fork
            printf("Error while creating a new process...\n");
            for(int j = 0; j <= i; j++)                                 // Loops through every process already created
                kill(pids[j], SIGKILL);                                 // ... And kills it.
            exit(1);
        }
    }

    if(_fork == 0){
        if((strcmp(option, HYPER) == 0) || (strcmp(option, TORUS) == 0)){
            connections = get_htConnection(ht, _id);                   // Verifies each process connections on the Hypercube/Torus structure
            // readHTConnections(connections, _id);                       // Just debug!!
        } else {
            connections = get_fTreeConnection(ft, _id);                // Verifies each process connections on the Fat Tree structure
            // readFTConnections(connections, _id);                       // Just debug!!
        }

        manager_process(_id, connections, option);                     // Manager routine
    } else{
        delayed_scheduler(_struct);                                    // Calls the Delayed Scheduler Routine
    }

    exit(0);
}