#include <commons/collections/list.h>
#include <commons/log.h>

#ifndef COMMONS_STRUCTURES_H_
#define COMMONS_STRUCTURES_H_

	typedef struct{
		int x;
		int y;
	}t_coordenadas;

	typedef struct{
		char* path;
		int disponible;
		char duenio;
		int nivel;
		char identificadorPokenest;
		char* nombre;
	}t_pokemon_custom;

	typedef struct{
		char* tipo;
		char identificador;
		t_coordenadas ubicacion;
		t_list* pokemons;
	}t_pokenest;

	typedef struct{
		char* nombre;
		int tiempoChequeoDeadlock;
		int batalla;
		char* algoritmo;
		int quantum;
		int retardo;
		char* ip;
		char* puerto;
		t_list* pokeNests;
	}t_mapa;

	typedef struct{
		int quantum;
		t_coordenadas ubicacionObjetivo;
	}t_planificador_entrenador;

	typedef struct{
		char simbolo;
		int socket;
		t_coordenadas ubicacion;
		t_list* pokemons;
		t_pokenest* pokenestBloqueante;
		t_planificador_entrenador planificador;
	}t_entrenador;

	t_mapa* mapa;
	char* name;
	char* pokedexPath;
	t_log* archivoLog;

	pthread_t hiloPlanificador;
	pthread_t hiloDeadlock;

	pthread_mutex_t mutexEntrBQ;
	pthread_mutex_t mutexMapa;

	t_list* entrenadoresPreparados;
	t_list* entrenadoresListos;
	t_list* entrenadoresBloqueados;
	t_list* elementosUI;
	t_list* garbageCollectorEntrenadores;
	t_list* listaDeIdentificadoresDePokenests;

	int rows, cols;
	int recvSIGUSR2;
#endif /* COMMONS_STRUCTURES_H_ */
