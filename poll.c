#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define PORT 8189
#define MAX_FDS 4005 // max number of simultaneous connections

uint64_t factorial(uint64_t param)
{
    if (param == 1 || param == 0)
        return 1;
    if (param > 20)
    {
        return factorial(20);
    }
    return param * factorial(param - 1);
}

int main()
{
    int connection_counter = 0;
    int len, rc, on = 1;
    int listen_sd = -1, new_sd = -1;
    int desc_ready, end_server = 0, closed_connection = 1;
    int close_conn;
    char buffer[1024];
    struct sockaddr_in addr;
    int timeout;
    struct pollfd fds[MAX_FDS];
    int nfds = 1, current_size = 0, i, j;
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failed");
        exit(-1);
    }
    rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR,
                    (char *)&on, sizeof(on));
    if (rc < 0)
    {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(-1);
    }

    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        perror("ioctl() failed");
        close(listen_sd);
        exit(-1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }
    rc = listen(listen_sd, 512);
    if (rc < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }
    memset(fds, 0, sizeof(fds));
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;
    timeout = (3 * 60 * 1000);

    do
    {
        // printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, timeout);
        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }
        if (rc == 0)
        {
            // printf("  poll() timed out.  End program.\n");
            break;
        }
        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
            if (fds[i].revents == 0)
                continue;
            if (fds[i].revents != POLLIN)
            {
                // printf("  Error! revents = %d\n", fds[i].revents);
                end_server = 1;
                break;
            }

            if (fds[i].fd == listen_sd)
            {
                do
                {
                    new_sd = accept(listen_sd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("accept() failed");
                            end_server = 1;
                        }
                        break;
                    }
                    // printf("New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } while (new_sd != -1);
            }
            else
            {
                close_conn = 0;
                connection_counter++;
                do
                {
                    memset(buffer, 0, sizeof(buffer));
                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    // printf("Client: %s\n", buffer);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  recv() failed");
                            close_conn = 1;
                        }
                        break;
                    }
                    if (rc == 0)
                    {
                        // printf("Connection closed\n");
                        close_conn = 1;
                        break;
                    }
                    len = rc;
                    int data = atoi(buffer);
                    uint64_t fact = (uint64_t)data;
                    uint64_t reply = factorial(fact);
                    sprintf(buffer, "%ld", reply);
                    // printf("Server: %s\n", buffer);
                    rc = send(fds[i].fd, buffer, strlen(buffer), 0);
                    if (rc < 0)
                    {
                        perror("  send() failed");
                        close_conn = 1;
                        break;
                    }
                } while (1);

                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    closed_connection = 1;
                }
            }
        }
        // remove the closed connection
        if (closed_connection)
        {
            closed_connection = 0;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for (j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j + 1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }
        if (connection_counter >= MAX_FDS - 1)
        {
            // printf("Max number of connections reached\n");
            break;
        }
    } while (end_server == 0);

    for (i = 0; i < nfds; i++)
    {
        if (fds[i].fd >= 0)
            close(fds[i].fd);
    }
}
