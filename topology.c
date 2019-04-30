#include <stdio.h>
#include <string.h> // for memset
#include <stdlib.h> // for malloc, free, exit

#include "common.h"
#include "topology.h"

/*
Há memória compartilhada onde há uma fila de mensagens para cada processo
Alguém deposita uma mensagem na fila
O gerenciador de processsos fica aguardando uma mensagem na fila de mensagens
ou o processo filho terminar

sem pipe e fila de mensagem

excelente lugar para procurar por man pages:
    https://www.die.net/

as ligações entre os nós pode ser modelada como um grafo
este grafo fica na memória compartilhada, assim todos processos filhos poderão
calcular uma rota
o grafo pode ser de matriz porque no máximo temos 16 vértices

*/

#define g_node(g, i,j) g->edges[i*g->size + j]

graph2D* new_graph(uint sz){
    graph2D *g = (graph2D*) malloc(sizeof(graph2D));
    if (g == NULL){
        return NULL;
    }
    // allocate edges
    const size_t edges_sof = sizeof(char) * sz*sz;
    char *edges = (char*) malloc(edges_sof);
    if (edges == NULL){
        free(g); return NULL;
    }

    // two vectors size sz
    // example of using the same malloc for multiple arrays
    uint *block = (uint*) malloc(sizeof(int) * sz*3);
    if (block == NULL){
        free(g); free(edges); return NULL;
    }

    // setting our vectors
    g->dist = block;
    g->priority_list = block + sz;
    g->prev = block + (sz*2);

    // 
    memset(edges, 0, edges_sof);

    g->size = sz;
    g->edges = edges;
    g->node_c = 0;

    return g;
}

void free_graph(graph2D *g){
    free(g->dist);
    free(g);
}

void print_graph(graph2D *g){
    for (int i=0; i<g->size; i++){
        for(int j=0; j<g->size; j++){
            printf("%u ", g_node(g, i,j));
        }printf("\n");
    }
}

void pl_print(int *vec, uint sz){
    for (int i=0; i<sz; i++){
        printf("%d ", vec[i]);
    }printf("\n");
}

uint pl_get_min(int *l, uint *sz){
    uint ret, pos = 0;

    ret = l[0]; // get the first element
    *sz += -1; // array size descreses one
    while (pos < *sz){ // all elements will shift left
        l[pos] = l[pos+1]; // current is next
        pos += 1; // walk cursor
    }
    l[pos] = -1; // illustration propouses

    return ret;
}

void pl_insert(int *l, uint *sz, uint el, uint *cmp){
    int pos;
    uint shift[2], shift_cur = 0;

    // first we need to check if element does not already exists
    // if does, remove it
    pos = *sz - 1;
    while(pos >= 0){
        if (l[pos] == el){
            // element already exists, then we'll shift all the others
            uint tmp_sz = *sz - pos;
            pl_get_min(&l[pos], &tmp_sz);
            *sz += -1; // removed the element
            break;
        }
        pos += -1;
    }

    pos = *sz;
    while(1){
        if (pos == 0){
            break;
        }
        if (cmp[l[pos-1]] < cmp[el]){
            break;
        }
        pos += -1;
    }
    if (pos < *sz){
        // shift elements to not override
        shift[shift_cur] = el;
        while(pos <= *sz){
            shift[1-shift_cur] = l[pos];
            l[pos] = shift[shift_cur];

            shift_cur = 1-shift_cur; // switch side
            pos += 1; // walk one
        }
    } else {
        l[pos] = el;
    }

    *sz += 1;
}

int pl_has_el(int *l, uint *sz){
    return (*sz > 0);
}

void dijkstra(graph2D *g, uint source){
    uint pl_size = 0; // our priority list size
    g->dist[source] = 0;

    for (uint nid = 0; nid < g->size; nid++){
        if (nid != source){
            g->dist[nid] = 1000; // this is our infinity!
        }
        g->prev[nid] = -1; // undefined
    }

    pl_insert(g->priority_list, &pl_size, source, g->dist);

    uint current, new_dist;
    while (pl_has_el(g->priority_list, &pl_size)){
        current = pl_get_min(g->priority_list, &pl_size);
        // iterate over all neighbors of current
        for(int el = 0; el < g->size; el++){
            if (g->edges[current*g->size + el] == 0){
                continue; // is not adjacent
            }
            //printf("%d is adjancent of %d\n", el, current);
            new_dist = g->dist[current] + 1;
            if (new_dist < g->dist[el]){
                g->dist[el] = new_dist;
                g->prev[el] = current;
                pl_insert(g->priority_list, &pl_size, el, g->dist);
                //pl_print(g->priority_list, pl_size);
            }
        }
    }
}

void build_route(graph2D *g, uint dest, int as_hex){
    uint cur = dest;
    while(1){
        if (as_hex && g->prev[cur] != -1){
            printf("Previous of %x is %x\n", cur, g->prev[cur]);
        } else {
            printf("Previous of %d is %d\n", cur, g->prev[cur]);
        }
        if (g->prev[cur] == -1){
            break;
        }
        cur = g->prev[cur];
    }
}

