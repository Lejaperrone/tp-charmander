/*
 * constants.h
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include <commons/log.h>

#ifndef COMMONS_CONSTANTS_H_
#define COMMONS_CONSTANTS_H_

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

#define MOVERSE_ARRIBA 1 //Define el movimiento hacia arriba a la hora de ir a buscar la pokenest
#define MOVERSE_DERECHA 2 //Define el movimiento hacia la derecha a la hora de ir a buscar la pokenest
#define MOVERSE_ABAJO 3 //Define el movimiento hacia abajo a la hora de ir a buscar la pokenest
#define MOVERSE_IZQUIERDA 4 //Define el movimiento hacia la izquierda a la hora de ir a buscar la pokenest

t_log* archivoLog;
t_entrenador* entrenador;

#endif /* COMMONS_CONSTANTS_H_ */
