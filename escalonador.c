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
//! Flag de finalização do sistema
int finish = 0;
//! Array com os PIDs de todos os gerentes
pid_t *pids;
//! Fila de gerentes prontos para executar
manq *_ready;

void shutdown(){
    system("clear");                                                    // Clears the screen
    finish = 1;

    if(eq != NULL){                                                     // If there are jobs on the queue, prints that they won't execute
        printf("The following won't execute:\n");
        listProcesses(eq);
        printf("\n");
        while(eq != NULL)
            removeProcess(&eq);                                         // Frees the data of the queue
    }

    printf("Summary of execution:\n\n");
    if(ed != NULL){                                                     // If any jobs were executed, prints them
        listExecD(ed);
        deleteExecD(&ed);                                               // Frees the data of the queue
    } else
        printf("No jobs were executed!\n");
}

void new_schedule(){
    int msgsjid = get_channel(MQ_SJ);
    int recebido;
    msg_packet *p;

    while(1){
        p = malloc(sizeof(msg_packet));
        recebido = msgrcv(msgsjid, p, sizeof(msg_packet)-sizeof(long), 0x1, IPC_NOWAIT);
        if(recebido != -1){
            if(eq == NULL){                                             // If there are no jobs on the queue
                _alarm = p->delay;                                      // Sets the _alarm to the job received
                alarm(_alarm);                                          // Sets a alarm with _alarm
            }
            
            insertProcess(&eq, p->name, p->delay);                      // Job is inserted to the queue
            if(eq->prox != NULL)
                updateDelays(&eq);                                      // Updates the delay if there are more than one job to be executed
            printf("\n");
            listProcesses(eq);                                          // Prints the process queue
            printf("\n\n");
            free(p);
        } else
            break;
    }
    if(eq->rDelay < _alarm){                                            // If the job being inserted has delay less then _alarm
        _alarm = eq->rDelay;                                            // _alarm is updated
        alarm(_alarm);                                                  // Sets a new alarm to _alarm
    }
    if(eq->rDelay == 0)                                                 // If the job being inserted has delay equal to 0
        kill(getpid(), SIGALRM);                                        // Starts execution right away
}

void send_pid(){
    int msgsdid = get_channel(MQ_SD);                                   // Gets the message queue ID
    pid_packet ppkg;

    ppkg.type = 0x1;                                                    // Sets the message type
    ppkg.pid  = getpid();                                               // Sends the Scheduler PID

    msgsnd(msgsdid, &ppkg, sizeof(pid_packet)-sizeof(long), 0);         // Sends the message
}

void execute_job(){
    int recebido, count;
    manq *r1;
    msg_packet p, *q;
    time_t begin, end;
    double makespan;

    count = 0;
    while(_ready != NULL){                                              // Removes a process of the "ready" queue
        r1 = removeManQ(&_ready);

        q = malloc(sizeof(msg_packet));                                 // Alocates the message
        q->type  = 0x2;                                                 // Sets message type
        strcpy(q->name, eq->name);                                      // Program name is sent with the message
        q->_mdst = r1->_id;                                             // Destination manager is sent with the message
        q->_id   = r1->_id;                                             // ID of the manager is sent with the message (Scheduler control)
        q->finish= 0;

        msgsnd(msgsmid, q, sizeof(msg_packet)-sizeof(long), 0);         // Message is sent
        free(r1);                                                       // Frees the data of the manager
        free(q);                                                        // Frees the data of the message
    }

    while(1){                                                           // Infinity loop while receiving messages
        recebido = msgrcv(msgsmid, &p, sizeof(msg_packet)-sizeof(long), 0x1, IPC_NOWAIT);
        if(recebido >= 0){
            insertExecD(&ed, p.pid, p.name, eq->sent, p.begin, p.end);      // Saves the execution data anwsered by the manager

            if(count == 0)                                                  // When the first manager anwsers, saves the time
                begin = p.begin;
            if(recebido != -1){                                             // If no errors are reported when receiving the answer
                insertManQ(&_ready, p._id);                                 // Manager goes back to the "ready" queue
                count++;                                                    // Manager's count is incremented
            }
            if(count == _managers){                                         // If Manager's count is equal to the number of Managers
                end = p.end;                                                // Saves the time
                makespan = difftime(end, begin);                            // Calculates the makespan
                printf("\njob=%d,\tprogram=%s,\tdelay=%d,\tmakespan=%.0lf segundos\n\n", eq->job, eq->name, eq->rDelay, makespan);
                removeProcess(&eq);                                         // Removes the job from the queue

                break;                                                      // Exits the loop
            }
        }
    }

    updateDelays(&eq);                                                  // After a job is done, all jobs have their delays updated
    listProcesses(eq);                                                  // Prints the queue of jobs left
    printf("\n");
    if(eq != NULL){                                                     // If the are jobs left
        if(eq->uDelay != 0)                                             // And the first job of the queue has delay different than "0"
            alarm(eq->uDelay);                                          // Sets the alarm to that jobs delay
        if(eq->uDelay == 0)                                             // If the first job of the queue has delay equal to 0
            kill(getpid(), SIGALRM);                                    // Starts the execution right away
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

        while(1){
            if(finish == 0){
                if(eq == NULL)                                          // If there are no jobs to execute
                    pause();                                            // Pause execution until a new job arrives
            }
            else
                return;
        }
    }
    else{
        printf("Error on opening a new channel...\n");
    }
}

