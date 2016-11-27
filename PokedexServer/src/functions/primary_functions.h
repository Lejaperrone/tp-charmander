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

	int proce_getattr(int clientSocket);
	int proce_readdir(int clientSocket);
	int proce_readfile(int clientSocket);
	int proce_create(int clientSocket);
	int proce_truncate(int clientSocket);
	int proce_mkdir(int clientSocket);
	int proce_rename(int clientSocket);
	int proce_write(int clientSocket);
	int proce_statfs(int clientSocket);
	int proce_removeFile(int clientSocket);
	int proce_removeDir(int clientSocket);
	int proce_open(int clientSocket);

#endif /* FUNCTIONS_PRIMARY_FUNCTIONS_H_ */