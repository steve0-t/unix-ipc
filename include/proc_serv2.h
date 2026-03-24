#ifndef UDP_SERVER
#define UDP_SERVER

#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./helpers.h"

#define PROC_NAME "Proc serv2"
#define OUT_FILE  "serv2.txt"

void convert_args_serv2(const char** argv, int* udp_port);
int  setup_udp_serv(const int port_no);
void handler_serv2(int sig_num);

#endif
