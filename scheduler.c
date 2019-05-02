#include "scheduler.h"

#define CHILDS 16
#define FATCHILDS 15
#define HYPER "-h"
#define TORUS "-t"
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

void create_connections(int _id, char* _option){
    pid_t _parent = getppid(), *connections;
    int _xor;
    int i, j, aux;
    // TODO
    if(strcmp(_option, FAT) == 0){
        printf("FAT TREE IS BEING ASSEMBLED!\n");
    } else if(strcmp(_option, HYPER) == 0){
        connections = malloc(sizeof(pid_t)*4);

        for(i = 0, j = 0; i < CHILDS; i++){                         // Checks between all the children to see the correct connections
            if(i != _id){                                           // Doesn't check itself
                _xor = _id^(i);                                     // XOR operation to check connection
                if((_xor == 1) || (_xor == 2) || (_xor == 4) || (_xor == 8)){   // Only 1-bit of difference: Connect
                    connections[j] = _parent + (i+1);               // Set the connection between 2 processes using the PID
                    j++;                                            // Prepares for the next connection
                }
            }
        }

        // Just for checking:
        printf("Process #%d\t(%d) is connected to: ", (getpid() - _parent)-1, getpid());
        for(i = 0; i < 4; i++)
            printf("%d ", (connections[i] - _parent)-1);
        printf("\n");
    } else {
        // printf("TORUS IS BEING ASSEMBLED!\n");
        // TODO
        connections = malloc(sizeof(pid_t)*4);

        j = 0;
        if((_id%4) == 0 ){                                          // The "left" nodes of the structure connect with:
            connections[j++] = _parent + (_id + 1);                 // The nearest node; and
            connections[j++] = _parent + (_id + 3);                 // The further node of its current level
        } else if(((_id%4) == 1) || ((_id%4) == 2)){                // The "center" nodes of the structure connect with:
            connections[j++] = _parent + (_id - 1);                 // The nearest node to its left; and
            connections[j++] = _parent + (_id + 1);                 // The nearest node to its right on the same level
        } else {                                                    // The "right" nodes of the structure connect with:
            connections[j++] = _parent + (_id - 1);                 // The nearest node to its left; and
            connections[j++] = _parent + (_id - 3);                 // The further node to its left on the same level
        }

        if((_id-4) < 0)                                             // The "top" nodes
            connections[j++] = _parent + ((_id+12)%16);             // Connect with the bottom nodes on the same column
        else                                                        // The other nodes
            connections[j++] = _parent + (_id-4);                   // Connect with the first top node to it
        
        if((_id+4) > 15)                                            // The "bottom" nodes
            connections[j] = _parent + ((_id+4)%16);                // Connect with the bottom node on the same column
        else                                                        // The other nodes
            connections[j] = _parent + (_id+4);                     // Connect with the first bottom node to it

        // Just for checking:
        printf("Process #%d\t(%d) is connected to: ", (getpid() - _parent)-1, getpid());
        for(i = 0; i < 4; i++)
            printf("%d ", (connections[i] - _parent));
        printf("\n");
    }
}

void manager_process(int id){
    // TODO
}

void delayed_scheduler(){
    // TODO
}

int main(int argc, char* argv[]){
    int _struct, _fork, _id;
    char *option;

    option = argv[1];                                               // Receives the Scheduler struct type

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
            if(strcmp(FAT, option) == 0){                           // Fat Tree structure was selected. 15 children!
                _struct = FATCHILDS;                                // Prepares for 15 manager processes
            } else {                                                // Hypercube or Torus was selected. 16 children!
                _struct = CHILDS;                                   // Prepares for 16 manager processes 
            }
        }
    }

    // child = malloc(sizeof(pid_t)*_struct);
    for(int i = 0; i < _struct; i++){                               // Creates the manager processes
        // TODO
        _fork = fork();
        if(_fork == 0){                                             // Child executes
            _id = i;                                                // Needed to create the proper structure
            break;                                                  // Childs won't create any processes
        } else if(_fork == -1){                                     // Error on fork
            // TODO
        }
    }

    if(_fork == 0)
        create_connections(_id, option);                           // Calls the routine to create the connections between processes
    else
        delayed_scheduler();                                        // Calls the routine to wait for a program to be scheduled

    return 0;
}