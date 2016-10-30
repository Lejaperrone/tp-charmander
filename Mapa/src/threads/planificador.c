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
#include <limits.h>
#include <commons/string.h>
#include <tad_items.h>
#include <curses.h>
#include <nivel.h>
#include <unistd.h>
#include <pthread.h>

#include "../commons/structures.h"
#include "../functions/collections_list_extension.h"
#include "../functions/recursos.h"
#include "../functions/signals.h"

extern pthread_mutex_t mutexEntrBQ;

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

void logEntrenadoresListos(){
	int i;
	char * mensaje = string_new();
	string_append(&mensaje, "Entrenadores listos: ");
	for(i=0; i<list_size(entrenadoresListos); i++){
		t_entrenador* entrenador = list_get(entrenadoresListos, i);
		string_append(&mensaje, &(entrenador->simbolo));
		string_append(&mensaje, " ");
	}

	log_info(archivoLog, mensaje);
}
void logEntrenadoresBloqueados(){
	int i;
	char * mensaje = string_new();
	string_append(&mensaje, "Entrenadores bloqueados: ");
	for(i=0; i<list_size(entrenadoresBloqueados); i++){
		t_entrenador* entrenador = list_get(entrenadoresBloqueados, i);
		string_append(&mensaje, &(entrenador->simbolo));
		string_append(&mensaje, " ");
	}
	log_info(archivoLog, mensaje);
}
void logColasEntrenadores(){
	logEntrenadoresListos();
	logEntrenadoresBloqueados();
}

void procesarEntrenadoresPreparados(){
	int i;
	if(list_size(entrenadoresPreparados)>0){
		for(i=0; i<list_size(entrenadoresPreparados); i++){
			t_entrenador* entrenador = (t_entrenador*)list_remove(entrenadoresPreparados, i);
			log_info(archivoLog, "Planificador - Agrego entrenador %c a lista de Listos", entrenador->simbolo);
			list_add(entrenadoresListos, entrenador);
			CrearPersonaje(elementosUI,entrenador->simbolo,entrenador->ubicacion.x,entrenador->ubicacion.y);
		}
		nivel_gui_dibujar(elementosUI,mapa->nombre);

		logColasEntrenadores();
	}
}
void procesarEntrenadoresBloqueados(){
	bool _pokemon_Este_Disponible(t_pokemon_custom* pokemon){
		return (pokemon->disponible == 1);
	}
	int _asignar_PokemonsDisponible_PorElCualElEntrenadorSeBloqueo(t_entrenador* entrenador){
		log_trace(archivoLog, "Planificador - %c quiere %c",entrenador->simbolo, entrenador->pokenestBloqueante->identificador);
		t_pokemon_custom* pokemon = list_find(entrenador->pokenestBloqueante->pokemons, (void*)_pokemon_Este_Disponible);
		if(pokemon != NULL){
			log_trace(archivoLog, "Planificador - encontre uno disponible %s",pokemon->path);
			pokemon->disponible = 0;
			pokemon->duenio = entrenador->simbolo;
			list_add(entrenador->pokemons, pokemon);
			restarRecurso(elementosUI,entrenador->pokenestBloqueante->identificador);

			char* C = "C";
			if(sendWithGarbageCollector(entrenador->socket, C, 1, entrenador)){
				entrenador->planificador.ubicacionObjetivo.x=-1;
				entrenador->planificador.ubicacionObjetivo.y=-1;
				log_trace(archivoLog, "Planificador - envie confirmacion");
				return 1;
			}else{
				pokemon->disponible = 1;
				pokemon->duenio = ' ';
				sumarRecurso(elementosUI,pokemon->identificadorPokenest);
				return 2;
			}
		}else{
			return 0;
		}
	}

	int i;
	if(!list_is_empty(entrenadoresBloqueados)){
		for(i=0 ;i<list_size(entrenadoresBloqueados); i++){
			//pthread_mutex_lock(&mutexEntrBQ);
			t_entrenador* entrenador = list_get(entrenadoresBloqueados,i);
			int valorDeRetorno = _asignar_PokemonsDisponible_PorElCualElEntrenadorSeBloqueo(entrenador);
			switch(valorDeRetorno){
			case 1:
				list_remove(entrenadoresBloqueados,i);
				list_add(entrenadoresListos,entrenador);
				break;
			case 2:
				list_remove(entrenadoresBloqueados,i);
				break;
			}
			//pthread_mutex_unlock(&mutexEntrBQ);
		}
	}

	nivel_gui_dibujar(elementosUI, mapa->nombre);
}
void procesarEntrenadoresGarbageCollector(){
	void _procesar_Entrenador_GarbageCollector(t_entrenador* entrenador){
		void _procesor_Pokemon_Entrenador_GarbageCollector(t_pokemon_custom* pokemon){
			pokemon->disponible = 1;
			pokemon->duenio = ' ';
			sumarRecurso(elementosUI,pokemon->identificadorPokenest);
		}

		list_iterate(entrenador->pokemons, (void*) _procesor_Pokemon_Entrenador_GarbageCollector);
		list_destroy(entrenador->pokemons);
		BorrarItem(elementosUI, entrenador->simbolo);
		close(entrenador->socket);
		free(entrenador);
	}

	if (!list_is_empty(garbageCollectorEntrenadores)){
		list_iterate(garbageCollectorEntrenadores, (void*) _procesar_Entrenador_GarbageCollector);
		list_clean(garbageCollectorEntrenadores);
	}

	nivel_gui_dibujar(elementosUI, mapa->nombre);
}

