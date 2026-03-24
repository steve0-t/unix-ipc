#include "../include/proc_p2.h"

// Main
int main(int argc, const char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s: Incorrect number of arguments\n", PROC_NAME);
        exit(EXIT_FAILURE);
    }

    setup_sig_handler(handler_p2);

    // Convert arg to int
    int r1_pd;
    convert_arg(argv[1], &r1_pd);

    int fd = open(FILE_NAME, O_RDONLY);
    if (fd <= 0) {
        fprintf(stderr, "Could not open specified file: %s\n", FILE_NAME);
        return -1;
    }

    // Initialize mask
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        kill(getppid(), SIGTERM);
        FatalError("Cannot block signals", PROC_NAME);
    }

    kill(getppid(), SIGUSR1);
    while (1) {
        if (sigwaitinfo(&mask, NULL) == SIGUSR1) {
            if (write_to_pipe(r1_pd, fd) < 0) {
                kill(getppid(), SIGTERM);
                FatalError("Error while writing to pipe", PROC_NAME);
            }
        } else
            fprintf(stderr, "%s: Received invalid signal\n", PROC_NAME);
    }

    close(fd);
    return EXIT_SUCCESS;
}

void handler_p2(int sig_num) {
    _exit(sig_num);
}
