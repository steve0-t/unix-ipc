#include "../include/zadanie.h"
#include "../include/helpers.h"

// Static array containing program name and pathname
// clang-format off
static const ProgramInfo programs[PROGRAM_COUNT] = {
    [P1] = { "P1", "./proc_p1" },
    [P2] = { "P2", "./proc_p2" },
    [PR] = { "PR", "./proc_pr" },
    [T] = { "T", "./proc_t" },
    [S] = { "S", "./proc_s" },
    [D] = { "D", "./proc_d" },
    [SERV1] = { "SERV1", "./proc_serv1" },
    [SERV2] = { "SERV2", "./proc_serv2" },
};

// clang-format on

// IPC resources
static int   r1_pd[2], r2_pd[2];
static void *mem_addr1 = NULL, *mem_addr2 = NULL;
static int   shm1_id = -1, shm2_id = -1;
static int   sem1_id = -1, sem2_id = -1;

// Struct for timed wait
static struct timespec to;

// Mask for signals
static sigset_t mask;

static int      pid_buffer[PROGRAM_COUNT];

// Main
int main(int argc, char* argv[]) {
    if (argc != 3)
        FatalError("Incorrect count of arguments! Usage: <zadanie> <port num 1> "
                   "<port num 2>",
                   PROC_NAME);
    // Setup sig handler

    // Init mask
    initialize_mask();

    // Init sig handler
    setup_sig_handler(handler_main);

    // Ipc_resources ipc_resources;
    // memset(&ipc_resources, 0, sizeof(Ipc_resources));

    // Set time to wait for signal from child
    to.tv_sec  = TIME_TO_WAIT;
    to.tv_nsec = 0;

    // Create pipes
    if (pipe(r1_pd) < 0 || pipe(r2_pd) < 0)
        FatalError("Error with creating pipe", PROC_NAME);

    // Launch proc P1 and P2
    const int p_args[1] = {r1_pd[WRITE]};
    pid_buffer[P1]      = launch_program(P1, 1, p_args);
    pid_buffer[P2]      = launch_program(P2, 1, p_args);

    // Launch proc PR
    const int pr_args[] = {pid_buffer[P1], pid_buffer[P2], r1_pd[READ], r2_pd[WRITE]};
    pid_buffer[PR]      = launch_program(PR, 4, pr_args);

    create_shm_and_sem();

    // Launch proc S
    const int s_args[] = {shm1_id, sem1_id, shm2_id, sem2_id};
    pid_buffer[S]      = launch_program(S, 4, s_args);

    // Launch proc T
    const int t_args[] = {shm1_id, sem1_id, r2_pd[READ]};
    pid_buffer[T]      = launch_program(T, 3, t_args);

    // Launch proc SERV1 and SERV2
    pid_buffer[SERV1] = launch_server(SERV1, argv[1], argv[2]);
    pid_buffer[SERV2] = launch_server(SERV2, argv[1], argv[2]);

    // Launch proc D
    int tcp_port_no = convert_port(argv[1]);
    if (tcp_port_no < 0)
        FatalError("Could not convert port", PROC_NAME);
    int d_args[]  = {shm2_id, sem2_id, tcp_port_no};
    pid_buffer[D] = launch_program(D, 3, d_args);

    sigtimedwait(&mask, NULL, &to);

    cleanup_ipc_resources();
    return EXIT_SUCCESS;
}

// Helper function to init mask
void initialize_mask() {
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        FatalError("Cannot block signals", PROC_NAME);
}

// Helper function to execute program specified by which
int execute_program(int program, char* const args[], char* const env[]) {
    int pid = fork();
    if (pid < 0) {
        cleanup_ipc_resources();
        FatalError("Error with fork", PROC_NAME);
    } else if (pid == 0)
        execve(programs[program].pathname, args, env);

    if (sigtimedwait(&mask, NULL, &to) != SIGUSR1) {
        cleanup_ipc_resources();
        FatalError("Received no signal or received wrong one", PROC_NAME);
    }
#if DEBUG
    fprintf(stdout, "launched %s: %d\n", programs[program].name, pid);
#endif
    return pid;
}

// Helper function to launch program specified by param which
// Constructs params for execute program
int launch_program(const int which, const int argc, const int args[]) {
    char* argv[argc + 2];
    if (asprintf(&argv[0], "%s", programs[which].pathname) < 0) {
        cleanup_ipc_resources();
        FatalError("Asprintf error", PROC_NAME);
    }
    for (int i = 1; i <= argc; i++) {
        if (asprintf(&argv[i], "%d", args[i - 1]) < 0) {
            cleanup_ipc_resources();
            FatalError("Asprintf error", PROC_NAME);
        }
    }
    argv[argc + 1] = NULL;

#if DEBUG
    fprintf(stdout, "launching: %s\n", programs[which].name);
    printf("args:");
    for (int i = 0; i < argc + 1; i++)
        printf(" %s", argv[i]);
    printf("\n");
#endif

    int pid = execute_program(which, argv, NULL);
    for (int i = 0; i < argc + 2; i++)
        free(argv[i]);
    return pid;
}

