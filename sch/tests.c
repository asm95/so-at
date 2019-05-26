#include "jobs.h"

void print_jl(job_node *head){
    while (head){
        printf("(%ld,%s) -> ", head->sch_t, head->prog_name);
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