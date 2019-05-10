#include "escalonador.h"

#define CHILDS 16
#define FATCHILDS 15
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

void definesTree(fTree **_tree, int _parent, int *_node, int _level){
    fTree *t1;
    int aux;

    t1 = malloc(sizeof(fTree));
    t1->id = *_node;
    t1->left = NULL;
    t1->right = NULL;

    aux = (*_node)++;

    if(_level < 3){
        t1->lenght = 3;
        t1->connects = malloc(sizeof(int)*(t1->lenght));

        t1->connects[0] = _parent;                                      // Connects to the parent
        t1->connects[1] = aux+1;                                        // Connects to the left child
        if(_level == 0)                                                 // Connects to the right child on level 1
            t1->connects[2] = aux+8;
        else if(_level == 1)                                            // Connects to the right child on level 2
            t1->connects[2] = aux+4;
        else                                                            // Connects to the right child on level 3
            t1->connects[2] = aux+2;
        
        (*_tree) = t1;                                                  // Sets the node
        _level++;                                                       // Prepares for the next level
        definesTree(&(*_tree)->left, aux, _node, _level);               // Builds to the left
        definesTree(&(*_tree)->right, aux, _node, _level);              // Builds to the left
    } else {
        t1->lenght = 1;
        t1->connects = malloc(sizeof(int));

        t1->connects[0] = _parent;
        (*_tree) = t1;
    }

    return;
}

void readTree(fTree *_tree){
    if(_tree != NULL){
        printf("NODE: %d\n", _tree->id);
        printf("Connects to: ");
        for(int i = 0; i < _tree->lenght; i++)
            printf("%d ", _tree->connects[i]);
        printf("\n");

        readTree(_tree->left);
        readTree(_tree->right);
    }
}

pid_t* get_fTreeConnection(fTree *_tree){
    pid_t *connections = NULL;

    if(_tree != NULL){
        if(_tree->id == getpid() - getppid() - 1){
            connections = malloc(sizeof(pid_t)*(_tree->lenght));
            for(int i = 0; i < _tree->lenght; i++)
                connections[i] = _tree->connects[i];
        } else {
            connections = get_fTreeConnection(_tree->left);
            if(connections != NULL)
                goto RETURN;
            connections = get_fTreeConnection(_tree->right);
        }
    }

    RETURN:
    return connections;
}

void definesHyper(hyperTorus **_hyper, int _id){
    hyperTorus *h1, *h2;
    int i, j, _xor;

    h1 = malloc(sizeof(hyperTorus));
    h1->id = _id;

    if(_id == 0){
        h1->length = 5;
        h1->connects = malloc(sizeof(pid_t)*(h1->length));
        h1->connects[4] = -1;
    } else{
        h1->length = 4;
        h1->connects = malloc(sizeof(pid_t)*(h1->length));
    }
    for(i = 0, j = 0; i < CHILDS; i++){                                 // Checks between all the children to see the correct connections
        if(i != _id){                                                   // Doesn't check itself
            _xor = _id^(i);                                             // XOR operation to check connection
            if((_xor == 1) || (_xor == 2) || (_xor == 4) || (_xor == 8))// Only 1-bit of difference: Connect
                h1->connects[j++] = i;                                  // Set the connection between 2 processes using the PID
        }
    }

    if((*_hyper) == NULL)
        *_hyper = h1;
    else{
        h2 = *_hyper;
        while(h2->next != NULL)
            h2 = h2->next;
        h2->next = h1;
    }
}

void readHyper(hyperTorus *_hyper){
    hyperTorus *h1;

    h1 = _hyper;
    while(h1 != NULL){
        printf("Process #%d is connected to: ", h1->id);
        for(int i = 0; i < h1->length; i++)
            printf("%d ", h1->connects[i]);
        printf("\n");

        h1 = h1->next;
    }
}

pid_t* get_hyperConnection(hyperTorus *_hyper){
    hyperTorus *h1;
    pid_t *connections;

    h1 = _hyper;
    while(h1 != NULL){
        if(h1->id == getpid() - getppid() - 1){
            connections = malloc(sizeof(pid_t)*(h1->length));
            for(int i = 0; i < h1->length; i++)
                connections[i] = h1->connects[i];

            break;
        }

        h1 = h1->next;
    }

    return connections;
}

