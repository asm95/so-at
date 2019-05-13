#include <stdlib.h>
#include <string.h>

#define MAX_PROG_NAME_JOBS 25

typedef struct job_list_t {
    unsigned int sch_t, term_t;
    int job_id, delay;
    char prog_name[MAX_PROG_NAME_JOBS];
    struct job_list_t *next;
} job_node;

job_node * new_jl(){
    return NULL;
}

job_node * delete_jl(job_node *head){
    job_node *cur;

    if (!head){
        return NULL;
    }

    cur = head->next;

    while (1){
        free(head);
        if (!cur){
            break;
        }
        head = cur;
        cur = cur->next;
    }

    return NULL;
}

job_node * insert_jl(job_node *head, job_node *el){
    if (! head){
        return el;
    }
    // will insert sorted
    job_node *cur, *ant = NULL;
    cur = head;
    while(1){
        if (el->sch_t < cur->sch_t){
            break;
        }

        ant = cur;
        if (cur->next == NULL){
            cur = NULL;
            break;
        }
        cur = cur->next;
    }

    if(cur == head){
        el->next = cur;
        return el;
    }

    ant->next = el;
    el->next = cur;

    return head;
}

void push_front_jl(job_node **head, job_node *el){
    el->next = *head;
    *head = el;
}

job_node * pop_front_jl(job_node **head){
    // the tip of list will point to the next element
    // the first element is then returned
    // it's job of the programmer to delete the element afterwareds with free

    job_node * el = *head;
    if (el != NULL){
        *head = (*head)->next;
    }
    return el;
}

int min(int a, int b){
    return (a<b) ? a : b;
}

job_node * create_job(unsigned int sch_t, int delay, char *prog_name){
    job_node *el = malloc(sizeof(job_node) * 1);
    if (! el){
        return NULL;
    }
    el->sch_t = sch_t;
    el->delay = delay;
    strncpy(el->prog_name, prog_name, MAX_PROG_NAME_JOBS-1);
    int zero_pos = min(strlen(prog_name), MAX_PROG_NAME_JOBS-2);
    el->prog_name[zero_pos] = '\0';
    el->next = NULL;

    return el;
}

int get_jl_sz(job_node *head){
    int sz = 0;
    while(head){
        sz += 1;
        head = head->next;
    }
    return sz;
}

void print_jl(job_node *head){
    while (head){
        printf("(%d,%s) -> ", head->sch_t, head->prog_name);
        head = head->next;
    }printf("\n");
}

void print_list_sz(job_node *head, char *list_name){
    printf("(I) len of %s: %d\n", list_name, get_jl_sz(head));
}

void test_jl(){
    char *list_name = "main_l";
    job_node *main_l, *el;

    main_l = new_jl();
    printf("(I) New job list should have length 0\n");
    print_list_sz(main_l, list_name);

    printf("(I) Inserting some elements in the list...\n");
    main_l = insert_jl(main_l, create_job(5, 0, "call-me-cool"));
    main_l = insert_jl(main_l, create_job(2, 0, "call-me-crazy"));
    main_l = insert_jl(main_l, create_job(6, 0, "whatever-works-for-u"));
    main_l = insert_jl(main_l, create_job(1, 0, "hey"));
    print_jl(main_l);
    print_list_sz(main_l, list_name);
    printf("(I) Removing a element from the list...\n");
    el = pop_front_jl(&main_l);
    free(el);
    print_jl(main_l);
    print_list_sz(main_l, list_name);
    printf("(I) Deleting the list...\n");
    main_l = delete_jl(main_l);
    print_list_sz(main_l, list_name);
}