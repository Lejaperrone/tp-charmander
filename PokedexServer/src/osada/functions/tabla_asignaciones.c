/*
 * tabla_asignaciones.c
 *
 *  Created on: 22/10/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "../commons/declarations.h"
#include "../commons/osada.h"
#include <errno.h>

void actualizarTablaDeAsignaciones(int* subindice, bool* hayMasBloques){
	if (osada_drive.asignaciones[*subindice]>=0){
		*subindice=osada_drive.asignaciones[*subindice];
	}else{
		osada_drive.asignaciones[*subindice]=-1;
		*hayMasBloques=false;
	}

}




