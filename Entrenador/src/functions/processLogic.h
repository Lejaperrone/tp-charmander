/*
 * processLogic.h
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include "../commons/structures.c"

#ifndef FUNCTIONS_PROCESSLOGIC_H_
#define FUNCTIONS_PROCESSLOGIC_H_

	int procesarMapa(t_mapa* mapa);
	int procesarObjetivo(t_mapa* mapa, t_objetivo* objetivo, int* movimiento, int serverMapa);
	int accionesSegunLasVidasDisponibles();
	void resetearObjetivos(t_objetivo* objetivo);
	t_log* archivoLog;

#endif /* FUNCTIONS_PROCESSLOGIC_H_ */
