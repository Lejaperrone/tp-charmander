/*
 * sockets.h
 *
 *  Created on: 10/12/2016
 *      Author: utnso
 */

#ifndef FUNCTIONS_SOCKETS_H_
#define FUNCTIONS_SOCKETS_H_

int sendString(int clientSocket, char* parameter, int size);
int recvString(int clientSocket, char** string);

int sendValue(int clientSocket, char* parameter, int size);
int recvValue(int clientSocket, void* buffer);

int recvInt(int clientSocket);
int sendInt(int clientSocket, int number);

int recibirNombreDeLaFuncion(int socketCliente, char* nombreFuncion);
void recibirPath(int socketCliente,char** path);

#endif /* FUNCTIONS_SOCKETS_H_ */
