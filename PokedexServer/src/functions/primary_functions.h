/*
 * primary_functions.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "../socketLib.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../osada/osada.h"
#include <pthread.h>
#include <stdint.h>
#include <pthread.h>
#include "../commons/structures.h"
#include "../commons/definitions.h"

#ifndef FUNCTIONS_PRIMARY_FUNCTIONS_H_
#define FUNCTIONS_PRIMARY_FUNCTIONS_H_

	void proce_getattr(int clientSocket, char* path);
	void proce_readdir(int clientSocket, char* path);
	void proce_readfile(int clientSocket, char* path, char** bufParaElRead);
	void proce_create(int clientSocket, char* path);
	void proce_truncate(int clientSocket, char* path);
	void proce_mkdir(int clientSocket, char* path);
	void proce_rename(int clientSocket, char* path);
	void proce_write(int clientSocket, char* path);
	void proce_statfs(int clientSocket, char* path);
	void proce_removeFile(int clientSocket, char* path);
	void proce_removeDir(int clientSocket, char* path);
	void proce_open(int clientSocket, char* path);

#endif /* FUNCTIONS_PRIMARY_FUNCTIONS_H_ */
