/*
 * signals.c
 *
 *  Created on: 20/11/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "../commons/structures.c"
#include "../commons/constants.h"
#include <commons/log.h>

void sigusr1_handler(int signum){
	log_info(archivoLog,"Recibo senial SIGUSR1, agrego una vida.");
	entrenador->vidas++;
	log_info(archivoLog,"Tengo %d vidas.", entrenador->vidas);
}

void sigterm_handler(int signum){
	log_info(archivoLog,"Recibo senial SIGTERM, pierdo una vida.");
	entrenador->vidas--;
	log_info(archivoLog,"Tengo %d vidas.", entrenador->vidas);
}