int calcularDistanciaEntrenadorObjetivo(t_entrenador* entrenador){
	if(entrenador->planificador.ubicacionObjetivo.x!=-1 && entrenador->planificador.ubicacionObjetivo.y!=-1){
		int distanciaX = entrenador->planificador.ubicacionObjetivo.x - entrenador->ubicacion.x;
		if(distanciaX <0){
			distanciaX = distanciaX * -1;
		}

		int distanciaY = entrenador->planificador.ubicacionObjetivo.y - entrenador->ubicacion.y;
		if(distanciaY <0){
			distanciaY = distanciaY * -1;
		}

		return distanciaX + distanciaY;
	}else{
		return -1;
	}
}
t_entrenador* obtenerProximoEntrenadorCercano(){
	t_entrenador* entrenador = NULL;
	int distanciaMin = INT_MAX;

	//Busco entrenadores que no se la ubicacion del objetivo
		bool _entrenador_objetivo_desconocido(t_entrenador* entrenador){
			return (entrenador->planificador.ubicacionObjetivo.x==-1 || entrenador->planificador.ubicacionObjetivo.y==-1);
		}
		entrenador = list_remove_by_condition(entrenadoresListos, (void*)_entrenador_objetivo_desconocido);
		if(entrenador != NULL){
			entrenador->planificador.quantum = -2;
		}
	//Busco el enternador que este mas cerca al objetivo.
		if(entrenador == NULL){
			if(list_size(entrenadoresListos)>0){
				void _entrenador_objetivo_distancia(t_entrenador* entrenador){
					int distanciaObj = calcularDistanciaEntrenadorObjetivo(entrenador);
					if(distanciaMin>distanciaObj){
						distanciaMin = distanciaObj;
					}
				}
				list_iterate(entrenadoresListos, (void*)_entrenador_objetivo_distancia);

				bool _entrenador_objetivo_min(t_entrenador* entrenador){
					return (calcularDistanciaEntrenadorObjetivo(entrenador) == distanciaMin);
				}
				entrenador = list_remove_by_condition(entrenadoresListos, (void*)_entrenador_objetivo_min);
				if(entrenador != NULL){
					entrenador->planificador.quantum = -1;
				}
			}
		}

	return entrenador;
}

