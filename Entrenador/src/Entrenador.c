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

#include "commons/structures.c"
#include "commons/constants.h"
#include "functions/log.h"
#include "functions/config.h"
#include "functions/positions.h"
#include "functions/processLogic.h"

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

int main(int argc, char *argv[]){

	if(argc != 3){
		printf("El entrenador no tiene los parametros correctamente seteados.\n");
		return 1;
	}
	char* name = argv[1]; //Red
	char* pokedexPath = argv[2]; //../../PokedexConfig


	//char* name = "Red";
	//char* pokedexPath = "/home/utnso/projects/tp-2016-2c-Chamba/PokedexConfig";

	/*char* name = "Red";
	char* pokedexPath = "/home/utnso/projects/tp-2016-2c-Chamba/PokedexConfig";*/


	//Pido memoria para guardar el entrenador y leo la configuracion
		entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
		leerConfiguracion(entrenador, name, pokedexPath);

	//Creo el archivo de log
		archivoLog = crearArchivoLog(entrenador->nombre);

	//Logueo que arranco bien y laconfiguracion del entrenador
		log_info(archivoLog,"Cliente levantado.\n");
		loguearConfiguracion(archivoLog, entrenador);

	//Informo mi PID
		printf("PID: %d\n", getpid());

	//Registro signal handler
		signal(SIGUSR1, sigusr1_handler); //signal-number 10
		signal(SIGTERM, sigterm_handler); //signal-number 15

	//Arranco a recorrer los mapas
		int i;
		for(i=0; i<list_size(entrenador->hojaDeViaje); i++){
			//Recupero el mapa al que conectarme
				t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, i);
				log_info(archivoLog,"Proceso mapa %s", mapa->nombre);
				procesarMapa(mapa);

		}

		free(entrenador);
		free(archivoLog);
		return 0;
}

