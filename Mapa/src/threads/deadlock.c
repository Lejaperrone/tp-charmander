/*
 * deadlock.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#include "../commons/structures.h"

void* deadlock(void* arg){
	log_trace(archivoLog, "Deadldock - Arranca");
	return arg;
}
