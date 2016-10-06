/*
 * planificador.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

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

#include "../commons/structures.h"
#include "../functions/collections_list_extension.h"


void procesarEntrenadoresPreparados(){
	int i;
	if(list_size(entrenadoresPreparados)>0){
		for(i=0; i<list_size(entrenadoresPreparados); i++){
			t_entrenador* entrenador = (t_entrenador*)list_remove(entrenadoresPreparados, i);
			log_trace(archivoLog, "Planificador - Agrego entrenador a listos: %c", entrenador->simbolo);
			list_add(entrenadoresListos, entrenador);
			CrearPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
		}
		nivel_gui_dibujar(elementosUI,mapa->nombre);
	}
}
void procesarEntrenadoresBloqueados(){
	//TODO
}
void liberarPokemonsDelEntrenador(t_entrenador* entrenador){
	int longitudListaPokemons = list_size(entrenador->pokemons);
	int j;
	for(j=1; j<=longitudListaPokemons; j++){
		t_pokemon_custom* pokemon = list_get(entrenador->pokemons,j);
		pokemon->disponible = 1;
	}

	list_destroy(entrenador->pokemons);
}
void procesarEntrenadorGarbageCollector(){
	if (!list_is_empty(garbageCollectorEntrenadores)){
		int longitudLista = list_size(garbageCollectorEntrenadores);
		int i;
		for(i=1; i<=longitudLista; i++){
			t_entrenador* entrenador = list_get(garbageCollectorEntrenadores,i);
			liberarPokemonsDelEntrenador(entrenador);
			BorrarItem(elementosUI, entrenador->simbolo);
			nivel_gui_dibujar(elementosUI, mapa->nombre);
			close(entrenador->socket);
			free(entrenador);
		}
	}
}

int recvWithGarbageCollector(int socket, char* package, int cantBytes, t_entrenador* entrenador){
	int nbytes = recv(socket, package, cantBytes, 0);
	if(nbytes!=cantBytes){
		log_trace(archivoLog, "Planificador - Mando a %c al Garbage collector", entrenador->simbolo);
		list_add(garbageCollectorEntrenadores, entrenador);
		return 0;
	}

	return 1;
}
int sendWithGarbageCollector(int socket, char* package, int cantBytes, t_entrenador* entrenador){
	int nbytes = send(socket, package, cantBytes, 0);
	if(nbytes!=cantBytes){
		log_trace(archivoLog, "Planificador - Mando a %c al Garbage collector", entrenador->simbolo);
		list_add(garbageCollectorEntrenadores, entrenador);
		return 0;
	}

	return 1;
}

t_entrenador* obtenerSiguienteEntrenadorPlanificadoRR(t_entrenador* entrenadorAnterior){
	if(entrenadorAnterior != NULL){
		if(list_size(entrenadoresListos)>0){
			t_entrenador* otroEntrenador = list_get(entrenadoresListos, list_size(entrenadoresListos)-1);

			if(otroEntrenador->simbolo==entrenadorAnterior->simbolo && entrenadorAnterior->planificador.quantum>0){
				list_remove(entrenadoresListos, list_size(entrenadoresListos)-1);

				entrenadorAnterior->planificador.quantum--;
				return entrenadorAnterior;
			}else{
				entrenadorAnterior->planificador.quantum=0;

				if(list_size(entrenadoresListos)>0){
					t_entrenador* proximoEntrenador = list_remove(entrenadoresListos, 0);
					proximoEntrenador->planificador.quantum = mapa->quantum-1;
					return proximoEntrenador;
				}else{
					return NULL;
				}
			}
		}else{
			entrenadorAnterior->planificador.quantum=0;
			return NULL;
		}
	}else{
		if(list_size(entrenadoresListos)>0){
			t_entrenador* proximoEntrenador = list_remove(entrenadoresListos, 0);
			proximoEntrenador->planificador.quantum = mapa->quantum-1;
			return proximoEntrenador;
		}else{
			return NULL;
		}
	}
}
t_entrenador* obtenerSiguienteEntrenadorPlanificadoSRDF(t_entrenador* entrenadorAnterior){
	//TODO
	return NULL;
}

void atenderEntrenadorUbicacionPokenest(t_entrenador* entrenador){
	char paquete;
	if(recvWithGarbageCollector(entrenador->socket, &paquete, 1,entrenador)){
		log_trace(archivoLog, "Planificador - Solicitud U%c", paquete);
		t_pokenest* pokenestObjetivo = find_pokenest_by_id(paquete);
		log_trace(archivoLog, "Planificador - Encontre la pokenest %c");

		char* posx=malloc(sizeof(char)*3);
		sprintf(posx,"%i",pokenestObjetivo->ubicacion.x);

		char* posy=malloc(sizeof(char)*3);
		sprintf(posy,"%i",pokenestObjetivo->ubicacion.y);

		char pos[4];
		pos[0] = posx[0];
		pos[1]=posx[1];
		pos[2]=posy[0];
		pos[3]=posy[1];
		if(sendWithGarbageCollector(entrenador->socket, pos,5,entrenador)){
			log_trace(archivoLog, "Planificador - Posicion enviada %d, %d", pokenestObjetivo->ubicacion.x, pokenestObjetivo->ubicacion.y);
			list_add(entrenadoresListos, entrenador);
		}
	}
}
void atenderEntrenadorMover(t_entrenador* entrenador){
	char paquete;
	if(recvWithGarbageCollector(entrenador->socket, &paquete, 1,entrenador)){
		log_trace(archivoLog, "Planificador - Solicitud M%c", paquete);
		int despl = atoi(&paquete);
		switch(despl){
			case 1:
				entrenador->ubicacion.x--; //Arriba
				break;
			case 2:
				entrenador->ubicacion.y++; //Derecha
				break;
			case 3:
				entrenador->ubicacion.x++; //Abajo
				break;
			case 4:
				entrenador->ubicacion.y--; //Izquierda
				break;
		}

		MoverPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
		nivel_gui_dibujar(elementosUI,mapa->nombre);
		list_add(entrenadoresListos, entrenador);

		log_trace(archivoLog, "Planificador - Hacia %d, %d", entrenador->ubicacion.x, entrenador->ubicacion.y);
	}
}
void atenderEntrenadorCapturar(t_entrenador* entrenador){
	char paquete;

	if(recvWithGarbageCollector(entrenador->socket, &paquete, 1,entrenador)){
		log_trace(archivoLog, "Planificador - Solicitud F%c", paquete);
		entrenador->pokenestBloqueante = find_pokenest_by_id(paquete);
		list_add(entrenadoresBloqueados, entrenador);
		log_trace(archivoLog, "Planificador - Lo mando a la lista de bloqueados");
	}
}
void atenderEntrenador(t_entrenador* entrenador){
	char paquete;
	log_trace(archivoLog, "Planificador - Inicio atencion de %c", entrenador->simbolo);
	if(recvWithGarbageCollector(entrenador->socket, &paquete, 1,entrenador)){
		switch(paquete){
			case 'U': //Ubicacion de pokenest
				atenderEntrenadorUbicacionPokenest(entrenador);
				break;
			case 'M': //Solicitud para moverse
				atenderEntrenadorMover(entrenador);
				break;
			case 'F':
				atenderEntrenadorCapturar(entrenador);
				break;
			default:
				log_trace(archivoLog, "Planificador - Solicitud desconocida: %c", paquete);
				break;
		}
	}
}

void* planificador(void* arg){
	log_trace(archivoLog, "Planificador - Arranca");

	char Q = 'Q';
	t_entrenador* entrenador = NULL;
	while(1){
		if(!strcmp(mapa->algoritmo, "RR")){
			entrenador = obtenerSiguienteEntrenadorPlanificadoRR(entrenador);
		}else{
			entrenador = obtenerSiguienteEntrenadorPlanificadoSRDF(entrenador);
		}

		//procesarEntrenadorGarbageCollector();
		//procesarEntrenadoresBloqueados();
		procesarEntrenadoresPreparados();

		if(entrenador != NULL){
			log_trace(archivoLog, "Planificador - Envio turno a %c", entrenador->simbolo);
			if(sendWithGarbageCollector(entrenador->socket, &Q, 1, entrenador)){
				atenderEntrenador(entrenador);
			}
		}
		sleep(1);
	}

	return arg;
}


