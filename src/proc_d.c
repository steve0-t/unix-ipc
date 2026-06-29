#include "../include/proc_d.h"
#include <stdio.h>

static void* shared_mem;

// Main
int main(int argc, const char* argv[])
{
    if (argc != 4)
    {
        kill(getppid(), SIGTERM);
        FatalError("Incorrect number of arguments", PROC_NAME);
    }

    setup_sig_handler(handler_d);

    int shm2_id = 0, sem2_id = 0; // System V stuff
    int port_no = 0, sock_fd = 0; // TCP server stuff
    convert_args_d(argv, &shm2_id, &sem2_id, &port_no);

    if ((sock_fd = connect_to_server(port_no)) < 0)
    {
        kill(getppid(), SIGTERM);
        FatalError("Could not get socket", PROC_NAME);
    }

    char* shm_buffer = attach_shm(shm2_id);
    char  buffer[BUFFER_SIZE + 1];

    kill(getppid(), SIGUSR1);

    while (1)
    {
#if DEBUG
        printf("%s: waiting on sem\n", PROC_NAME);
        fflush(stdout);
#endif
        if (sem_op_handler(sem2_id, 1, -1) < 0)
        {
            kill(getppid(), SIGTERM);
            FatalError("Error when changing sem value", PROC_NAME);
        }
        memcpy(buffer, shm_buffer, BUFFER_SIZE);
#if DEBUG
        fprintf(stdout, "%s: read from shm: %s\n", PROC_NAME, buffer);
        fflush(stdout);
#endif
        if (sem_op_handler(sem2_id, 0, 1) < 0)
        {
            kill(getppid(), SIGTERM);
            FatalError("Error when changing sem value", PROC_NAME);
        }
        send_to_server(sock_fd, buffer);
        sleep(1);
        memset(buffer, 0, BUFFER_SIZE);
    }
    return EXIT_SUCCESS;
}

void handler_d(int sig_num)
{
    if (shared_mem != NULL)
    {
        if (shmdt(shared_mem) < 0)
        {
            kill(getppid(), SIGTERM);
            write(1, "Could not detach shm\n", 22);
        }
    }
    _exit(sig_num);
}

void convert_args_d(const char** argv, int* shm2_id, int* sem2_id, int* port_no)
{
    if (convert_arg(argv[1], shm2_id) < 0) goto error;
    if (convert_arg(argv[2], sem2_id) < 0) goto error;
    if ((*port_no = convert_port(argv[3])) < 0) goto error;
    return;
error:
    kill(getppid(), SIGTERM);
    FatalError("Could not convert args", PROC_NAME);
}

char* attach_shm(int shm_id)
{
    shared_mem = shmat(shm_id, NULL, 0);
    if (shared_mem == MAP_FAILED)
    {
        kill(getppid(), SIGTERM);
        FatalError("Could not attach shared mem", PROC_NAME);
    }
    return (char*)shared_mem;
}

void send_to_server(const int sock_fd, const char* buffer)
{
    size_t len = (size_t)strlen(buffer) + 1;
    if (send(sock_fd, buffer, len, 0) < 0)
    {
        kill(getppid(), SIGTERM);
        FatalError("Could not send message to server", PROC_NAME);
    }
}
