#include "../hdr/functions.h"

/*
* Declare:
* - server_fd - fd of server socket;
* - clients_served - number of served clients.
*/
int server_fd;
unsigned int clients_served = 0;

/* Signal handler for SIGINT */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shutdown_server(void)
{
    close(server_fd);

	printf("Server shutdown: %d clients served\n", clients_served);
}

int serial_server(void)
{
	puts("Serial server");

	/*
	* Declare:
    * - sa - sigaction, used to redefine signal handler for SIGINT;
	* - server & client - server's and client's endpoints;
	* - client_fd - fd of client socket;
	* - client_size - size of client's endpoint;
	* - msg - message buffer.
	*/
    struct sigaction sa;
	struct sockaddr_in server, client;
    int client_fd;
	int client_size;
	char msg[MSG_SIZE];
	

	/* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		perror("inet_pton");
		exit(EXIT_FAILURE);
	}
	server.sin_port = SERVER_TCP_PORT;

	/* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Allow reuse of local address */
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	/* Bind server's endpoint to socket */
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	/*
	* TCP protocol only function, mark 'server_fd' socket as passive
	* socket.
	*/
	if (listen(server_fd, SERVER_LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Set atexit function */
	atexit(shutdown_server);

	while(1)
	{
		/*
		* TCP protocol only function, await connection and get client's
		* endpoint and fd.
		*/
		client_size = sizeof(client);
		if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
								&client_size)) == -1)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		/* Send message to client */
		strncpy(msg, SERVER_MSG, MSG_SIZE);
		if (send(client_fd, msg, MSG_SIZE, 0) == -1)
		{
			perror("send");
			continue;
		}
		else
		{
			/* Wait for message from client */
			if (recv(client_fd, msg, MSG_SIZE, 0) == -1)
			{
				perror("recv");
				continue;
			}
			else
			{
				clients_served++;
			}
		}
		close(client_fd);
	}

	return EXIT_SUCCESS;
}