void definesTorus(hyperTorus **_torus, int _id){
    hyperTorus *t1, *t2;
    int i, j;

    t1 = malloc(sizeof(hyperTorus));
    t1->id = _id;
    
    if(_id == 0)
        t1->length = 5;
    else
        t1->length = 4;

    t1->connects = malloc(sizeof(int)*(t1->length));

    j = 0;
    if(_id == 0)
        t1->connects[j++] = -1;                                         // Connects to the Scheduler

    if((_id%4) == 0 ){                                                  // The "left" nodes of the structure connect with:
        t1->connects[j++] = _id + 1;                                    // The nearest node; and
        t1->connects[j++] = _id + 3;                                    // The further node of its current level
    } else if(((_id%4) == 1) || ((_id%4) == 2)){                        // The "center" nodes of the structure connect with:
        t1->connects[j++] = _id - 1;                                    // The nearest node to its left; and
        t1->connects[j++] = _id + 1;                                    // The nearest node to its right on the same level
    } else {                                                            // The "right" nodes of the structure connect with:
        t1->connects[j++] = _id - 1;                                    // The nearest node to its left; and
        t1->connects[j++] = _id - 3;                                    // The further node to its left on the same level
    }

    if((_id-4) < 0)                                                     // The "top" nodes
        t1->connects[j++] = (_id+12)%16;                                // Connect with the bottom nodes on the same column
    else                                                                // The other nodes
        t1->connects[j++] = _id-4;                                      // Connect with the first top node to it
    
    if((_id+4) > 15)                                                    // The "bottom" nodes
        t1->connects[j] = (_id+4)%16;                                   // Connect with the bottom node on the same column
    else                                                                // The other nodes
        t1->connects[j] = _id+4;                                        // Connect with the first bottom node to it

    if((*_torus) == NULL)
        *_torus = t1;
    else{
        t2 = *_torus;
        while(t2->next != NULL)
            t2 = t2->next;
        t2->next = t1;
    }
}

void readTorus(hyperTorus *_torus){
    hyperTorus *t1;

    t1 = _torus;
    while(t1 != NULL){
        printf("Process #%d is connected to: ", t1->id);
        for(int i = 0; i < t1->length; i++)
            printf("%d ", t1->connects[i]);
        printf("\n");

        t1 = t1->next;
    }
}

pid_t* get_torusConnection(hyperTorus *_torus){
    hyperTorus *t1;
    pid_t *connections;

    t1 = _torus;
    while(t1 != NULL){
        if(t1->id == getpid() - getppid() - 1){
            connections = malloc(sizeof(pid_t)*(t1->length));
            for(int i = 0; i < t1->length; i++)
                connections[i] = t1->connects[i];

            break;
        }

        t1 = t1->next;
    }

    return connections;
}

void killproc(){
    int qid = open_channel();

    printf("Closing channel...\n");
    delete_channel(qid);
    exit(1);
}

void manager_process(int _id, pid_t *connections){
    // TODO
    sleep(5);
    exit(1);
}

void delayed_scheduler(){
    // Under Construction
    int qid;
    msg_packet p;
    _queue *queue;

    signal(SIGINT, killproc);

    printf("Attempting to create a msg queue...\n");
    qid = create_channel();

    if(qid >= 0){
        printf("Channel was created! Channel ID: %d\n", qid);
        createQueue(&queue);

        while(1){
            msgrcv(qid, &p, sizeof(msg_packet) - sizeof(long), 0x1, 0);
            insertProcess(&queue, p.name, p.delay);

            printf("Job #\tProgram\t\tDelay\n");
            printf("-----------------------------\n");
            listProcesses(queue);
            // system("pause");
            // system("clear");
        }
    }
    else
        printf("Error on creating a new channel...\n");
}

int main(int argc, char* argv[]){
    pid_t *connections, _parent;
    int   _struct, _fork, _id, _status, aux = 0;
    char  *option;
    fTree *ft = NULL;
    hyperTorus *ht = NULL;

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
                definesTree(&ft, -1, &aux, 0);                           // Defines the Fat Tree structure
                // readTree(ft);                                           // Just for debug!!
            } else {                                                    // Hypercube or Torus was selected. 16 children!
                _struct = CHILDS;                                       // Prepares for 16 manager processes
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

    if((_fork == 0) && (strcmp(option, HYPER) == 0)){
        connections = get_hyperConnection(ht);                          // Verify each process connections on the hypercube structure
        printf("Connections of %d: ", getpid() - getppid() - 1);
        for(int i = 0; i < 5; i++)
            printf("%d ", connections[i]);
        printf("\n");
        exit(0);
    } else if((_fork == 0) && (strcmp(option, TORUS) == 0)){
        connections = get_torusConnection(ht);                          // Verify each process connections on the torus structure
        printf("Connections of %d: ", getpid() - getppid() - 1);
        for(int i = 0; i < 5; i++)
            printf("%d ", connections[i]);
        printf("\n");
        exit(0);
    } else if((_fork == 0) && (strcmp(option, FAT) == 0)){
        connections = get_fTreeConnection(ft);                          // Creates and verifies the connections on the fat tree structure
        printf("Connections of %d: ", getpid() - getppid() - 1);
        for(int i = 0; i < 3; i++)
            printf("%d ", connections[i]);
        printf("\n");
        exit(0);
    }
    else{
        // delayed_scheduler();                                            // Calls the routine to wait for a program to be scheduled
        // sleep(5);                                                       // Temporary
        if((strcmp(option, HYPER) == 0) || (strcmp(option, TORUS) == 0))
            for(int i = 0; i < CHILDS; i++)
                wait(&_status);
        else
            for(int i = 0; i < FATCHILDS; i++)
                wait(&_status);
    }

    exit(0);
}