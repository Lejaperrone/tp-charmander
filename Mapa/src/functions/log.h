/*
 * log.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include "../commons/structures.h"

#ifndef FUNCTIONS_LOG_H_
#define FUNCTIONS_LOG_H_

	t_log* crearArchivoLog();
	void loguearConfiguracion(t_log* archivoLogs, t_mapa* mapa);

#endif /* FUNCTIONS_LOG_H_ */
