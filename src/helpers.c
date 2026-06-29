#include "../include/helpers.h"

void FatalError(const char* msg, const char* proc_name)
{
    fprintf(stderr, "%s: %d:%s:%s\n", proc_name, errno, strerror(errno), msg);
    exit(EXIT_FAILURE);
}

ssize_t write_to_pipe(const int pipe_fd, const int fd)
{
    ssize_t bytes_read              = 0;
    char    buffer[BUFFER_SIZE + 1] = { 0 };
    if ((bytes_read = read_line(fd, buffer, BUFFER_SIZE + 1)) > 0)
    {
        if (write(pipe_fd, buffer, bytes_read) < 0)
        {
            fprintf(stderr, "Could not read from file\n");
            return -1;
        }
        if (write(pipe_fd, "\n", 1) < 0)
        {
            fprintf(stderr, "Could not read from file\n");
            return -1;
        }
    }
    return bytes_read;
}

ssize_t read_line(int fd, char* buf, int nbytes)
{
    ssize_t bytes_read = 0, temp = 0;
    char    c;
    while (bytes_read < nbytes)
    {
        temp = read(fd, &c, 1);
        if (temp == 0) return bytes_read;
        if (temp < 0) return -1;
        if (c == '\n')
        {
            buf[bytes_read] = 0;
            return bytes_read;
        }
        buf[bytes_read++] = c;
    }
    buf[bytes_read] = 0;
    return bytes_read;
}

int convert_arg(const char* argv, int* res)
{
    if (sscanf(argv, "%d", &(*res)) == EOF)
    {
        fprintf(stderr, "Sscanf error");
        return -1;
    }
    return 0;
}

int sem_op_handler(int sem_id, unsigned short sem_num, short sem_op)
{
    struct sembuf buf;
    buf.sem_num = sem_num;
    buf.sem_op  = sem_op;
    buf.sem_flg = SEM_UNDO;
    if (semop(sem_id, &buf, 1) < 0) return -1;
    return 0;
}

int convert_port(const char* arg)
{
    if (arg == NULL) return -1;
    char* endptr = NULL;
    long  temp   = strtol(arg, &endptr, 10);
    if (temp <= LONG_MIN || temp >= LONG_MAX || errno == ERANGE) return -1;
    return (int)temp;
}

int connect_to_server(const int port_no)
{
    int                sock_fd = 0;
    struct sockaddr_in serv_addr;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port_no);
    if (inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr) <= 0) return -1;
    if (connect(sock_fd, (SA*)&serv_addr, sizeof(serv_addr)) < 0) return -1;
    return sock_fd;
}

void setup_sig_handler(void (*f)(int))
{
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = f;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGQUIT, &new_action, NULL);
}
