#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdlib.h> // for NULL

#include "sch/jobs.h"

extern int channel_id, do_exit;
extern job_node *job_l, *job_done_l;
extern int finished_c;
extern int job_id;
extern int g_exec_ord;

#endif