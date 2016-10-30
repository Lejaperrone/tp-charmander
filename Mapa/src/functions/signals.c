/*
 * signals.c
 *
 *  Created on: 30/9/2016
 *      Author: utnso
 */

#include <signal.h>

#include "../commons/structures.h"
#include "config.h"


void sigusr2_handler(int signum){
	log_info(archivoLog,"Recibo senial SIGUSR2, releo metadata.");
	recvSIGUSR2 = 1;

}

void verificarSenialesRecibidas(){
	if(recvSIGUSR2==1){
		recvSIGUSR2 = 0;
		leerConfiguracionMetadataMapa(mapa, name, pokedexPath);
	}
}
