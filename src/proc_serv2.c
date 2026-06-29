#include "../include/proc_serv2.h"
#include <stdio.h>

static int udp_sock_fd = -1; // Socket

// Main
int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        kill(getppid(), SIGTERM);
        FatalError("Incorrect number of arguments", PROC_NAME);
    }

    setup_sig_handler(handler_serv2);

    int                fd       = 0;
    int                udp_port = 0;
    char               buffer[BUFFER_SIZE + 1];
    ssize_t            rx_message_len = 0;
    socklen_t          peer_addr_len  = 0;
    struct sockaddr_in peer_addr;

    convert_args_serv2(argv, &udp_port);
    memset(buffer, 0, sizeof(buffer));

    if ((udp_sock_fd = setup_udp_serv(udp_port)) < 0)
    {
        kill(getppid(), SIGTERM);
        FatalError("Could not get udp socket\n", PROC_NAME);
    }

    fd = open(OUT_FILE, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            if (unlink(OUT_FILE) < 0) goto error;
            if ((fd = open(OUT_FILE, O_CREAT | O_EXCL | O_WRONLY, 0644)) < 0)
                goto error;
        }
        else
            goto error;
    }

    kill(getppid(), SIGUSR1);

    while ((rx_message_len = recvfrom(udp_sock_fd, buffer, BUFFER_SIZE, 0,
                                      (SA*)&peer_addr, &peer_addr_len))
           >= 0)
    {
#if DEBUG
        fprintf(stdout, "%s: received %s\n", PROC_NAME, buffer);
        fflush(stdout);
#endif
        if (write(fd, buffer, rx_message_len - 1) != rx_message_len - 1)
        {
            kill(getppid(), SIGTERM);
            FatalError("Could not write to outfile", PROC_NAME);
        }
        if (write(fd, "\n", 1) != 1)
        {
            kill(getppid(), SIGTERM);
            FatalError("Could not write to outfile", PROC_NAME);
        }
        rx_message_len = 0;
        memset(buffer, 0, sizeof(buffer));
    }

    kill(getppid(), SIGUSR2);
    if (udp_sock_fd != -1) close(udp_sock_fd);

    return EXIT_SUCCESS;

error:
    kill(getppid(), SIGTERM);
    if (udp_sock_fd != -1) close(udp_sock_fd);
    FatalError("Could not open outfile", PROC_NAME);
}

void handler_serv2(int sig_num)
{
    if (udp_sock_fd > 0) close(udp_sock_fd);
    _exit(sig_num);
}

void convert_args_serv2(const char** argv, int* udp_port)
{
    if ((*udp_port = convert_port(argv[2])) < 0) goto error;
    return;
error:
    kill(getppid(), SIGTERM);
    FatalError("Could not convert args", PROC_NAME);
}

int setup_udp_serv(const int port_no)
{
    int                sock_fd = 0;
    struct sockaddr_in serv_addr;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port_no);
    if (inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr) <= 0) return -1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int))
        < 0)
        return -1;
    if (bind(sock_fd, (SA*)&serv_addr, sizeof(serv_addr)) < 0) return -1;
    return sock_fd;
}