t_entrenador* obtenerSiguienteEntrenadorPlanificadoRR(t_entrenador* entrenadorAnterior){
	if(entrenadorAnterior != NULL){
		if(entrenadorAnterior->planificador.quantum == -2){
			entrenadorAnterior->planificador.quantum = 0;
		}

		if(list_size(entrenadoresListos)>0){
			t_entrenador* otroEntrenador = list_get(entrenadoresListos, list_size(entrenadoresListos)-1);
			if(entrenadorAnterior->planificador.quantum == -1 && otroEntrenador->simbolo == entrenadorAnterior->simbolo){
				list_remove(entrenadoresListos, list_size(entrenadoresListos)-1);
				return entrenadorAnterior;
			}else{
				if(entrenadorAnterior->planificador.quantum == -1){
					entrenadorAnterior->planificador.quantum =  0;
				}

				if(otroEntrenador->simbolo==entrenadorAnterior->simbolo && entrenadorAnterior->planificador.quantum>0){
					list_remove(entrenadoresListos, list_size(entrenadoresListos)-1);

					entrenadorAnterior->planificador.quantum--;
					return entrenadorAnterior;
				}else{
					entrenadorAnterior->planificador.quantum=0;
					log_info(archivoLog, "Planficador - El entrenador %c va al final de la cola de Listos", entrenadorAnterior->simbolo);

					if(list_size(entrenadoresListos)>0){
						t_entrenador* proximoEntrenador = list_remove(entrenadoresListos, 0);
						proximoEntrenador->planificador.quantum = mapa->quantum-1;

						log_info(archivoLog, "Planficador - Planifico al entrenador %c", proximoEntrenador->simbolo);
						logColasEntrenadores();
						return proximoEntrenador;
					}else{
						return NULL;
					}
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
	if(entrenadorAnterior != NULL){
		log_trace(archivoLog, "Planificador - SRDF - Entrenador anterior no es null");
		if(list_size(entrenadoresListos)>0){
			log_trace(archivoLog, "Planificador - SRDF - Tengo entrenadores listos");
			t_entrenador* otroEntrenador = list_get(entrenadoresListos, list_size(entrenadoresListos)-1);
			if(otroEntrenador->simbolo == entrenadorAnterior->simbolo){
				log_trace(archivoLog, "Planificador - SRDF - Entrenador anterior y ultimo listo coinciden");
				if(entrenadorAnterior->planificador.quantum>0){
					log_trace(archivoLog, "Planificador - SRDF - entrenador  anterior le queda quantum");
					return obtenerSiguienteEntrenadorPlanificadoRR(entrenadorAnterior);
				}else if(entrenadorAnterior->planificador.quantum == -1){
					log_trace(archivoLog, "Planificador - SRDF - Quantum -1");
					list_remove(entrenadoresListos, list_size(entrenadoresListos)-1);
					return entrenadorAnterior;
				}else if(entrenadorAnterior->planificador.quantum == -2){
					log_trace(archivoLog, "Planificador - SRDF - Quantum -2");
					entrenadorAnterior->planificador.quantum = 0;
					return obtenerProximoEntrenadorCercano();
				}else{
					log_trace(archivoLog, "Planificador - SRDF - Otro quantum, error");
					return obtenerProximoEntrenadorCercano();
				}
			}else{
				log_trace(archivoLog, "Planificador - SRDF - Entrenador  anterior  no  coincide con  ultimo listo");
				entrenadorAnterior->planificador.quantum=0;
				return obtenerProximoEntrenadorCercano();
			}
		}else{
			log_trace(archivoLog, "Planificador - SRDF - No hay entrenadores listos");
			return NULL;
		}
	}else{
		log_trace(archivoLog, "Planificador - SRDF - Entrenador anterior es null");
		return obtenerProximoEntrenadorCercano();
	}
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
			entrenador->planificador.ubicacionObjetivo.x = pokenestObjetivo->ubicacion.x;
			entrenador->planificador.ubicacionObjetivo.y = pokenestObjetivo->ubicacion.y;
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
		log_info(archivoLog, "Planificador - Envio a %c a la lista de bloqueados", entrenador->simbolo);
		entrenador->pokenestBloqueante = find_pokenest_by_id(paquete);
		pthread_mutex_lock(&mutexEntrBQ);
		list_add(entrenadoresBloqueados, entrenador);
		logColasEntrenadores();
		pthread_mutex_unlock(&mutexEntrBQ);
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
		verificarSenialesRecibidas();

		if(!strcmp(mapa->algoritmo, "RR")){
			entrenador = obtenerSiguienteEntrenadorPlanificadoRR(entrenador);
		}else{
			entrenador = obtenerSiguienteEntrenadorPlanificadoSRDF(entrenador);
		}

		procesarEntrenadoresGarbageCollector();
		procesarEntrenadoresBloqueados();
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
