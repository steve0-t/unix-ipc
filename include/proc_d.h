#ifndef PROC_T_H
#define PROC_T_H

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./helpers.h"

#define PROC_NAME "Proc D"

int   convert_port(const char* arg);
char* attach_shm(int shm_id);
int   connect_to_server(const int port_no);
void  send_to_server(const int sock_fd, const char* buffer);
void  handler_d(int sig_num);
void  convert_args_d(const char** argv, int* shm2_id, int* sem2_id, int* port_no);

#endif
