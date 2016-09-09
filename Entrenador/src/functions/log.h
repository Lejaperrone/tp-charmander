/*
 * log.h
 *
 *  Created on: 6/9/2016
 *      Author: utnso
 */


#ifndef FUNCTIONS_LOG_H_
#define FUNCTIONS_LOG_H_

t_log* crearArchivoLog(char* nombre);
void loguearConfiguracion(t_log* archivoLogs, t_entrenador* entrenador);

#endif /* FUNCTIONS_LOG_H_ */
