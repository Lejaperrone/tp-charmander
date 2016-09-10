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
	char* ptrSimbol=malloc(sizeof(char*));
	char* name = argv[1]; //Red
	char* pokedexPath = argv[2]; //../../PokedexConfig

	//Pido memoria para guardar el entrenador y leo la configuracion
		t_entrenador* entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
		leerConfiguracion(entrenador, name, pokedexPath, ptrSimbol);

	//Creo el archivo de log
		archivoLog = crearArchivoLog(entrenador->nombre);

	//Logueo que arranco bien y laconfiguracion del entrenador
		log_info(archivoLog,"Cliente levantado.\n");
		loguearConfiguracion(archivoLog, entrenador);
		char simbolo;
		simbolo=entrenador->simbolo;
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
				send(serverMapa,&simbolo, 2, 0);
				while(enviar){
					fgets(message, PACKAGESIZE, stdin);
					if (!strcmp(message,"exit\n")) enviar = 0;
					if (enviar) send(serverMapa, message, strlen(message) + 1, 0);

				int modoTestCheckpoint1 = 1;
				if(modoTestCheckpoint1){
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
				}else{
					int j;
					int movimiento;
					for(j=0; j<list_size(mapa->objetivos); j++){
						t_objetivo* objetivo = (t_objetivo *)list_get(mapa->objetivos, j);
						printf("Inicio objetivo %s.\n", objetivo->nombre);
						movimiento = 0;

						while(objetivo->logrado==0 && 1){ //aca deberia esperar al siguiente quantum.
							if(objetivo->ubicacion.x==-1 || objetivo->ubicacion.y==-1){ //Obtengo ubicacion de pokenest
								//Aca necesito pedirle al servidor la ubicacion de la pokenest: objetivo->nombre

								printf("Obtuve posicion x:%d, y: %d.\n", objetivo->ubicacion.x, objetivo->ubicacion.y);
							}else if((movimiento = siguienteMovimiento(mapa->miPosicion, objetivo, movimiento))){ //Me muevo
								//Aca tengo que transmitir el movimiento que quiero realizar: arriba(1), derecha(2), abajo(3) o izquierda(4)


								switch(movimiento){
								case 1:
									printf("Me muevo hacia arriba");
									break;
								case 2:
									printf("Me muevo hacia la derecha");
									break;
								case 3:
									printf("Me muevo hacia abajo");
									break;
								case 4:
									printf("Me muevo hacia la izquierda");
									break;
								}

							}else{ //Tengo que solicitar el pokemon
								//Envio la peticion para capturar el pokemon

								printf("Obtuve el pokemon de la pokenest. //El mapa me mato por pedir un pokemon no disponible :'(");
								objetivo->logrado = 1;
							}

						}

					}

					close(serverMapa);

				}
		}
		}

	free(entrenador);
	free(archivoLog);
	return 0;
}
