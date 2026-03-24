#include "../include/proc_t.h"

static void* shared_mem;

// Main
int main(int argc, const char* argv[]) {
    if (argc != 4) {
        kill(getppid(), SIGTERM);
        FatalError("Incorrect number of arguments", PROC_NAME);
    }

    setup_sig_handler(handler_t);

    int r2_pd = 0, shm1_id = 0, sem1_id = 0;
    convert_args_t(argv, &r2_pd, &shm1_id, &sem1_id);

    char*   shm_buffer = attach_shm(shm1_id);

    ssize_t n = 0;
    char    buffer[BUFFER_SIZE + 1];

    kill(getppid(), SIGUSR1);
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = read_line(r2_pd, buffer, sizeof(buffer));
#if DEBUG
        fprintf(stdout, "%s: read from pipe: %s\n", PROC_NAME, buffer);
        fflush(stdout);
#endif
        if (n < 0) {
            kill(getppid(), SIGTERM);
            FatalError("Error when reading from pipe", PROC_NAME);
        }
        if (sem_op_handler(sem1_id, 0, -1) < 0) {
            kill(getppid(), SIGTERM);
            FatalError("Error when changing sem value", PROC_NAME);
        }
        memcpy(shm_buffer, buffer, n + 1);
        if (sem_op_handler(sem1_id, 1, 1) < 0) {
            kill(getppid(), SIGTERM);
            FatalError("Error when changing sem value", PROC_NAME);
        }
    }

    return EXIT_SUCCESS;
}

char* attach_shm(int shm_id) {
    shared_mem = shmat(shm_id, NULL, 0);
    if (shared_mem == MAP_FAILED) {
        kill(getppid(), SIGTERM);
        FatalError("Could not attach shared mem", PROC_NAME);
    }
    return (char*)shared_mem;
}

void handler_t(int sig_num) {
    if (shared_mem != NULL) {
        if (shmdt(shared_mem) < 0) {
            kill(getppid(), SIGTERM);
            write(1, "Could not detach shm\n", 22);
        }
    }
    _exit(sig_num);
}

void convert_args_t(const char** argv, int* r2_pd, int* shm1_id, int* sem1_id) {
    if (convert_arg(argv[1], shm1_id) < 0)
        goto error;
    if (convert_arg(argv[2], sem1_id) < 0)
        goto error;
    if (convert_arg(argv[3], r2_pd) < 0)
        goto error;
    return;
error:
    kill(getppid(), SIGTERM);
    FatalError("Could not convert args", PROC_NAME);
}
