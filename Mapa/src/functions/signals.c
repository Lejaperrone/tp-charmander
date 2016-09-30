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
	leerConfiguracionMetadataMapa(mapa, name, pokedexPath);
}
