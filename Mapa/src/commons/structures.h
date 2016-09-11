/*
 * structures.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#ifndef COMMONS_STRUCTURES_H_
#define COMMONS_STRUCTURES_H_

	typedef struct{
		char* nombre;
		int tiempoChequeoDeadlock;
		int batalla;
		char* algoritmo;
		int quantum;
		int retardo;
		char* ip;
		char* puerto;
	}t_mapa;

	typedef struct{
		char* nombre;
		char* simbolo;
		int vidas;
		int reintentos;
		//Falta hoja de viaje y los objetivos
	}t_entrenador;

#endif /* COMMONS_STRUCTURES_H_ */
