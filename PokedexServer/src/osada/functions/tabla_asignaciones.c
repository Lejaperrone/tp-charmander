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

extern pthread_mutex_t mutexTablaAsignaciones;
int avanzarBloquesParaLeer (int bloqueInicial,int desplazamientoLimite){
	int i;
	int bloqueInicioLectura;
	for (i=0;i<desplazamientoLimite;i++){
		bloqueInicioLectura=osada_drive.asignaciones[bloqueInicial];
		bloqueInicial=bloqueInicioLectura;
	}
	return bloqueInicioLectura;
}
int avanzarBloquesParaEscribir (int bloqueInicial,int desplazamientoLimite){
	int i;
	int bloqueInicioEscritura;
	for (i=0;i<desplazamientoLimite;i++){
		bloqueInicioEscritura=osada_drive.asignaciones[bloqueInicial];
		bloqueInicial=bloqueInicioEscritura;
	}
	return bloqueInicioEscritura;
}



int existeProximoBloque(int* subindice){
	return (osada_drive.asignaciones[*subindice]>=0);
}

void obtenerProximoBloque(int* subindice){
	if (existeProximoBloque(subindice)){
		*subindice=osada_drive.asignaciones[*subindice];
	}else{
		*subindice=0xFFFFFFFF;
	}
}

void ocuparBloqueSegunElUltimo(ultimoBloque,bloqueAOcupar){
	osada_drive.asignaciones[ultimoBloque]=bloqueAOcupar;
}
void modificarBloquesAsignadosATablaDeAsignaciones(t_list* bloques){
	int i;
	for (i=0;i<list_size(bloques);i++){
		if (i==list_size(bloques)){
			//4294967295
			osada_drive.asignaciones[i-1]=0xFFFFFFFF;
		}else{
			osada_drive.asignaciones[(int)list_get(bloques,i)]=(int)list_get(bloques,i+1);
		}

	}
}




