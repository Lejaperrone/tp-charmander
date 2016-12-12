/*
 * data.c
 *
 *  Created on: 11/12/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../commons/declarations.h"
#include "../../commons/definitions.h"
#include "../commons/osada.h"
#include <errno.h>

void osada_D_truncateBlock(int indice, int offset){
	int i;
	for(i=offset;i<OSADA_BLOCK_SIZE;i++){
		osada_drive.data[indice][i] = '\0';
	}
}
