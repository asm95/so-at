#include <stdarg.h> // for va_list

typedef enum en_log_grp {
	// no class
    LOG_GRP_DEFAULT,
    // logs that are always active regardless user disable them    
    LOG_GRP_ALWAYS,
    LOG_GRP_TOPOLOGY_CHILD,
    // anything related to exchange of packages between nodes
    LOG_GRP_TOPOLOGY,
    // when workers (or nodes) acutally execute a binary program
    LOG_GRP_EXEC_PROG,
    // info messages that usually the user will want to check out
    LOG_GRP_CHILD_INFO,
    LOG_GRP_CHILD_VERBOSE,

    LOG_GRP_JOBS, // scheduler jobs
    LOG_GRP_MASTER_INFO, // includes shutdown
    LOG_GRP_MASTER_WAR, // warning messages
    LOG_GRP_MASTER_ERR // when job list is corrupted
} log_group;

int log_node(const char *node_label, const char *format, va_list args);
int log_node_grp(log_group grp_id, const char *node_label, const char *format, ...);

int log_init();
int log_set_enabled(log_group grp_id, int is_enabled);

#define LOG_GRP_COUNT 2
#define LOG_GRP_DISABLED 0
#define LOG_GRP_ENABLED 1

#define log_master(grp, ...) log_node_grp(grp, "  M", __VA_ARGS__)
#define log_master_war(...) log_node_grp(LOG_GRP_MASTER_WAR, " MW", __VA_ARGS__)
#define log_info(...)   log_node_grp(LOG_GRP_ALWAYS, "I", __VA_ARGS__)
#define log_war(...)    log_node_grp(LOG_GRP_ALWAYS, "W", __VA_ARGS__)
#define log_err(...)    log_node_grp(LOG_GRP_ALWAYS, "E", __VA_ARGS__)