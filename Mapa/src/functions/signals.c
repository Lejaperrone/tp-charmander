/*
 * signals.c
 *
 *  Created on: 30/9/2016
 *      Author: utnso
 */

#include <signal.h>
#include <pthread.h>

#include "../commons/structures.h"
#include "config.h"


void sigusr2_handler(int signum){
	log_info(archivoLog,"Recibo senial SIGUSR2, releo metadata.");
	recvSIGUSR2 = 1;
	log_info(archivoLog,"Recibi senial SIGUSR2, continuo.");

}

void verificarSenialesRecibidas(){
	if(recvSIGUSR2==1){
		pthread_mutex_lock(&mutexMapa);
		recvSIGUSR2 = 0;
		leerConfiguracionMetadataMapa(mapa, name, pokedexPath);
		pthread_mutex_unlock(&mutexMapa);
	}
}
