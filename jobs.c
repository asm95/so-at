#include <stdlib.h>
#include <string.h>

#define MAX_PROG_NAME_JOBS 25

typedef struct job_list_t {
    int delay;
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
        if (el->delay < cur->delay){
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

int min(int a, int b){
    return (a<b) ? a : b;
}

job_node * create_job(int delay, char *prog_name){
    job_node *el = malloc(sizeof(job_node) * 1);
    if (! el){
        return NULL;
    }
    el->delay = delay;
    strncpy(el->prog_name, prog_name, MAX_PROG_NAME_JOBS-1);
    int zero_pos = min(strlen(prog_name), MAX_PROG_NAME_JOBS-2);
    el->prog_name[zero_pos] = '\0';
    el->next = NULL;
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
        printf("(%d,%s) -> ", head->delay, head->prog_name);
        head = head->next;
    }printf("\n");
}

void test_jl(){
    printf("(I) New job list should have length 0\n");
    job_node *main_l = new_jl();
    printf("(I) len of: %d\n", get_jl_sz(main_l));

    main_l = insert_jl(main_l, create_job(5, "call-me-cool"));
    main_l = insert_jl(main_l, create_job(2, "call-me-crazy"));
    main_l = insert_jl(main_l, create_job(6, "whatever-works-for-u"));
    main_l = insert_jl(main_l, create_job(1, "hey"));
    print_jl(main_l);
    printf("(I) len of: %d\n", get_jl_sz(main_l));
    main_l = delete_jl(main_l);
    printf("(I) len of: %d\n", get_jl_sz(main_l));
}