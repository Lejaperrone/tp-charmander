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
#include <pthread.h>

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("El entrenador no tiene los parametros correctamente seteados.\n");
		return 1;
	}
	name = argv[1]; //Red
	pokedexPath = argv[2]; //../../PokedexConfig

	//name = "Ash";
	//pokedexPath = "/home/utnso/projects/tp-2016-2c-Chamba/PokedexConfig";

	//Aloco memoria para el entrenador
	entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));

	//Inicializo mutex
	pthread_mutex_init(&mutexMapaVidasReinicio,NULL);

	//Leo la configuracion
	leerConfiguracion(entrenador, name, pokedexPath);

	//Creo el archivo de log
	archivoLog = crearArchivoLog(entrenador->nombre);
	printf("PID: %d\n", getpid());

	//Logueo que arranco bien y laconfiguracion del entrenador
	loguearConfiguracion(archivoLog, entrenador);

	//Registro signal handler
	signal(SIGUSR1, sigusr1_handler); //signal-number 10
	signal(SIGTERM, sigterm_handler); //signal-number 15

	//Informo mi PID
	log_info(archivoLog, "Entrenador inicia ejecucion PID: %d\n", getpid());

	//Reinicio medallas
	reiniciarJuego();

	t_mapa* mapa = getNextMap();
	while(mapa != NULL){
		pthread_mutex_lock(&mutexMapaVidasReinicio);
		if(entrenador->vidas>0){
			pthread_mutex_unlock(&mutexMapaVidasReinicio);
			reiniciarMapa(mapa);
			if(procesarMapa(mapa)){
				if(mapa->terminado == 0){
					pthread_mutex_lock(&mutexMapaVidasReinicio);
					entrenador->vidas--;
					actualizarMetadata();
					printf("Perdio una vida\n");
					pthread_mutex_unlock(&mutexMapaVidasReinicio);
				}else{
					mapa = getNextMap();
				}
			}else{
				printf("No fue posible conectarse al mapa %s, desea reintentar? (Y/N)", mapa->nombre);
				char resp;
				scanf("%c",&resp);
				if(resp=='N' || resp=='n'){
					mapa = NULL;
				}
			}
		}else{
			pthread_mutex_unlock(&mutexMapaVidasReinicio);
			printf("Te quedaste sin vidas y reintentaste %d veces.\nDesea reiniciar el juego? (Y/N)\n", entrenador->reintentos);

			char string [256];
			fgets (string, 256, stdin);
			char resp = string[0];

			if(resp=='Y' || resp=='y'){
				printf("Reinciaste\n");
				pthread_mutex_lock(&mutexMapaVidasReinicio);
				entrenador->vidas=1;
				entrenador->reintentos++;
				list_iterate(entrenador->hojaDeViaje, (void*)reiniciarMapa);
				mapa = getNextMap();
				if(mapa!=NULL){
					printf("El mapa proximo es: %s\n", mapa->nombre);
				}else{
					printf("No se encontro proximo mapa\n");
				}
				actualizarMetadata();
				reiniciarJuego();
				pthread_mutex_unlock(&mutexMapaVidasReinicio);
			}else{
				mapa=NULL;
				printf("No reinciaste\n");
			}
		}
	}

	if(getNextMap() == NULL){
		//Calculo tiempo total de aventura
		time_t tiempoActual;
		time(&tiempoActual);
		entrenador->tiempoTotalAventura = tiempoActual - entrenador->tiempoTotalAventura;

		//Reporte final
		printf("Felicitaciones, te has convertido en un MAESTRO POKEMON.\n");
		printf("Duracion de la aventura: %d segundos\n", (int)entrenador->tiempoTotalAventura);
		printf("Tiempo transcurrido bloqueado: %d segundos\n", (int)entrenador->tiempoBloqueado);
		printf("Cantidad de  muertes: %d\n", entrenador->muertes);
		printf("Cantidad de veces en deadlock %d\n", entrenador->deadlocks);
	}else{
		printf("Esta vez no lograste convertirte en MAESTRO POKEMON.\nMejor suerte la proxima\n");
	}


	int i,j;
	for(i=0;i<list_size(entrenador->hojaDeViaje);i++){
		t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, i);
		for(j=0;j<list_size(mapa->objetivos);j++){
			t_objetivo* objetivo = (t_objetivo*)list_get(mapa->objetivos, j);
			free(objetivo->nombre);
			free(objetivo);
		}
		list_destroy(mapa->objetivos);
		free(mapa->ip);
		free(mapa->nombre);
		free(mapa->puerto);
		free(mapa);
	}
	list_destroy(entrenador->hojaDeViaje);
	free(entrenador->nombre);
	free(entrenador);
	log_destroy(archivoLog);
	return 0;
}



