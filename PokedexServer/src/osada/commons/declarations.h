/*
 * declarations.c
 *
 *  Created on: 17/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <commons/bitarray.h>

#ifndef OSADA_COMMONS_DECLARATIONS_C_
#define OSADA_COMMONS_DECLARATIONS_C_

	typedef struct {
		int state;
		uint32_t file_size;
	} file_attr;

	typedef struct{
		osada_header* header;
		t_bitarray * bitmap;
		osada_file * directorio;
		osada_block_pointer* asignaciones;
		osada_block * data;
	}t_osada_drive;

	t_osada_drive osada_drive;

#endif /* OSADA_COMMONS_DECLARATIONS_C_ */