void build_fat_tree(graph2D *g, uint depth, uint cur_node){
    if (depth == 0){
        return;
    }
    uint left, right;
    uint next_child = g->node_c;
    left = next_child + 1;
    right = next_child + 2;

    g->node_c += 2;

    g_node(g, cur_node, left) = 1;
    g_node(g, left, cur_node) = 1;
    g_node(g, cur_node, right) = 1;
    g_node(g, right, cur_node) = 1;

    //printf("Children of %d are (%d,%d); depth %d\n", cur_node, left, right, depth);
    build_fat_tree(g, depth-1, left);
    build_fat_tree(g, depth-1, right);
}

void test_fat_tree(graph2D *g){
    /*
    id of root 0
    children of root 1 2
    children of 1: 3 4
    children of 2: 
    */

    build_fat_tree(g, 4-1, g->node_c);
    dijkstra(g, 9);
    build_route(g, 4, 0);
}

void build_hypercube(graph2D *g){
    uint id, xor_op;
    const char lim = 0xf;
    for (id = 0; id <= lim; id++){
        //printf("id is %02d (chr: %X)\n", id, id);
        xor_op = 0x1;
        while(xor_op <= 0x8){
            //printf("%x links to %x\n", id, id ^xor_op);
            g_node(g, id, id ^ xor_op) = 1;
            xor_op = xor_op << 1;
        }
    }
}

void test_hypercube(graph2D *g){
    build_hypercube(g);
    dijkstra(g, 5);
    build_route(g, 0xa, 1);
}

void build_torus(graph2D *g){
    int i,j, idx, adj;
    uint lim = g->size;
    lim = 4;
    for(i=0; i<lim; i++){
        for (j=0; j<lim; j++){
            idx = i*lim + j;
            adj = i*lim + ((j+1)%lim);
            //printf("node %d links to %d left\n", idx, adj);
            g_node(g, idx, adj) = 1; // left
            g_node(g, adj, idx) = 1; // left-inv
            adj = ((i+1) % lim) * lim + j;
            //printf("node %d links to %d down\n", idx, adj);
            g_node(g, idx, adj) = 1; // down
            g_node(g, adj, idx) = 1; // down-inv
        }
    }
}

void test_torus(graph2D *g){
    build_torus(g);
    dijkstra(g, 11);
    build_route(g, 5, 0);
}

typedef struct {
    int dest_id;
    int action;
} msg;

#define print_next_pl(list, l_size, cur_size) \
    printf("cur_lz: %d\n", cur_sz); \
    pl_print(list, lz)

#define print_insert_pl(list, l_size, el, cmp) \
    printf("Inserting %d\n", el); \
    pl_insert(list, l_size, el, cmp)

void test_pl (){
    const uint lz = 5;
    int list[5] = {-1, -1, -1, -1, -1};
    uint cmp[5] = {3,5,6,7,8};
    uint cur_sz = 0;
    pl_print(list, lz);
    print_insert_pl(list, &cur_sz, 4, cmp);
    print_next_pl(list, lz, cur_sz);
    print_insert_pl(list, &cur_sz, 3, cmp);
    print_next_pl(list, lz, cur_sz);
    print_insert_pl(list, &cur_sz, 2, cmp);
    print_next_pl(list, lz, cur_sz);
    pl_get_min(list, &cur_sz);
    print_next_pl(list, lz, cur_sz);
    pl_get_min(list, &cur_sz);
    print_next_pl(list, lz, cur_sz);
    // example when element already exists and chances priority
    printf("When element already exists\n");
    print_insert_pl(list, &cur_sz, 0, cmp);
    print_next_pl(list, lz, cur_sz);
    printf("Changing priority of %d to %d\n", 4, 2);
    cmp[4] = 2;
    print_insert_pl(list, &cur_sz, 0, cmp);
    print_next_pl(list, lz, cur_sz);
    // insert 4 again should be in the same place (beginning)
    print_insert_pl(list, &cur_sz, 4, cmp);
    print_next_pl(list, lz, cur_sz);
}

struct to_proxy_t {
    graph2D *nodes;
    to_types type;
};

to_proxy * topology_create(to_types topo_type){
    to_proxy * new = malloc(sizeof(to_proxy) * 1);
    if (new == NULL){
        return NULL;
    }

    new->nodes = new_graph(16);
    switch (topo_type){
        case HYPER_C:
            build_hypercube(new->nodes); break;
        case TORUS:
            build_torus(new->nodes); break;
        case FAT_TREE:
            build_fat_tree(new->nodes, 4-1, new->nodes->node_c); break;
        default: break;
    }

    new->type = topo_type;

    return new;
}

int * topology_query(to_proxy *el, uint start, uint end, int *arr_sz){
    dijkstra(el->nodes, start);

    // building path
    uint ant, cur = end;
    // getting an unused array
    int *arr = el->nodes->dist; // this is size of graph2D.size
    uint idx = 1; // starts on second element because 1rst is reserved
    uint arr_rsz = el->nodes->size - 1; // arr(ay)_(r)eal(s)i(z)e


    arr[arr_rsz] = cur; // first spot is reserverd
    while(1){
        ant = el->nodes->prev[cur];
        if (ant == -1){
            break;
        }
        arr[arr_rsz - idx] = ant;
        cur = ant;
        idx += 1;
    }

    *arr_sz = idx;

    return &arr[arr_rsz - idx + 1];
}

void topology_clear(to_proxy *el){
    free_graph(el->nodes);
    free(el);
}