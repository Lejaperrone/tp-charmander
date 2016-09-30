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
	printf("Inicio objetivo %s.\n", objetivo->nombre);

	char quantum[7];

	while(objetivo->logrado==0 && recv(serverMapa, (void*)quantum, 6, 0) <= 6 /*&& strcmp(quantum, "QUANTUM")*/){ //aca deberia esperar al siguiente quantum.
		if(objetivo->ubicacion.x==-1 || objetivo->ubicacion.y==-1){ //Obtengo ubicacion de pokenest
			//Creo el mensaje
				char* mensaje = string_new();
				string_append(&mensaje, "CAPTU");
				string_append(&mensaje, objetivo->nombre);
				string_append(&mensaje, &entrenador->simbolo);
				printf("el mensaje que voy a enviar es: %s\n",mensaje);
				//string_append(&mensaje,(char)*socketEntrenador);


			//Envio el mensaje
				int pedirPokenest(int serverMapa, char* mensaje, int size, int s){
					return send(serverMapa, mensaje, 7, 0);
				}
				int resp = pedirPokenest(serverMapa, mensaje, 7, 0);
				if(resp == -1){
					printf("No pude enviar el mensaje: %s\n", mensaje);
					exit(EXIT_FAILURE);
				}
				printf("envie el mensaje %s\n",mensaje);

			//Espero la respuesta
				char x[2], y[2];
				//if (recv(serverMapa, (void*)x, 2, 0) == 2 && recv(serverMapa, (void*)y, 2, 0) == 2)
				int recibirPosicionX(int serverMapa, char* mensaje, int size, int s){
					return recv(serverMapa,(void*)mensaje,2,0);
				}
				int recibirPosicionY(int serverMapa, char* mensaje, int size, int s){
									return recv(serverMapa,(void*)mensaje,2,0);
				}

				if (recibirPosicionX(serverMapa, (void*)mensaje,2, 0)<=2)
				{
					x[0]=mensaje[0];
					x[1]=mensaje[1];
					objetivo->ubicacion.x = atoi(x);
				}
				if (recibirPosicionY(serverMapa, (void*)mensaje,2, 0)<=2){
					y[0]=mensaje[0];
					y[1]=mensaje[1];
					objetivo->ubicacion.y=atoi(y);
				}
				printf("Obtuve posicion x:%d, y: %d.\n", objetivo->ubicacion.x, objetivo->ubicacion.y);
		}else if((*movimiento = siguienteMovimiento(mapa->miPosicion, objetivo, *movimiento))){ //Me muevo

			//Creo el mensaje
				char* mensaje = string_new();
				string_append(&mensaje, "MOVER");

			switch(*movimiento){
			case 1:
				string_append(&mensaje, "1");
				mapa->miPosicion.x--;
				printf("Me muevo hacia %d %d\n",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 2:
				string_append(&mensaje, "2");
				mapa->miPosicion.y++;
				printf("Me muevo hacia %d %d\n",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			case 3:
				string_append(&mensaje, "3");
				mapa->miPosicion.x++;
				printf("Me muevo hacia %d %d\n",mapa->miPosicion.x, mapa->miPosicion.y);

				break;
			case 4:
				string_append(&mensaje, "4");
				mapa->miPosicion.y--;
				printf("Me muevo hacia %d %d\n",mapa->miPosicion.x, mapa->miPosicion.y);
				break;
			}
			string_append(&mensaje,&entrenador->simbolo);
			printf("Envio el mensaje: %s\n",mensaje);

			//Envio el mensaje
			int moverse(int serverMapa,char** mensaje, int size, int s){
				return send(serverMapa, *mensaje, 8, 0);
			}
			int resp = moverse(serverMapa, &mensaje, 8, 0);
				if(resp == -1){
					printf("No pude enviar el mensaje: %s\n", mensaje);
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
		}

	}
}

void procesarMapa(t_mapa* mapa){
	int serverMapa;
	//Defino el socket con el que se va a manejar el entrenador durante todo el transcurso del mapa

	//Me conecto al mapa
		printf("Conectandose al mapa %s...\n", mapa->nombre);
		create_socketClient(&serverMapa, mapa->ip, mapa->puerto);
		printf("Conectado al mapa %s.\n", mapa->nombre);

	//Le paso mi simbolo al  mapa
		printf("Me identifico con el mapa como: %c\n", entrenador->simbolo);
		int resp = send(serverMapa, &(entrenador->simbolo), 2, 0);
		if(resp == -1){
			printf("No me pude identificar con el mapa");
			exit(EXIT_FAILURE);
		}


	//Recorro los objetivos  y los proceso
		int j;
		int movimiento = 0;
		for(j=0; j<list_size(mapa->objetivos); j++){
			t_objetivo* objetivo = (t_objetivo *)list_get(mapa->objetivos, j);
			procesarObjetivo(mapa, objetivo, &movimiento, serverMapa);

		}

		close(serverMapa);
}
