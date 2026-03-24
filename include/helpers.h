#ifndef HELPERS_H
#define HELPERS_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>

#define PAGE_SIZE (sizeof(void *))
#define ALIGN_UP_POW2(n, p)                                                    \
  (((uint64_t)(n) + ((uint64_t)(p) - 1)) & (~((uint64_t)(p) - 1)))

#define BUFFER_SIZE ALIGN_UP_POW2(150, PAGE_SIZE)
#define IP_ADDR "127.0.0.1"
#define SA struct sockaddr

#define DEBUG 1

void FatalError(const char *msg, const char *proc_name);

// Read from file and write to pipe
// @param pipe_fd the writing end of pipe
// @param fd the file to read from
ssize_t write_to_pipe(const int pipe_fd, const int fd);

// Read up to char '\n'
// @param fd descriptor to read from
// @param buf to write to
// @param nbytes bytes to write
ssize_t read_line(int fd, char *buf, int nbytes);

// Simple helper function for converting argument to int
// @param argv the argument to convert
// @param res pointer to write to
int convert_arg(const char *argv, int *res);

// Helper function for performing semaphore operations
// @param sem_op the operation to perform
// @param sem_num which semaphore from set
// @param sem_id semaphore id
int sem_op_handler(int sem_id, unsigned short sem_num, short sem_op);

// Simple helper for converting arg to porn with error checks
int convert_port(const char *arg);

// Helper function for connecting to server
// @param port_no the port number of server
// @return server socket descriptor
int connect_to_server(const int port_no);

void setup_sig_handler(void (*f)(int));

#endif
