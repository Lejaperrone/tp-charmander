/*
 /*
 * structures.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

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
		char id;
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
	}t_planificador_entrenador;

	typedef struct{
		char simbolo;
		int socket;
		t_coordenadas ubicacion;
		t_coordenadas ubicacionObjetivo;
		t_list* pokemons;
		t_pokemon_custom* ultimoPokeSolicitado;
		t_pokenest* pokenestBloqueante;
		t_planificador_entrenador planificador;
	}t_entrenador;

	t_mapa* mapa;
	char* name;
	char* pokedexPath;
	t_log* archivoLog;

	pthread_t hiloPlanificador;
	pthread_t hiloDeadlock;

	t_list* entrenadoresPreparados;
	t_list* entrenadoresListos;
	t_list* entrenadoresBloqueados;
	t_list* elementosUI;
	t_list* garbageCollectorEntrenadores;
	t_list* listaDeIdentificadoresDePokenests;

	int rows, cols;

	typedef struct pokemonsDisponibles{
		char idPokenest;
		int cantidad;
	}t_pokemonsDisponibles;

	typedef struct pokemonsCapturados{
		char idPokemon;
		int cantidad;
	}t_pokemon_transac;

	typedef struct pokemonsAsignados{
	char idEntrenador;
	t_pokemon_transac pokemonsCapturados;
	}t_poke_asignados;

	typedef struct pokemonsSolicitados{
		char idEntrenador;
		t_pokemon_transac pokemonsSolicitados;
	}t_poke_solicitados;

#endif /* COMMONS_STRUCTURES_H_ */
