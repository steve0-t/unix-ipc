#ifndef PROC_T_H
#define PROC_T_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>

#include "./helpers.h"

#define PROC_NAME "Proc T"

void  convert_args_t(const char** argv, int* r2_pd, int* shm1_id, int* sem1_id);
char* attach_shm(int shm_id);
void  handler_t(int sig_num);

#endif
