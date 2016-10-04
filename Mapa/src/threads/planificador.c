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

t_pokenest *find_pokenest_by_id(char id) {
	int _is_the_one(t_pokenest *p) {
			return (p->identificador==id);
	}
    return list_find(mapa->pokeNests, (void*) _is_the_one);
}

void procesarEntrenadoresPreparados(){
	int i;
	for(i=0; i<list_size(entrenadoresPreparados); i++){
		t_entrenador* entrenador = (t_entrenador*)list_get(entrenadoresPreparados,i);
		list_add(entrenadoresListos, entrenador);
		(t_entrenador*)list_remove(entrenadoresPreparados, i);
		CrearPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
		list_add(elementosUI,&entrenador->simbolo);
	}
	nivel_gui_dibujar(elementosUI,mapa->nombre);
}

void atenderEntrenador(t_entrenador* entrenador){
	int nbytes;
	char paquete[1];
	if((nbytes = recv(*entrenador->socket, &paquete, 1,0)) ==1){
		switch(paquete[0]){
			case 'U':
				if((nbytes = recv(*entrenador->socket, &paquete, 1,0)) ==1){
					t_pokenest* pokenestObjetivo = find_pokenest_by_id(paquete[0]);
					char* ubic = string_new();

					char* posx=malloc(sizeof(char));
					sprintf(posx,"%i",pokenestObjetivo->ubicacion.x);
					if(entrenador->ubicacion.x<=9){
						string_append(&ubic,"0");
					}
					string_append(&ubic,posx);

					char* posy=malloc(sizeof(char));
					sprintf(posy,"%i",pokenestObjetivo->ubicacion.y);
					if(entrenador->ubicacion.y<=9){
						string_append(&ubic,"0");
					}
					string_append(&ubic,posy);

					send(*entrenador->socket, &ubic,4,0);
				}else{
					//Se desconecta debido a procesamiento indebido de mensaje
				}
				break;
			case 'M':
				if((nbytes = recv(*entrenador->socket, &paquete, 1,0)) ==1){
					int despl = atoi(paquete);
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
				break;
		}
	}
}



void* planificador(void* arg){
	log_trace(archivoLog, "Planificador - Arranca");

	while(1){
		procesarEntrenadoresPreparados();

		int i;
		for(i=0; i<list_size(entrenadoresListos); i++){
			t_entrenador* entrenador = (t_entrenador*)list_remove(entrenadoresListos, i);
			send(*entrenador->socket,"QUANTUM",7,0);
			atenderEntrenador(entrenador);
		}

		sleep(mapa->retardo);
	}
}


