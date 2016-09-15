#ifndef SOCKETLIB_H_
#define SOCKETLIB_H_

void create_socketClient(int* serverSocket, char* ip, char* port);
void create_serverSocket(int* listenningSocket, char* port);
void accept_connection(int listenningSocket, int* clientSocket);

#endif /* SOCKETLIB_H_ */
