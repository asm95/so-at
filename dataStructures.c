#include "dataStructures.h"

void createQueue(execq **queue){
    *queue = NULL;
}

void insertProcess(execq **queue, char *_name, int _delay){
    execq *q1, *q2;

    q1 = malloc(sizeof(execq));
    q1->name = malloc(sizeof(char)*strlen(_name));
    strcpy(q1->name, _name);
    q1->delay = _delay;
    q1->prox  = NULL;

    if(*queue == NULL)
        *queue = q1;
    else{
        q2 = *queue;

        while(q2->prox != NULL)
            q2 = q2->prox;
        q2->prox = q1;
    }
}

execq* removeProcess(execq **queue){
    execq *q1;

    if(*queue == NULL)
        return NULL;
    else{
        q1 = *queue;
        *queue = q1->prox;
        q1->prox = NULL;
    }

    return q1;
}

void listProcesses(execq *queue){
    int job = 0;
    execq *q1;

    if(queue == NULL)
        printf("No processes on execution queue...\n");
    else{
        q1 = queue;
        while(q1 != NULL){
            printf("%d\t%s\t%d\n", job, q1->name, q1->delay);
            job++;
            q1 = q1->prox;
        }
    }
}

void createFTree(fTree **_tree){
    *_tree = NULL;
}

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

void readTree(fTree *_tree){                                            // Reads the Fat Tree structure. Debug only!
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

pid_t* get_fTreeConnection(fTree *_tree){                               // Gets all the processes connections on the Fat Tree structure
    pid_t *connections = NULL;

    if(_tree != NULL){                                                  // If the node is not NULL
        if(_tree->id == getpid() - getppid() - 1){                      // Checks the PID against the ID
            connections = malloc(sizeof(pid_t)*(_tree->lenght)+1);      // Gets the number of connections
            connections[0] = _tree->lenght;                             // Saves the total connections of the node
            for(int i = 1, j = 0; i < _tree->lenght+1; i++, j++)
                connections[i] = _tree->connects[j];                    // Saves the nodes connections
        } else {                                                        // If PID != ID
            connections = get_fTreeConnection(_tree->left);             // Checks the left node
            if(connections != NULL)                                     // If the left node has PID == ID
                goto RETURN;                                            // Returns
            connections = get_fTreeConnection(_tree->right);            // Checks the right node
        }
    }

    RETURN:
    return connections;                                                 // Returns the node's connections
}

void createHyperTorus(hyperTorus **_ht){
    *_ht = NULL;
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

void readHyperTorus(hyperTorus *_ht){                                   // Reads the Hypercube/Torus structure. Debug only!
    hyperTorus *h1;

    h1 = _ht;
    while(h1 != NULL){
        printf("Process #%d is connected to: ", h1->id);
        for(int i = 0; i < h1->length; i++)
            printf("%d ", h1->connects[i]);
        printf("\n");

        h1 = h1->next;
    }
}

pid_t* get_htConnection(hyperTorus *_ht){                               // Gets all the processes connections on the Hypercube or Torus structure
    hyperTorus *ht1;
    pid_t *connections;

    ht1 = _ht;
    while(ht1 != NULL){
        if(ht1->id == getpid() - getppid() - 1){                        // Checks the PID against the ID of each node
            connections = malloc(sizeof(pid_t)*(ht1->length)+1);        // Gets the number of connections + 1
            connections[0] = ht1->length;                               // The first value is the number of connections
            for(int i = 1, j = 0; i < ht1->length+1; i++, j++)
                connections[i] = ht1->connects[j];                      // Saves each connection

            break;                                                      // Exits the while loop
        }

        ht1 = ht1->next;                                                // Advances to the next node if PID != ID
    }

    return connections;                                                 // Return the node's connections
}

void readConnections(pid_t *connections){                               // Reads all the nodes connections. Debug only!
    printf("Connections of %d: ", getpid() - getppid() - 1);
    for(int i = 1; i < connections[0]+1; i++)
        printf("%d ", connections[i]);
    printf("\n");
}

void createManQ(manq **_manq){
    *_manq = NULL;
}

void insertManQ(manq **_manq, int _id){
    manq *r1, *r2;

    r1 = malloc(sizeof(manq));
    r1->_id  = _id;
    r1->next = NULL;

    if(*_manq == NULL)
        *_manq = r1;
    else{
        r2 = *_manq;
        while(r2->next != NULL)
            r2 = r2->next;
        r2->next = r1;
    }
}

manq* removeManQ(manq **_manq){
    manq *r1;

    r1 = *_manq;
    *_manq = r1->next;

    return r1;
}