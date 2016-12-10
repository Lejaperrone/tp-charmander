/*
 * onDemand.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#ifndef THREADS_ONDEMAND_H_
#define THREADS_ONDEMAND_H_

	void proce_getattr(int clientSocket, char* path);
	void proce_readdir(int clientSocket, char* path);
	void proce_open(int clientSocket, char* path);
	void proce_readfile(int clientSocket, char* path);
	void proce_create(int clientSocket, char* path);
	void proce_truncate(int clientSocket, char* path);
	void proce_mkdir(int clientSocket, char* path);
	void proce_rename(int clientSocket, char* path);
	void proce_removeFile(int clientSocket, char* path);
	void proce_removeDir(int clientSocket, char* path);
	void proce_write(int clientSocket, char* path);
	void proce_statfs(int clientSocket, char* path);
	void* procesarPeticiones(int* socket);


#endif /* THREADS_ONDEMAND_H_ */
