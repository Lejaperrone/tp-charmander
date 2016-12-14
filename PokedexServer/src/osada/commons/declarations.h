/*
 * declarations.c
 *
 *  Created on: 17/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <commons/bitarray.h>
#include <pthread.h>

#ifndef OSADA_COMMONS_DECLARATIONS_C_
#define OSADA_COMMONS_DECLARATIONS_C_

	typedef struct {
		int state;
		uint32_t file_size;
		time_t lastmod;
	} file_attr;

	typedef struct{
		osada_header* header;
		t_bitarray * bitmap;
		osada_file * directorio;
		osada_block_pointer* asignaciones;
		osada_block * data;
	}t_osada_drive;

	typedef struct{
		pthread_mutex_t directorio[2048];
		pthread_mutex_t * block;
		int countBlocks;
	}t_osada_mutex;

	t_osada_drive osada_drive;
	t_osada_mutex osada_mutex;

#endif /* OSADA_COMMONS_DECLARATIONS_C_ */
