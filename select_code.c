#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8189
#define MAX_CONNECTIONS 4005


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

int createSocket();

int main()
{
    fd_set read_fd_set;
    struct sockaddr_in new_addr;
    int server_fd, new_fd, ret_val, i;
    socklen_t addrlen;
    char buf[1024];
    int desc = 4;
    int array[MAX_CONNECTIONS];
    int count = 0;

    server_fd = createSocket();
    if (server_fd == -1)
    {
        perror("FD Negative\n");
        exit(1);
    }

    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        array[i] = -1;
    }
    array[0] = server_fd;

    while (1)
    {
        FD_ZERO(&read_fd_set);

        for (i = 0; i < MAX_CONNECTIONS; i++)
        {
            if (array[i] >= 0)
            {
                FD_SET(array[i], &read_fd_set);
            }
        }

        // printf("\nListening ...... \n");
        ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        if (ret_val >= 0)
        {

            if (FD_ISSET(server_fd, &read_fd_set))
            {

                new_fd = accept(server_fd, (struct sockaddr *)&new_addr, &addrlen);
                if (new_fd >= 0)
                {
                    //  printf(" New connection assigned file descriptor: %d\n", new_fd);
                    count++;
                    for (i = 0; i < MAX_CONNECTIONS; i++)
                    {
                        if (array[i] < 0)
                        {
                            array[i] = new_fd;
                            break;
                        }
                    }
                }
                else
                {

                    perror("Accept failed");
                }
                ret_val--;
                if (!ret_val)
                    continue;
            }

            for (i = 1; i < MAX_CONNECTIONS; i++)
            {
                if ((array[i] > 0) &&
                    (FD_ISSET(array[i], &read_fd_set)))
                {

                    ret_val = recv(array[i], buf, sizeof(buf), 0);
                    if (ret_val == 0)
                    {
                        //  printf("Detected connection termination request for fd :%d\n", array[i]);
                        close(array[i]);
                        array[i] = -1;
                    }
                    if (ret_val > 0)
                    {

                        // printf("Client sent %s\n", buf);
                        int data = atoi(buf);
                        uint64_t fact = (uint64_t)data;
                        uint64_t ans = factorial(fact);
                        char reply[1024];
                        sprintf(reply, "%ld", ans);
                        // printf("Server sent %s\n\n", reply);
                        send(array[i], reply, sizeof(reply), 0);
                    }
                    if (ret_val == -1)
                    {
                        perror("Recv() failed");
                        exit(1);
                    }
                }
                ret_val--;
                if (!ret_val)
                    continue;
            }
        }
    }

    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (array[i] > 0)
        {
            close(array[i]);
        }
    }

    return 0;
}

int createSocket()
{
    struct sockaddr_in saddr;
    int fd, ret_val;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {

        perror("Socket error");
        return -1;
    }

    // printf("Created socket with file Descriptor : %d\n", fd);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    ret_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret_val != 0)
    {
        fprintf(stderr, "bind failed [%s]\n", strerror(errno));
        perror("Bind error");
        exit(1);
    }

    ret_val = listen(fd, 512);
    if (ret_val != 0)
    {
        perror("Listen failed");
        exit(1);
    }
    return fd;
}
