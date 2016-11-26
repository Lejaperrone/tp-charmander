/*
 * definitions.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#ifndef COMMONS_DEFINITIONS_H_
#define COMMONS_DEFINITIONS_H_

t_log* logPokedexServer;

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
pthread_attr_t attr;
pthread_t thread;
pthread_mutex_t mutexTablaArchivos;
pthread_mutex_t mutexTablaAsignaciones;
pthread_mutex_t mutexBitmap;
pthread_mutex_t mutexDatos;
char *PORT;

#endif /* COMMONS_DEFINITIONS_H_ */
