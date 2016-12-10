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
#include <commons/log.h>
#include "../commons/declarations.h"
#include "../../commons/definitions.h"
#include "../commons/osada.h"
#include <errno.h>

int osada_TG_avanzarNBloques (int bloqueInicial,int desplazamientoLimite){
	int i;

	for (i=0;i<desplazamientoLimite;i++){
		if(bloqueInicial != 0xFFFF && bloqueInicial != -1){
			bloqueInicial=osada_drive.asignaciones[bloqueInicial];
		}else{
			bloqueInicial = -1;
		}
	}
	return bloqueInicial;
}


int avanzarBloquesParaEscribir (int bloqueInicial,int desplazamientoLimite){
	int i;
	for (i=0;i<desplazamientoLimite;i++){
		bloqueInicial=osada_drive.asignaciones[bloqueInicial];
	}
	return bloqueInicial;
}



bool existeProximoBloque(int* subindice){
	return (osada_drive.asignaciones[*subindice] != 0xFFFF);
}

void obtenerProximoBloque(int* subindice){
	if (existeProximoBloque(subindice) && *subindice != -1){
		*subindice=osada_drive.asignaciones[*subindice];
	}else{
		*subindice=0xFFFF;
	}
}

void ocuparBloqueSegunElUltimo(ultimoBloque,bloqueAOcupar){
	osada_drive.asignaciones[ultimoBloque]=bloqueAOcupar;
}
void modificarBloquesAsignadosATablaDeAsignaciones(t_list* bloques){
	int i;
	for (i=0;i<list_size(bloques);i++){
		if (i==list_size(bloques)){
			osada_drive.asignaciones[i-1]=0xFFFFFFFF;
		}else{
			osada_drive.asignaciones[(int)list_get(bloques,i)]=(int)list_get(bloques,i+1);
		}

	}
}




