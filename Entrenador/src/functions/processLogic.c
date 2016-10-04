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
				char x[2], y[2];
				if (recv(serverMapa, (void*)x, 2, 0) == 2 && recv(serverMapa, (void*)y, 2, 0) == 2){
					objetivo->ubicacion.x = atoi(x);
					objetivo->ubicacion.y=atoi(y);
				}

				log_info(archivoLog,"Obtuve posicion x:%d, y: %d.", objetivo->ubicacion.x, objetivo->ubicacion.y);
		}else if((*movimiento = siguienteMovimiento(mapa->miPosicion, objetivo, *movimiento))){ //Me muevo

			//Creo el mensaje
				char* mensaje = string_new();
				string_append(&mensaje, "M");

			switch(*movimiento){
			case 1:
				string_append(&mensaje, "1");
				mapa->miPosicion.x--;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 2:
				string_append(&mensaje, "2");
				mapa->miPosicion.y++;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 3:
				string_append(&mensaje, "3");
				mapa->miPosicion.x++;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);

				break;
			case 4:
				string_append(&mensaje, "4");
				mapa->miPosicion.y--;
				log_info(archivoLog,"Me muevo hacia %d %d",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			}
			string_append(&mensaje,&entrenador->simbolo);
			log_info(archivoLog,"Envio el mensaje: %s\n",mensaje);

			//Envio el mensaje
			int resp = send(serverMapa, &mensaje, 2, 0);
				if(resp == -1){
					log_info(archivoLog,"No pude enviar el mensaje: %s", mensaje);
					exit(EXIT_FAILURE);
				}

		}else{ //Tengo que solicitar el pokemon
			/*char* mensaje ="FINOB";
			int resp = send(serverMapa, &mensaje, 6, 0);
			if(resp == -1){
				printf("No pude enviar el mensaje: %s\n", mensaje);
				exit(EXIT_FAILURE);
			}*/

			objetivo->logrado = 1;
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
