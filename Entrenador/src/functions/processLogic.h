/*
 * processLogic.h
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include "../commons/structures.c"

#ifndef FUNCTIONS_PROCESSLOGIC_H_
#define FUNCTIONS_PROCESSLOGIC_H_

	void copiarPokemonFile(char* path);
	void copiarMedalla(t_mapa* mapa);
	int procesarMapa(t_mapa* mapa);
	int procesarObjetivo(t_mapa* mapa, t_objetivo* objetivo, int* movimiento, int serverMapa);

#endif /* FUNCTIONS_PROCESSLOGIC_H_ */