/** \brief Escalonador de processos via estruturas Hypercube, Torus ou Fat Tree.
 *  
 *  A função principal do escalonador. Aqui ocorre toda a preparação para a execução do escalonador e de seus processos gerentes.
 *  A execução começa recebendo o parâmetro de estrutura (-h, -t ou -f). O arquivo "jobs.txt" é criado para guardar o contador de jobs
 *  enviados (o propósito é para utilização do programa "execucao_postergada"). Feito isso, os canais de comunicação são criados - um para
 *  comunicação com o programa "execucao_postergada" e "shutdown", outro para os processos gerentes e um terceiro exclusivo para envio dos jobs.
 *  O escalonador envia mensagem inicial, contendo seu PID, para comunicação, via signals, com "execucao_postergada" ou com o "shutdown". O escalonador 
 *  verifica o tipo de estrutura que foi escolhido para execução - hypercube, torus ou fat tree -, e cria os grafos ou a árvore simbólica dos nós gerentes.
 * 
 *  Após a criação das estruturas simbólicas, o escalonador realiza os N forks para criação dos processos gerentes. Para cada chamada fork
 *  o PID do filho é armazenado em um vetor, o "_id" é atualizado para o próximo filho e, caso alguma chamada fork ocasione erro, os processos
 *  que já tenham sido criados são terminados enviando um SIGKILL a eles.
 * 
 *  Os filhos criados executam uma função para receber, da estrutura simbólica, as conexões que eles fazem e, a partir dai, seguem para
 *  execução da função de gerenciamento. O pai segue para execução da função de escalonador postergado.
 * 
 *  Ao retornar da função de escalonamento (devido ao shutdown), o processo irá remover as estruturas simbólicas da memória, enviar uma mensagem
 *  a todos os gerentes para encerrarem a execução e esperar pelo exit dos mesmos. Por fim, as filas de mensagem serão removidas e o processo
 *  finalizado.
 * 
 *  \param argc; Quantidade de argumentos passados na CLI
 *  \param *argv[]; Argumentos passados na CLI
 */
int main(int argc, char* argv[]){
    FILE *file;
    pid_t *connections;
    int   _struct, _fork, _id = 0 , aux = 0;
    int msgsdid, msgsmid, msgsjid, status;
    fTree *ft;
    hyperTorus *ht;
    pid_packet *ppkg;
    msg_packet *q;

    option = argv[1];                                                   // Receives the Scheduler struct type
    file = fopen("jobs.txt", "w+");
    fputs("0", file);
    fclose(file);

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
            }

            if(msgsdid < 0){
                printf("Error while creating the queues. Terminating execution...\n");
                exit(0);
            }

            msgsmid = create_channel(MQ_SM);
            if(msgsmid >= 0)
                printf("Scheduler-Manager channel was created! Channel ID: %d\n", msgsmid);

            msgsjid = create_channel(MQ_SJ);
            if(msgsmid >= 0)
                printf("Scheduler-Jobs channel was created! Channel ID: %d\n", msgsjid);
            
            if(msgsmid < 0 || msgsdid < 0 || msgsjid < 0){
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
                    // readHyperTorus(ht);                                // Just for debug!!
                } else{
                    for(int i = 0; i < CHILDS; i++)
                        definesHyper(&ht, i);                           // Defines the Hypercube structure
                    // readHyperTorus(ht);                                // Just for debug!!
                }
            }
        }
    }

    pids = malloc(sizeof(pid_t)*_struct);
    for(int i = 0; i < _struct; i++){                                   // Creates the manager processes
        _fork = fork();
        if(_fork == 0)                                                  // Childs executes
            break;
        else if(_fork > 0){
            pids[i] = _fork;
            _id++;                                                      // Increments the count for the next manager
        }
        else {                                                          // Error on fork
            printf("Error while creating a new process...\n");
            for(int j = 0; j <= i; j++)                                 // Loops through every process already created
                kill(pids[j], SIGKILL);                                 // ... And kills it.
            exit(1);
        }
    }

    if(_fork == 0){
        if((strcmp(option, HYPER) == 0) || (strcmp(option, TORUS) == 0)){
            connections = get_htConnection(ht, _id);                    // Verifies each process connections on the Hypercube/Torus structure
            // readHTConnections(connections, _id);                       // Just debug!!
        } else {
            connections = get_fTreeConnection(ft, _id);                 // Verifies each process connections on the Fat Tree structure
            // readFTConnections(connections, _id);                       // Just debug!!
        }

        manager_process(_id, connections, option);                      // Manager routine
    } else{
        delayed_scheduler(_struct);                                     // Calls the Delayed Scheduler Routine
    }

    if(strcmp(option, FAT) == 0)
        deleteTree(&ft);                                                // Deletes the simbolic Fat Tree structure
    else
        deleteHyperTorus(&ht);                                          // Deletes the simbolic Hypercube/Torus structure

    for(int i = _managers - 1; i >= 0; i--){                            // Loops through the managers IDs
        q = malloc(sizeof(msg_packet));                                 // Allocates the message
        q->type  = 0x2;                                                 // Sets the type for Manager 0
        q->_mdst = i;                                                   // Sets the destination Manager
        q->finish= 1;                                                   // Writes the finish flag

        msgsnd(msgsmid, q, sizeof(msg_packet)-sizeof(long), 0);         // Sends the message
        wait(&status);                                                  // Waits for the nth-Manager to be shutdown
        free(q);                                                        // Frees the message data
    }

    printf("\nClosing Scheduler-Managers channel...\n");
    delete_channel(get_channel(MQ_SM));                                 // Closes the first message queue
    
    printf("\nClosing Scheduler-Delayed channel...\n");
    delete_channel(get_channel(MQ_SD));                                 // Closes the second message queue

    printf("\nClosing Scheduler-Jobs channel...\n");
    delete_channel(get_channel(MQ_SJ));                                 // Closes the third message queue

    exit(0);
}