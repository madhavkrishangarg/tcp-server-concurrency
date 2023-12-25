#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8189

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
    int sockfd, ret;
    struct sockaddr_in serverAddr;
    int clientSockets[3000] = {0}; // Array to store client sockets

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        // printf("Error in connection.\n");
        exit(1);
    }
    // printf("Server Socket is created.\n");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

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

    while (1)
    {
        int newSocket;
        struct sockaddr_in newAddr;
        socklen_t addr_size = sizeof(newAddr);

        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);

        if (newSocket < 0)
        {
            perror("Accept() error");
            continue;
        }

        // printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        int childpid = fork();

        if (childpid < 0)
        {
            perror("Fork error");
            exit(1);
        }
        if (childpid == 0)
        {
            close(sockfd); // Child process should not listen

            char buffer[1024];

            while (1)
            {
                int valread = recv(newSocket, buffer, sizeof(buffer), 0);

                if (valread <= 0)
                {
                    // printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }

                buffer[valread] = '\0';
                // printf("Client: %s\n", buffer);

                if (strcmp(buffer, ":exit") == 0)
                {
                    // printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }
                else
                {
                    int data = atoi(buffer);
                    uint64_t fact = (uint64_t)data;
                    uint64_t reply = factorial(fact);
                    sprintf(buffer, "%ld", reply);
                    // printf("Server: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                }
            }

            close(newSocket);
            exit(0);
        }
        else
        {
            // Parent process continues to accept new connections
            clientSockets[newSocket] = 1;
            close(newSocket); // Close the socket in the parent process
        }
    }

    // Close the server socket (not reached in this code)
    close(sockfd);

    return 0;
}
