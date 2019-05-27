int log_node(const char *node_label, const char *format, ...);

#define log_master(...) log_node("  M", __VA_ARGS__)
#define log_child(...)  log_node("  C", __VA_ARGS__)
#define log_info(...)   log_node("I", __VA_ARGS__)
#define log_war(...)    log_node("W", __VA_ARGS__)
#define log_err(...)    log_node("E", __VA_ARGS__)