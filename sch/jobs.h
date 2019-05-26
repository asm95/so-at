#define MAX_PROG_NAME_JOBS 25

typedef struct job_list_t {
    long sch_t, exc_t, term_t;
    int job_id, delay, exc_ord;
    char prog_name[MAX_PROG_NAME_JOBS];
    struct job_list_t *next;
} job_node;


job_node *  new_jl();
job_node *  delete_jl(job_node *head);
job_node *  insert_jl(job_node *head, job_node *el);
void        push_front_jl(job_node **head, job_node *el);
job_node *  pop_front_jl(job_node **head);
job_node *  create_job(unsigned int sch_t, int delay, char *prog_name);
int         get_jl_sz(job_node *head);
