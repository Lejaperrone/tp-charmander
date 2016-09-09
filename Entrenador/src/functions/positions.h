/*
 * positions.h
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../commons/structures.c"

#ifndef FUNCTIONS_POSITIONS_H_
#define FUNCTIONS_POSITIONS_H_

	int coordenadasCoinciden(t_coordenadas coordenadas, t_coordenadas coordenadas2);
	int siguienteMovimiento(t_coordenadas coordenadas, t_objetivo* objetivo, int ultimoMovimiento);

#endif /* FUNCTIONS_POSITIONS_H_ */
