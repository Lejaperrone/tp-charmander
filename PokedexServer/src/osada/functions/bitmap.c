/*
 * bitmap.c
 *
 *  Created on: 10/12/2016
 *      Author: utnso
 */

#include <sys/statvfs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <commons/collections/list.h>
#include "../commons/declarations.h"
#include "../../commons/definitions.h"
#include "../../commons/structures.h"
#include "tabla_archivos.h"
#include "../osada.h"
#include "../mapp.h"
#include <commons/bitarray.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <math.h>
#include <commons/log.h>
#include "tabla_asignaciones.h"
#include <pthread.h>

void osada_B_findFreeBlock(int* lugarLibre){
	int bloquesAsignaciones = (osada_drive.header->fs_blocks - osada_drive.header->bitmap_blocks - 1024 -1) * 4 / OSADA_BLOCK_SIZE;
	int bloqueInicialDeDatos=osada_drive.header->bitmap_blocks + 1024 + 1 + bloquesAsignaciones;

	int i;
	for (i=bloqueInicialDeDatos;i<=bitarray_get_max_bit(osada_drive.bitmap);i++){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			*lugarLibre=i;
			return;
		}
	}
}

int osada_B_cantBloquesLibres(){
	int t=bitarray_get_max_bit(osada_drive.bitmap);
	int i,tot=0;
	for (i=0;i<t;i++){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			tot++;
		}
	}
	return tot;
}
