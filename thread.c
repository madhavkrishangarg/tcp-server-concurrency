#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8189
#define MAX_CONNECTIONS 3005

struct thread_args
{
    int newSocket;
    char port[80];
    char ip[80];
};

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

void *clientHandler(void *arg)
{
    struct thread_args *helper = arg;

    char buffer[1024];

    while (1)
    {
        recv(helper->newSocket, buffer, 1024, 0);

        if (strlen(buffer) == 0)
        {
            // printf("Disconnected from %s:%s\n", helper->ip, helper->port);
            break;
        }

        if (strcmp(buffer, ":exit") == 0)
        {
            // printf("Disconnected from %s:%s\n", helper->ip, helper->port);
            break;
        }
        else
        {
            // printf("Client: %s\n", buffer);
            int data = atoi(buffer);
            uint64_t fact = (uint64_t)data;
            uint64_t reply = factorial(fact);
            sprintf(buffer, "%ld", reply);
            // printf("Server: %s\n\n", buffer);
            send(helper->newSocket, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof(buffer));
        }
    }
    close(helper->newSocket);
    //free(helper);
    pthread_exit(NULL);
}

int main()
{
    int sockfd, ret;
    struct sockaddr_in serverAddr;

    socklen_t addr_size;
    pthread_t tid[MAX_CONNECTIONS];
    // int newSocket[MAX_CONNECTIONS];
    struct thread_args helper[MAX_CONNECTIONS];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        // printf("Error in connection.\n");
        exit(1);
    }
    // printf("Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        // printf("Error in binding.\n");
        exit(1);
    }
    // printf("Bind to port %d\n", PORT);

    if (listen(sockfd, 512) == 0)
    {
        // printf("Listening....\n");
    }
    else
    {
        // printf("Error in binding.\n");
    }

    for(int i=0; i<MAX_CONNECTIONS; i++)
    {
        int newSocket = accept(sockfd, (struct sockaddr *)&serverAddr, &addr_size);

        if (newSocket < 0)
        {
            exit(1);
        }

        // printf("Connection accepted from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

        helper[i].newSocket = newSocket;
        strcpy(helper[i].ip, inet_ntoa(serverAddr.sin_addr));
        sprintf(helper[i].port, "%d", ntohs(serverAddr.sin_port));

        if (pthread_create(&tid[i], NULL, clientHandler, &helper[i]) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
    }
    for(int i=0; i<MAX_CONNECTIONS; i++)
    {
        pthread_join(tid[i], NULL);
    }

    close(sockfd);

    return 0;
}
