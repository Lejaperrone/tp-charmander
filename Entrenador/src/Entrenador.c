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

#include "socketLib.h"
#include "commons/structures.c"
#include "commons/constants.h"
#include "functions/log.h"
#include "functions/config.h"
#include "functions/positions.h"



t_log* archivoLog;

int main(int argc, char *argv[]){

	if(argc != 3){
		printf("El entrenador no tiene los parametros correctamente seteados.\n");
		return 1;
	}

	char* name = argv[1]; //Red
	char* pokedexPath = argv[2]; //../../PokedexConfig

	//Pido memoria para guardar el entrenador y leo la configuracion
		t_entrenador* entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
		leerConfiguracion(entrenador, name, pokedexPath);

	//Creo el archivo de log
		archivoLog = crearArchivoLog(entrenador->nombre);

	//Logueo que arranco bien y laconfiguracion del entrenador
		log_info(archivoLog,"Cliente levantado.\n");
		loguearConfiguracion(archivoLog, entrenador);

	//Arranco a recorrer los mapas
		int serverMapa;
		int i;
		for(i=0; i<list_size(entrenador->hojaDeViaje); i++){
			//Recupero el mapa al que conectarme
				t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, i);

			//Me conecto al mapa
				printf("Conectandose al mapa %s...\n", mapa->nombre);
				create_socketClient(&serverMapa, mapa->ip, mapa->puerto);
				printf("Conectado al mapa %s.\n", mapa->nombre);

			//Espero que quiera desconectarse para pasar al siguiente mapa
				printf("Deberiamos empezar a procesar los objetivos pero no llegamos a eso aun. Ingresa 'exit' para conectarte al proximo mapa\n"); //Borrar cuando los procesemos
				int enviar = 1;
				char message[PACKAGESIZE];

				while(enviar){
					fgets(message, PACKAGESIZE, stdin);
					if (!strcmp(message,"exit\n")) enviar = 0;
					if (enviar) send(serverMapa, message, strlen(message) + 1, 0);
				}

				close(serverMapa);
		}


	free(entrenador);
	free(archivoLog);
	return 0;
}
