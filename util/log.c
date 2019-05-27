// for variadic functions
#include <stdio.h> // for sprintf, vprintf
#include <stdarg.h>

#include "log.h"

// logging tools

int log_node(const char *node_label, const char *format, ...){
    static char print_fmt_buffer[256];
    int vprintf_rtrn;

    va_list args;
    va_start(args, format);
    sprintf(print_fmt_buffer, "(%.3s) %.246s\n", node_label, format);
    vprintf_rtrn = vprintf(print_fmt_buffer, args);
    va_end(args);

    return vprintf_rtrn;
}