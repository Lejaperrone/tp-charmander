/*
 * osada.c
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <commons/collections/list.h>
#include "commons/declarations.h"
#include "functions/tabla_archivos.h"

int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	//Obtengo los directorios
		osada_TA_obtenerDirectorios(parent, directorios);
	//Return
		return 0;
}
