// for variadic functions
#include <stdio.h> // for sprintf, vprintf

#include "log.h"

// logging tools

int log_node(const char *node_label, const char *format, va_list args){
    static char print_fmt_buffer[256];
    int vprintf_rtrn;

    sprintf(print_fmt_buffer, "(%.3s) %.246s\n", node_label, format);
    vprintf_rtrn = vprintf(print_fmt_buffer, args);

    return vprintf_rtrn;
}

static int log_grp_enabled [LOG_GRP_COUNT];

int log_init(){
    // zero fill
    for (int i=0; i < LOG_GRP_COUNT; i++){
        log_grp_enabled[i] = 0;
    }
    log_grp_enabled[LOG_GRP_ALWAYS] = LOG_GRP_ENABLED;
    return 0;
}

int log_set_enabled(log_group grp_id, int is_enabled){
    if (grp_id == LOG_GRP_ALWAYS){
        return LOG_GRP_ENABLED;
    }

    log_grp_enabled[grp_id] = is_enabled;
    return is_enabled;
}

int log_node_grp(log_group grp_id, const char *node_label, const char *format, ...){
    int log_rtrn;

    if (log_grp_enabled[grp_id] != 1){
        return 0;
    }

    va_list args;
    va_start(args, format);
    log_rtrn = log_node(node_label, format, args);
    va_end(args);

    return log_rtrn;
}