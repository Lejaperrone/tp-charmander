/*
 * processLogic.h
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include "../commons/structures.c"

#ifndef FUNCTIONS_PROCESSLOGIC_H_
#define FUNCTIONS_PROCESSLOGIC_H_

	void procesarMapa(t_mapa* mapa);
	void procesarObjetivo(t_mapa* mapa, t_objetivo* objetivo, int* movimiento, int serverMapa);
	t_log* archivoLog;

#endif /* FUNCTIONS_PROCESSLOGIC_H_ */
