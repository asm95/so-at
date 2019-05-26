#include "../msg/msg.h" // for msg_packet
#include "../topology.h"

void    shutdown(int pid_vec[]);
void    parent_manager(to_proxy *topo, int *pid_vec);
int     route_walk(msg_packet *p); // required by children nodes