#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <tad_items.h>
#include <commons/log.h>
#include <signal.h>
#include <commons/collections/dictionary.h>
#include "commons/constants.h"
#include "functions/log.h"
#include "functions/config.h"
#include "functions/positions.h"
#include "functions/processLogic.h"
#include "functions/signals.h"
#include <time.h>
#include "functions/nextItem.h"
#include "functions/reset.h"
#include "commons/structures.c"



time_t sumaTiemposBloqueos;

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("El entrenador no tiene los parametros correctamente seteados.\n");
		return 1;
	}
	name = argv[1]; //Red
	pokedexPath = argv[2]; //../../PokedexConfig

	//name = "Red";
	//pokedexPath = "/home/utnso/projects/tp-2016-2c-Chamba/PokedexConfig";

	//Aloco memoria para el entrenador
	entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));

	//Leo la configuracion
	leerConfiguracion(entrenador, name, pokedexPath);

	//Creo el archivo de log
	archivoLog = crearArchivoLog(entrenador->nombre);

	//Logueo que arranco bien y laconfiguracion del entrenador
	loguearConfiguracion(archivoLog, entrenador);

	//Registro signal handler
	signal(SIGUSR1, sigusr1_handler); //signal-number 10
	signal(SIGTERM, sigterm_handler); //signal-number 15

	//Informo mi PID
	log_info(archivoLog, "Entrenador inicia ejecucion PID: %d\n", getpid());

	t_mapa* mapa = getNextMap();
	int status;
	while(mapa != NULL){
		if(entrenador->vidas>0){
			reiniciarMapa(mapa);
			status = procesarMapa(mapa, sumaTiemposBloqueos);

			if(mapa->terminado == 0){
				entrenador->vidas--;
			}else{
				mapa = getNextMap();
			}

		}else{
			printf("Te quedaste sin vidas, Desea reiniciar el juego? (Y/N)");
			char* resp;
			scanf("%c",resp);
			if(resp[0]=='Y' || resp[0]=='y'){
				entrenador->vidas=1;
				list_iterate(entrenador->hojaDeViaje, (void*)reiniciarMapa);
				mapa = getNextMap();
			}else{
				mapa=NULL;
			}
		}
	}

	if(getNextMap() != NULL){
		time_t tiempoActual;
		time(&tiempoActual);
		entrenador->tiempoTotal=tiempoActual-entrenador->tiempoTotal;
		printf("\n-------------------------------------------------------------------\n");
		printf("\tTE HAS CONVERTIDO EN UN MAESTRO POKEMON!\n");
		printf("El entrenador ha estado bloqueado en total %ld segundos.\n",sumaTiemposBloqueos);
		printf("El tiempo total recorrido del mapa %s fue de: %ld segundos.\n",mapa->nombre,entrenador->tiempoTotal);
		printf("El entrenador murio %d veces durante la hazania.\n",entrenador->muertes);
		printf("-------------------------------------------------------------------\n");
	}

	free(entrenador);
	free(archivoLog);
	return 0;
}



