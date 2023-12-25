#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdint.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 4005
#define PORT 8189

void error(char *msg);

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
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	int connection_count=0;
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	// setup socket
	int sock_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_listen_fd < 0)
	{
		error("Error creating socket..\n");
	}

	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// bind socket and listen for connections
	if (bind(sock_listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		error("Error binding socket..\n");

	if (listen(sock_listen_fd, 512) < 0)
	{
		error("Error listening..\n");
	}
	// printf("epoll echo server listening for connections on port: %d\n", PORT);

	struct epoll_event ev, events[MAX_CONNECTIONS];
	int new_events, sock_conn_fd, epollfd;

	epollfd = epoll_create(MAX_CONNECTIONS);
	if (epollfd < 0)
	{
		error("Error creating epoll..\n");
	}
	ev.events = EPOLLIN;
	ev.data.fd = sock_listen_fd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_listen_fd, &ev) == -1)
	{
		error("Error adding new listeding socket to epoll..\n");
	}

	while (1)
	{
		if(connection_count>=MAX_CONNECTIONS-1){
			// printf("Max connections reached..\n");
			break;
		}
		new_events = epoll_wait(epollfd, events, MAX_CONNECTIONS, -1);

		if (new_events == -1)
		{
			error("Error in epoll_wait..\n");
		}

		for (int i = 0; i < new_events; ++i)
		{
			if (events[i].data.fd == sock_listen_fd)
			{
				sock_conn_fd = accept(sock_listen_fd, (struct sockaddr *)&client_addr, &client_len);
				if (sock_conn_fd == -1)
				{
					error("Error accepting new connection..\n");
				}

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = sock_conn_fd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_conn_fd, &ev) == -1)
				{
					error("Error adding new event to epoll..\n");
				}
			}
			else
			{
				connection_count++;
				do
				{
					int newsockfd = events[i].data.fd;
					int bytes_received = recv(newsockfd, buffer, sizeof(buffer), 0);
					if (bytes_received <= 0)
					{
						// printf("Disconnected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
						epoll_ctl(epollfd, EPOLL_CTL_DEL, newsockfd, NULL);
						shutdown(newsockfd, SHUT_RDWR);
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
						send(newsockfd, buffer, bytes_received, 0);
					}
					
				}while (1);
			}
		}
	}
}

void error(char *msg)
{
	perror(msg);
	// printf("erorr...\n");
	exit(1);
}
