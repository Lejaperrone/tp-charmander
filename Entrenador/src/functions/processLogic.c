/*
 * procesamiento.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

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



void procesarObjetivo(t_mapa* mapa, t_objetivo* objetivo, int* movimiento, int serverMapa){
	char quantum;
	log_info(archivoLog, "Arranco objetivo %s en mapa %s con mov %d", objetivo->nombre, mapa->nombre, *movimiento);

	while(objetivo->logrado==0 && recv(serverMapa, &quantum, 1, 0) && quantum=='Q'){ //aca deberia esperar al siguiente quantum.
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

			char conf;
			if (recv(serverMapa, &conf, 1,  0) == 1 && conf=='C'){
				objetivo->logrado = 1;
			}

			log_info(archivoLog,"Fin del objetivo");
		}

	}
}

void procesarMapa(t_mapa* mapa){
	int serverMapa;
	//Defino el socket con el que se va a manejar el entrenador durante todo el transcurso del mapa

	//Me conecto al mapa
		log_info(archivoLog,"Conectandose al mapa %s...", mapa->nombre);
		create_socketClient(&serverMapa, mapa->ip, mapa->puerto);
		log_info(archivoLog,"Conectado al mapa %s.", mapa->nombre);

	//Le paso mi simbolo al  mapa
		log_info(archivoLog,"Me identifico con el mapa como: %c", entrenador->simbolo);
		int resp = send(serverMapa, &(entrenador->simbolo), 1, 0);
		if(resp == -1){
			log_info(archivoLog,"No me pude identificar con el mapa");
			exit(EXIT_FAILURE);
		}


	//Recorro los objetivos  y los proceso
		int j;
		int movimiento = 0;
		for(j=0; j<list_size(mapa->objetivos); j++){
			t_objetivo* objetivo = (t_objetivo *)list_get(mapa->objetivos, j);
			log_info(archivoLog, "Proceso objetivo %s", objetivo->nombre);
			procesarObjetivo(mapa, objetivo, &movimiento, serverMapa);

		}

		close(serverMapa);
}
