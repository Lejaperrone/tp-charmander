/*
 * planificador.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#include "../commons/structures.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>
#include <tad_items.h>
#include <curses.h>
#include <nivel.h>
#include <unistd.h>

t_pokenest *find_pokenest_by_id(char id) {
	int _is_the_one(t_pokenest *p) {
			return (p->identificador==id);
	}
    return list_find(mapa->pokeNests, (void*) _is_the_one);
}

void procesarEntrenadoresPreparados(){
	int i;
	log_trace(archivoLog, "Planificador - Hay %d entrenador/es preparado/s", list_size(entrenadoresPreparados));
	for(i=0; i<list_size(entrenadoresPreparados); i++){
		t_entrenador* entrenador = (t_entrenador*)list_remove(entrenadoresPreparados, i);
		log_trace(archivoLog, "Planificador - Agrego entrenador a listos: %c", entrenador->simbolo);
		list_add(entrenadoresListos, entrenador);
		CrearPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
	}
	nivel_gui_dibujar(elementosUI,mapa->nombre);
}

void atenderEntrenador(t_entrenador* entrenador){
	int nbytes;
	char paquete;
	log_trace(archivoLog, "Planificador - Inicio atencion de %c", entrenador->simbolo);
	if((nbytes = recv(entrenador->socket, &paquete, 1,0)) ==1){
		switch(paquete){
			case 'U':
				if((nbytes = recv(entrenador->socket, &paquete, 1,0)) ==1){
					log_trace(archivoLog, "Planificador - Entrenador: %c solicito U con %c", entrenador->simbolo, paquete);
					t_pokenest* pokenestObjetivo = find_pokenest_by_id(paquete);
					log_trace(archivoLog, "Planificador - Encontre la pokenest");

					char* posx=malloc(sizeof(char)*3);
					sprintf(posx,"%i",pokenestObjetivo->ubicacion.x);

					char* posy=malloc(sizeof(char)*3);
					sprintf(posy,"%i",pokenestObjetivo->ubicacion.y);

					char pos[4];
					pos[0] = posx[0];
					pos[1]=posx[1];
					pos[2]=posy[0];
					pos[3]=posy[1];
					send(entrenador->socket, pos,5,0);


					log_trace(archivoLog, "Planificador - Le envie la informacion %d, %d", pokenestObjetivo->ubicacion.x, pokenestObjetivo->ubicacion.y);
				}else{
					//Se desconecta debido a procesamiento indebido de mensaje
				}
				break;
			case 'M':
				if((nbytes = recv(entrenador->socket, &paquete, 1,0)) ==1){
					log_trace(archivoLog, "Planificador - Entrenador: %c solicito M con %c", entrenador->simbolo, paquete);
					int despl = atoi(&paquete);
					switch(despl){
						case 1:
							entrenador->ubicacion.x = entrenador->ubicacion.x -1; //Arriba
							break;
						case 2:
							entrenador->ubicacion.x = entrenador->ubicacion.y +1; //Derecha
							break;
						case 3:
							entrenador->ubicacion.x = entrenador->ubicacion.x +1; //Abajo
							break;
						case 4:
							entrenador->ubicacion.x = entrenador->ubicacion.y -1; //Izquierda
							break;
					}

					MoverPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
				}else{
					//Se desconecta debido a procesamiento indebido de mensaje
				}
				break;
			case 'F':
				log_trace(archivoLog, "Planificador - Entrenador: %c solicito F", entrenador->simbolo);
				break;
			default:
				log_trace(archivoLog, "Planificador - Entrenador: %c solicito desconocida: %s", entrenador->simbolo, paquete);
				break;
		}
	}
	sleep(mapa->retardo/1000);
}



void* planificador(void* arg){
	log_trace(archivoLog, "Planificador - Arranca");
	int count = 0;
	while(1){
		procesarEntrenadoresPreparados();

		int i;
		log_info(archivoLog,"Planiaficador - %d entrenadores listos", list_size(entrenadoresListos));
		for(i=0; i<list_size(entrenadoresListos); i++){
			t_entrenador* entrenador = (t_entrenador*)list_remove(entrenadoresListos, i);
			log_trace(archivoLog, "Planificador - Envio turno a %c", entrenador->simbolo);
			char Q = 'Q';
			int nbytes = send(entrenador->socket, &Q, 1, 0);
			log_trace(archivoLog, "Enviado : %d", nbytes);
			atenderEntrenador(entrenador);
		}

		sleep(5);
		count++;
		if(count==100){
			return arg;
		}
	}
}


