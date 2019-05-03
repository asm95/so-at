#include "scheduler.h"

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

void create_connections(int _id, char* _option){
    pid_t _parent, *connections;
    int _xor;
    int i, j, aux;

    _parent = getppid();
    if(_id == 0){
        connections = malloc(sizeof(pid_t)*5);
        connections[4] = _parent;
    }
    else
        connections = malloc(sizeof(pid_t)*4);
    if(strcmp(_option, HYPER) == 0){
        for(i = 0, j = 0; i < CHILDS; i++){                             // Checks between all the children to see the correct connections
            if(i != _id){                                               // Doesn't check itself
                _xor = _id^(i);                                         // XOR operation to check connection
                if((_xor == 1) || (_xor == 2) || (_xor == 4) || (_xor == 8)){   // Only 1-bit of difference: Connect
                    connections[j] = _parent + (i+1);                   // Set the connection between 2 processes using the PID
                    j++;                                                // Prepares for the next connection
                }
            }
        }

        // Just for checking:
        printf("Process #%d\t(%d) is connected to: ", (getpid() - _parent)-1, getpid());
        for(i = 0; i < 4; i++)
            printf("%d ", (connections[i] - _parent)-1);
        printf("\n");
    } else {
        j = 0;

        if((_id%4) == 0 ){                                              // The "left" nodes of the structure connect with:
            connections[j++] = _parent + (_id + 1);                     // The nearest node; and
            connections[j++] = _parent + (_id + 3);                     // The further node of its current level
        } else if(((_id%4) == 1) || ((_id%4) == 2)){                    // The "center" nodes of the structure connect with:
            connections[j++] = _parent + (_id - 1);                     // The nearest node to its left; and
            connections[j++] = _parent + (_id + 1);                     // The nearest node to its right on the same level
        } else {                                                        // The "right" nodes of the structure connect with:
            connections[j++] = _parent + (_id - 1);                     // The nearest node to its left; and
            connections[j++] = _parent + (_id - 3);                     // The further node to its left on the same level
        }

        if((_id-4) < 0)                                                 // The "top" nodes
            connections[j++] = _parent + ((_id+12)%16);                 // Connect with the bottom nodes on the same column
        else                                                            // The other nodes
            connections[j++] = _parent + (_id-4);                       // Connect with the first top node to it
        
        if((_id+4) > 15)                                                // The "bottom" nodes
            connections[j] = _parent + ((_id+4)%16);                    // Connect with the bottom node on the same column
        else                                                            // The other nodes
            connections[j] = _parent + (_id+4);                         // Connect with the first bottom node to it

        // Just for checking:
        printf("Process #%d\t(%d) is connected to: ", (getpid() - _parent)-1, getpid());
        for(i = 0; i < 4; i++)
            printf("%d ", (connections[i] - _parent));
        printf("\n");
    }
}

void create_fat_tree(pid_t _ppid, int _level){
    // TODO
    int id, _fork, i = 0;
    pid_t *connections;

    id = getpid() - _ppid - 1;

    if(_level == 0){                                                    // Defines the number of connections for nodes at level 0
        connections = malloc(sizeof(pid_t)*5);
        connections[i++] = getppid();                                   // Connects to the Scheduler
    }
    else if(_level == 1)                                                // Defines the number of connections for nodes at level 1
        connections = malloc(sizeof(pid_t)*4);
    else if(_level == 2)                                                // Defines the number of connections for nodes at level 2
        connections = malloc(sizeof(pid_t)*3);
    else                                                                // Defines the number of connections for nodes at level 3
        connections = malloc(sizeof(pid_t)*1);

    if(_level < 3){
        _fork = fork();                                                 // Creates the left node
        if(_fork == 0)                                                  // Child executes
            goto FORK;
        else{                                                           // Parent executes
            if(_level == 0){
                connections[i++] = _fork;                               // Connects to the first child
                connections[i++] = _fork;                               // Twice
            } else if(_level == 1){
                connections[i++] = getppid();                           // Connects to the parent
                connections[i++] = _fork;                               // Connects to the first child
            } else {
                connections[i++] = getppid();                           // Connects to the parent
                connections[i++] = _fork;                               // Connects to the first child
            }
        }
        
        _fork = fork();                                                 // Creates the right node
        if(_fork == 0)                                                  // Child executes
            goto FORK;
        else{                                                           // Parent executes
            if(_level == 0){
                connections[i++] = _fork;                               // Connects to the second child
                connections[i++] = _fork;                               // Twice
            } else if(_level == 1){
                connections[i++] = getppid();                           // Connects to the parent
                connections[i++] = _fork;                               // Connects to the second child
            } else {
                connections[i++] = _fork;                               // Connects to the second child
            }

            // Just checking
            printf("Process #%d\t(%d) is connected to: ", id, getpid());
            if(_level == 0){
                for(int j = 0; j < 5; j++)
                    printf("%d (%d) ", connections[j], (connections[j] - _ppid) -1);
                printf("\n");
            } else if(_level == 1){
                for(int j = 0; j < 4; j++)
                    printf("%d (%d) ", connections[j], (connections[j] - _ppid) -1);
                printf("\n");
            } else {
                for(int j = 0; j < 3; j++)
                    printf("%d (%d) ", connections[j], (connections[j] - _ppid) -1);
                printf("\n");
            }
            goto ENDCONNECTION;
        }

        FORK:
        _level++;
        create_fat_tree(_ppid, _level);                                 // Connects the left node of the next level
    } else {
        connections[i++] = getppid();

        // Just checking
        printf("Process #%d\t(%d) is connected to: ", id, getpid());
        for(int j = 0; j < 1; j++)
            printf("%d (%d) ", connections[j], (connections[j] - _ppid));
        printf("\n");
    }

    ENDCONNECTION:
    return;
}

void manager_process(int _id, pid_t *connections){
    // TODO
}

void delayed_scheduler(){
    // TODO
}

int main(int argc, char* argv[]){
    int _struct, _fork, _id;
    char *option;

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
        return 0;
    } else {
        if((strcmp(HYPER, option) != 0) && (strcmp(TORUS, option) != 0) && (strcmp(FAT, option) != 0)){
            printf("Invalid option! Try again with one of the three options: -h, -t or -f...\n");
            return 0;
        } else {
            if(strcmp(FAT, option) == 0){                               // Fat Tree structure was selected. 15 children!
                _struct = FATCHILDS;                                    // Prepares for 15 manager processes
            } else {                                                    // Hypercube or Torus was selected. 16 children!
                _struct = CHILDS;                                       // Prepares for 16 manager processes 
            }
        }
    }

    if((strcmp(option, FAT) != 0)){
        for(int i = 0; i < _struct; i++){                               // Creates the manager processes
            _fork = fork();
            if(_fork == 0){                                             // Child executes
                _id = i;                                                // Needed to create the proper structure
                break;                                                  // Childs won't create any processes
            } else if(_fork == -1){                                     // Error on fork
                // TODO
            }
        }
    } else {
        _fork = fork();                                                 // Creates the first node of the Fat Tree
    }

    if((_fork == 0) && (strcmp(option, FAT) != 0))
        create_connections(_id, option);                                // Calls the routine to create the connections between processes
    else if((_fork == 0) && (strcmp(option, FAT) == 0)){
        _id = 0;
        create_fat_tree(getppid(), 0);                                  // 0 = _id of the first process; 0 = _level of the first node
        sleep(5);
    }
    else
        // delayed_scheduler();                                         // Calls the routine to wait for a program to be scheduled
        sleep(5);                                                       // Temporary

    return 0;
}