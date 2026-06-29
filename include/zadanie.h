#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <bits/types/sigset_t.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "./helpers.h"

#define PROC_NAME "Conductor"

#define SHM_KEY_1 0xff12ab34
#define SHM_KEY_2 0xff56ab78
#define SHM_SIZE  (BUFFER_SIZE * 2)

#define SEM_KEY_1 0xff12cd34
#define SEM_KEY_2 0xff56cd78
#define N_SEMS    2

#define TIME_TO_WAIT 15

// Enum containing processes to launch
enum Program
{
    P1 = 0,
    P2,
    PR,
    T,
    S,
    D,
    SERV1,
    SERV2,
    PROGRAM_COUNT,
};

// To make pipe ops more sensible
enum Pipe_ends
{
    READ  = 0,
    WRITE = 1
};

typedef struct
{
    int   pd_1[2];
    int   pd_2[2];
    void* shm_addr1;
    void* shm_addr2;
    int   shm_id1;
    int   shm_id2;
    int   sem_id1;
    int   sem_id2;
} Ipc_resources;

typedef struct
{
    const char* name;
    const char* pathname;
} ProgramInfo;

union semun
{
    int              val;
    struct semid_ds* buf;
    unsigned short*  array;
};

void initialize_mask();
int  execute_program(int program, char* const args[], char* const env[]);
int  launch_program(const int which, const int argc, const int args[]);
int  launch_server(const int which, const char* port1, const char* port2);
void create_shm_and_sem();
int  create_shared_mem(void** mem_addr, key_t key);
int  create_semaphore(key_t key);
void initialize_sems(int sem_id);
void cleanup_ipc_resources();
void cleanup_shared_mem();
void cleanup_sempahores();
int  close_pipe(int pd[2]);
void handler_main(int sig_num);

#endif
