typedef struct {
    char *edges;
    unsigned int size;
    unsigned int node_c;

    int *dist;
    int *priority_list;
    int *prev;
} graph2D;

typedef enum {
    HYPER_C, TORUS, FAT_TREE
} to_types;

typedef struct to_proxy_t to_proxy;

graph2D* new_graph(unsigned int sz);
void free_graph(graph2D *g);

to_proxy    * topology_create(to_types t);
int         topology_init(to_proxy *el, unsigned int start);
int         * topology_query(to_proxy *el, unsigned int start, unsigned int end, int *arr_sz);
int         topology_search(to_proxy *el, unsigned int end, int *arr, int arr_sz);
void        topology_clear(to_proxy *el);