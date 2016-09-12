/*
 * structures.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#include <commons/collections/list.h>

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
	}t_pokemon;

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
		char* nombre;
		char* simbolo;
		int vidas;
		int reintentos;
		//Falta hoja de viaje y los objetivos
	}t_entrenador;

#endif /* COMMONS_STRUCTURES_H_ */
