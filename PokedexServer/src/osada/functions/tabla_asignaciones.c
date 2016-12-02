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

extern pthread_mutex_t mutexTablaAsignaciones;
int avanzarBloquesParaLeer (int bloqueInicial,int desplazamientoLimite){
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
	int bloqueInicioEscritura;
	for (i=0;i<desplazamientoLimite;i++){
		bloqueInicioEscritura=osada_drive.asignaciones[bloqueInicial];
		bloqueInicial=bloqueInicioEscritura;
	}
	return bloqueInicioEscritura;
}



bool existeProximoBloque(int* subindice){
	return (osada_drive.asignaciones[*subindice] != 0xFFFF);
}

void obtenerProximoBloque(int* subindice){
	if (existeProximoBloque(subindice)){
		log_info(logPokedexServer, "El subindice %d se reemplazara por %d", *subindice,osada_drive.asignaciones[*subindice]);
		*subindice=osada_drive.asignaciones[*subindice];
		log_info(logPokedexServer, "El nuevo subindice (dentro de ProximoBloque) es: %d",*subindice);
	}else{
		*subindice=0xFFFF;
		log_info(logPokedexServer, "El nuevo subindice (dentro de ProximoBloque) que deberia ser 65535 es: %d",*subindice);
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




