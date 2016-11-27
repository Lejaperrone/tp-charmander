/*
 * procesamiento.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../commons/structures.c"
#include "../commons/constants.h"
#include <commons/string.h>
#include "../socketLib.h"
#include "positions.h"
#include "processLogic.h"
#include <time.h>

void copiarPokemonFile(char* path){
	char* pathFrom=string_new();
	string_append(&pathFrom, pokedexPath);
	string_append(&pathFrom, path);

	char** splited = string_split(path, "/");
	char* pathTo=string_new();
	string_append(&pathTo, pokedexPath);
	string_append(&pathTo, "/Entrenadores/");
	string_append(&pathTo, name);
	string_append(&pathTo, "/Dir de Bill/");
	string_append(&pathTo, splited[4]);

	FILE *from, *to;
	char ch;
	if((from = fopen(pathFrom, "rb"))!=NULL) {
		if((to = fopen(pathTo, "wb+"))!=NULL) {
			while(!feof(from)) {
				ch = fgetc(from);
				if(!feof(from)) fputc(ch, to);
			}
		}
		fclose(to);
	}
	fclose(from);
}
void copiarMedalla(t_mapa* mapa){
	char* pathFrom=string_new();
	string_append(&pathFrom, pokedexPath);
	string_append(&pathFrom, "/Mapas/");
	string_append(&pathFrom, mapa->nombre);
	string_append(&pathFrom, "/medalla-");
	string_append(&pathFrom, mapa->nombre);
	string_append(&pathFrom, ".jpg");
	printf("Path medalla from: %s\n", pathFrom);


	char* pathTo=string_new();
	string_append(&pathTo, pokedexPath);
	string_append(&pathTo, "/Entrenadores/");
	string_append(&pathTo, name);
	string_append(&pathTo, "/medallas/medalla-");
	string_append(&pathTo, mapa->nombre);
	string_append(&pathTo, ".jpg");
	printf("Path medalla from: %s\n", pathTo);

	FILE *from, *to;
	char ch;
	if((from = fopen(pathFrom, "rb"))!=NULL) {
		if((to = fopen(pathTo, "wb+"))!=NULL) {
			while(!feof(from)) {
				ch = fgetc(from);
				if(!feof(from)) fputc(ch, to);
			}
		}
		fclose(to);
	}
	fclose(from);
}


int procesarObjetivo(t_mapa* mapa, t_objetivo* objetivo, int* movimiento, int serverMapa){
	char turno;

	while(objetivo->logrado==0 && recv(serverMapa, &turno, 1, 0) && turno=='Q'){ //aca deberia esperar al siguiente quantum.
		log_info(archivoLog, "Obtuve un turno");
		if(objetivo->ubicacion.x==-1 || objetivo->ubicacion.y==-1){ //Obtengo ubicacion de pokenest
			//Creo el mensaje
			char* mensaje = string_new();
			string_append(&mensaje, "U");
			string_append(&mensaje, objetivo->nombre);
			log_info(archivoLog,"el mensaje que voy a enviar es: %s",mensaje);

			//Envio el mensaje
			int resp = send(serverMapa, mensaje, 2, 0);
			if(resp <0){
				log_info(archivoLog,"No pude enviar el mensaje: %s", mensaje);
				exit(EXIT_FAILURE);
			}
			log_info(archivoLog,"Envie el mensaje %s",mensaje);

			//Espero la respuesta
			char pos[5];
			if (recv(serverMapa, pos, 5,  0) == 5){
				objetivo->ubicacion.x = atoi(string_substring(pos, 0, 2));
				objetivo->ubicacion.y = atoi(string_substring(pos, 2, 2));
			}
			log_info(archivoLog,"Obtuve posicion x:%d, y: %d.", objetivo->ubicacion.x, objetivo->ubicacion.y);
		}else if((*movimiento = siguienteMovimiento(mapa->miPosicion, objetivo, *movimiento))){ //Me muevo

			//Creo el mensaje
			char mensaje[2];
			mensaje[0] = 'M';

			switch(*movimiento){
			case 1:
				mensaje[1] = '1';
				mapa->miPosicion.x--;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 2:
				mensaje[1] = '2';
				mapa->miPosicion.y++;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 3:
				mensaje[1] = '3';
				mapa->miPosicion.x++;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);

				break;
			case 4:
				mensaje[1] = '4';
				mapa->miPosicion.y--;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			}
			log_info(archivoLog,"Envio el mensaje: %c%c",mensaje[0],mensaje[1]);

			//Envio el mensaje
			int resp = send(serverMapa, &mensaje, 2, 0);
			if(resp == -1){
				log_info(archivoLog,"No pude enviar el mensaje: %c%c",mensaje[0],mensaje[1]);
				exit(EXIT_FAILURE);
			}

		}else{
			//Creo el mensaje
			char* mensaje = string_new();
			string_append(&mensaje, "F");
			string_append(&mensaje, objetivo->nombre);
			log_info(archivoLog,"el mensaje que voy a enviar es: %s",mensaje);

			//Envio el mensaje
			int resp = send(serverMapa, mensaje, 2, 0);
			if(resp <0){
				log_info(archivoLog,"No pude enviar el mensaje: %s", mensaje);
				exit(EXIT_FAILURE);
			}
			log_info(archivoLog,"Envie el mensaje %s",mensaje);

			time_t tiempoInicialBloqueo;
			time(&tiempoInicialBloqueo);

			char conf;
			if (recv(serverMapa, &conf, 1,  0) == 1){
				sleep(1);
				time_t tiempoFinalBloqueo;
				time(&tiempoFinalBloqueo);
				entrenador->tiempoBloqueado += tiempoFinalBloqueo - tiempoInicialBloqueo;

				if (conf=='C'){
					printf("Capture el objetivo\n");
					char* size = malloc(sizeof(char)*11);

					if (recv(serverMapa, size, 11,  0) == 11){
						int size_int = atoi(size);
						char* temp = malloc(sizeof(char)*size_int);
						if (recv(serverMapa, temp, size_int,  0) == size_int){
							char* path=string_substring(temp,0,size_int);
							string_append(&path,"\0");
							copiarPokemonFile(path);
							objetivo->logrado = 1;
						}
					}
				}else if (conf=='K'){
					entrenador->deadlocks++;
					if (recv(serverMapa, &conf, 1,  0) == 1 && conf=='C'){
						printf("Capture el objetivo\n");
						char* size = malloc(sizeof(char)*11);

						if (recv(serverMapa, size, 11,  0) == 11){
							int size_int = atoi(size);
							char* temp = malloc(sizeof(char)*size_int);
							if (recv(serverMapa, temp, size_int,  0) == size_int){
								char* path=string_substring(temp,0,size_int);
								string_append(&path,"\0");
								copiarPokemonFile(path);
								objetivo->logrado = 1;
							}
						}
					}else{
						entrenador->muertes++;
						printf("Muerte por deadlock");
						objetivo->logrado = 0;
						return 0;
					}

				}
				return 1;
			}


		}

	}
	return 0;
}

int procesarMapa(t_mapa* mapa){
	int serverMapa;
	create_socketClient(&serverMapa, mapa->ip, mapa->puerto);

	log_info(archivoLog,"Me identifico con el mapa %s", mapa->nombre);
	int resp = send(serverMapa, &(entrenador->simbolo), 1, 0);
	if(resp == -1){
		log_info(archivoLog,"No me pude identificar con el mapa");
		close(serverMapa);
		return 0;
	}


	//Recorro los objetivos  y los proceso
	int j;
	int movimiento = 0;
	//time(&entrenador->tiempoTotal);
	for(j=0; j<list_size(mapa->objetivos); j++){
		t_objetivo* objetivo = (t_objetivo *)list_get(mapa->objetivos, j);
		log_info(archivoLog, "Proceso objetivo %s", objetivo->nombre);
		procesarObjetivo(mapa, objetivo, &movimiento, serverMapa);
		if(objetivo->logrado == 0){
			close(serverMapa);
			return 1;
		}

	}


	t_objetivo* obj = getNextObjective(mapa);
	if(obj == NULL){
		mapa->terminado=1;
		copiarMedalla(mapa);
	}else{
		printf("Objetivo fallido %c\n", obj->nombre[0]);
	}

	close(serverMapa);
	return 1;
}
