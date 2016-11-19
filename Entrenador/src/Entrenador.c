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
#include "commons/structures.c"
#include "commons/constants.h"
#include "functions/log.h"
#include "functions/config.h"
#include "functions/positions.h"
#include "functions/processLogic.h"
#include <time.h>


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
time_t sumaTiemposBloqueos;

int main(int argc, char *argv[]){
	printf("Paso -1");
	if(argc != 3){
		printf("El entrenador no tiene los parametros correctamente seteados.\n");
		return 1;
	}
	char* name = argv[1]; //Red
	char* pokedexPath = argv[2]; //../../PokedexConfig

	//char* name = "Red";
	//char* pokedexPath = "/home/utnso/projects/tp-2016-2c-Chamba/PokedexConfig";

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
	int i, j, valorDeRetornoMapa, valorDeRetornoAccionesSegunVidas;
	sumaTiemposBloqueos=0;
	time(&(entrenador->tiempoTotal));
	for(i=0; i<list_size(entrenador->hojaDeViaje); i++){
		//Recupero el mapa al que conectarme
		t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, i);
		log_info(archivoLog,"Proceso mapa %s", mapa->nombre);
		mapa->miPosicion.x = 1;
		mapa->miPosicion.y = 1;
		//Tomo el tiempo en que empiezo el mapa

		//printf("El tiempo al inicio vale: %ld \n",entrenador->tiempoTotal);
		valorDeRetornoMapa = procesarMapa(mapa, sumaTiemposBloqueos);
		log_info(archivoLog,"Procese el mapa %s",mapa->nombre);
		if(valorDeRetornoMapa == 1){
			valorDeRetornoAccionesSegunVidas = accionesSegunLasVidasDisponibles();
			if(valorDeRetornoAccionesSegunVidas == 0){
				list_map(mapa->objetivos, (void*)resetearObjetivos);
				log_info(archivoLog, "Resetie todos los objetivos del mapa. Ejecuto de vuelta el mismo mapa.");
				i--;
			}else{
				log_info(archivoLog, "Ejecuto el primer mapa de la hoja de viajes.");
				for(j = 0; j<list_size(entrenador->hojaDeViaje); j++){
					t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, j);
					list_map(mapa->objetivos, (void*)resetearObjetivos);
				}
				i = -1;
			}
		}else if (valorDeRetornoMapa==2){
			i=list_size(entrenador->hojaDeViaje);
		}
		if (i>=0){
		time_t tiempoActual;
		//log_info(archivoLog,"Tomo tiempo actual");
			time(&tiempoActual);
			//log_info(archivoLog,"El tiempo actual es %ld",(long)tiempoActual);
			entrenador->tiempoTotal=tiempoActual-entrenador->tiempoTotal;
			//log_info(archivoLog,"El tiempo que guarde en entrenador es %ld",(long)entrenador->tiempoTotal);
		}
		if (i==(list_size(entrenador->hojaDeViaje)-1)){
			printf("\n-------------------------------------------------------------------\n");
			printf("\tTE HAS CONVERTIDO EN UN MAESTRO POKEMON!\n");
		/*	void sumarTiemposDeBloqueos(char* key,time_t tiempo ){
				//printf("El tiempo de bloqueo en la Pokenest %s es %ld\n", key,tiempo);
				//printf("EL entrenador recorrido %d pokenests\n",dictionary_size(entrenador->tiempoTotalPokenests));
				sumaTiemposBloqueos+=tiempo;
			}
*/
			/*void calcularTiempototalDeBloqueoPorPokenest(){
				 dictionary_iterator(entrenador->tiempoTotalPokenests,(void*)sumarTiemposDeBloqueos);


			}*/

			//printf("El tiempo total vale: %ld",entrenador->tiempoTotal);
			//entrenador->tiempoTotal=time(&ultimoMomentoEnElMapa)-entrenador->tiempoTotal;
			//printf("El ultimo momento en el mapa vale: %ld\n", ultimoMomentoEnElMapa);
			//calcularTiempototalDeBloqueoPorPokenest();
			printf("El entrenador ha estado bloqueado en total %ld segundos.\n",sumaTiemposBloqueos);
			printf("El tiempo total recorrido del mapa %s fue de: %ld segundos.\n",mapa->nombre,entrenador->tiempoTotal);
				printf("El entrenador murio %d veces durante la hazania.\n",entrenador->muertes);
				printf("-------------------------------------------------------------------\n");
			}

	}


	//dictionary_destroy(entrenador->tiempoTotalPokenests);
	free(entrenador);
	free(archivoLog);
	return 0;
}



