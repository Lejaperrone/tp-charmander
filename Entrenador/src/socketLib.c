#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<unistd.h>
#include<stdbool.h>

#define BACKLOG 5

void create_serverSocket(int* listenningSocket, char* port){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, port, &hints, &serverInfo);
	*listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(*listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	listen(*listenningSocket, BACKLOG);
}

void accept_connection(int listenningSocket, int* clientSocket){

	struct sockaddr_in addr;

	socklen_t addrlen = sizeof(addr);

	*clientSocket = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
}

int* create_socketClient(int* serverSocket, char* ip, char* port){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, port, &hints, &serverInfo);

	*serverSocket=socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(*serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);
	return serverSocket;
}