// Helper function to launch SERV1 and SERV2
int launch_server(const int which, const char* port1, const char* port2) {
    size_t argc = 3;
    char*  argv[argc + 1];
    if (asprintf(&argv[0], "%s", programs[which].pathname) < 0) {
        cleanup_ipc_resources();
        FatalError("Asprintf error", PROC_NAME);
    }
    if (asprintf(&argv[1], "%s", port1) < 0) {
        cleanup_ipc_resources();
        FatalError("Asprintf error", PROC_NAME);
    }
    if (asprintf(&argv[2], "%s", port2) < 0) {
        cleanup_ipc_resources();
        FatalError("Asprintf error", PROC_NAME);
    }
#if DEBUG
    fprintf(stdout, "%s: server %s with port %s and %s\n", PROC_NAME, programs[which].name, port1, port2);
#endif
    argv[argc]     = NULL;
    int server_pid = execute_program(which, argv, NULL);
    for (size_t i = 0; i < argc; i++)
        free(argv[i]);
#if DEBUG
    fprintf(stdout, "executed: %s\n", programs[which].name);
#endif
    return server_pid;
}

// Grouped helper init functions
void create_shm_and_sem() {
    shm1_id = create_shared_mem(&mem_addr1, SHM_KEY_1);
    shm2_id = create_shared_mem(&mem_addr2, SHM_KEY_2);
    sem1_id = create_semaphore(SEM_KEY_1);
    sem2_id = create_semaphore(SEM_KEY_2);
    initialize_sems(sem1_id);
    initialize_sems(sem2_id);
}

// Helper function to create shared mem
int create_shared_mem(void** mem_addr, key_t key) {
    int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0)
        FatalError("Could not get shared mem", PROC_NAME);
    *mem_addr = shmat(shm_id, NULL, 0);
    if (*mem_addr == MAP_FAILED)
        FatalError("Could not attach shared mem", PROC_NAME);
#if DEBUG
    printf("shm id: %d\n", shm_id);
#endif
    return shm_id;
}

// Helper function to create semaphores
int create_semaphore(key_t key) {
    int sem_id = semget(key, N_SEMS, IPC_CREAT | 0644);
    if (sem_id < 0)
        FatalError("Could not get semaphore", PROC_NAME);
#if DEBUG
    printf("sem id: %d\n", sem_id);
#endif
    return sem_id;
}

// Helper function to initialize semaphores
void initialize_sems(int sem_id) {
    union semun    sem_union;
    unsigned short init_vals[N_SEMS] = {1, 0};
    sem_union.array                  = init_vals;
    if (semctl(sem_id, 0, SETALL, sem_union) < 0)
        FatalError("Could not initialize semaphore", PROC_NAME);
}

// Grouped helper cleanup functions
void cleanup_ipc_resources() {
    for (int i = 0; i < PROGRAM_COUNT; i++)
        kill(pid_buffer[i], SIGTERM);
    close_pipe(r1_pd);
    close_pipe(r2_pd);
    cleanup_shared_mem();
    cleanup_sempahores();
}

// Helper function to cleanup shared mem
void cleanup_shared_mem() {
    if (mem_addr1 != NULL) {
        if (shmdt(mem_addr1) < 0)
            write(1, "Could not detach shm1\n", 23);
    }
    if (mem_addr2 != NULL) {
        if (shmdt(mem_addr2) < 0)
            write(1, "Could not detach shm2\n", 23);
    }
    if (shm1_id >= 0) {
        if (shmctl(shm1_id, IPC_RMID, NULL) < 0)
            write(1, "Could not remove shm1\n", 23);
    }
    if (shm2_id >= 0) {
        if (shmctl(shm2_id, IPC_RMID, NULL) < 0)
            write(1, "Could not remove shm2\n", 23);
    }
}

// Helper function to cleanup semaphores
void cleanup_sempahores() {
    if (sem1_id >= 0) {
        if (semctl(sem1_id, 0, IPC_RMID) < 0)
            write(1, "Could not delete semaphore\n", 28);
    }
    if (sem2_id >= 0) {
        if (semctl(sem2_id, 0, IPC_RMID) < 0)
            write(1, "Could not delete semaphore\n", 28);
    }
}

int close_pipe(int pd[2]) {
    if (close(pd[READ]) < 0)
        return -1;
    if (close(pd[WRITE]) < 0)
        return -1;
    return 0;
}

void handler_main(int sig_num) {
    write(1, "Error occurred aborting\n", 25);
    cleanup_ipc_resources();
    _exit(sig_num);
}

void FatalError(const char* msg, const char* proc_name) {
    fprintf(stderr, "%s: %d:%s:%s\n", proc_name, errno, strerror(errno), msg);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

void setup_sig_handler(void (*f)(int)) {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = f;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGQUIT, &new_action, NULL);
}

inline int convert_port(const char* arg) {
    if (arg == NULL)
        return -1;
    char* endptr = NULL;
    long  temp   = strtol(arg, &endptr, 10);
    if (temp <= LONG_MIN || temp >= LONG_MAX || errno == ERANGE)
        return -1;
    return (int)temp;
}
