/*
 * auxiliar_functions.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#ifndef FUNCTIONS_AUXILIAR_FUNCTIONS_H_
#define FUNCTIONS_AUXILIAR_FUNCTIONS_H_

	int sendString(int clientSocket, char* parameter, int size);
	int recvValue(int clientSocket, void* buffer);
	int recvString(int clientSocket, char** string);
	int recvInt(int clientSocket);
	int sendInt(int clientSocket, int number);

	int recibirNombreDeLaFuncion(int socketCliente, char* nombreFuncion);
	void recibirPath(int socketCliente,char** path);

#endif /* FUNCTIONS_AUXILIAR_FUNCTIONS_H_ */
