/*
 * tabla_asignaciones.h
 *
 *  Created on: 29/10/2016
 *      Author: utnso
 */

#ifndef OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_
#define OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_

#include <stdbool.h>
#define true 1
#define false 0

int osada_TG_avanzarNBloques(int bloque,int desplazamientoHastaElBloque);

void obtenerProximoBloque(int* subindice);
void modificarBloquesAsignadosATablaDeAsignaciones(t_list* bloques);
int existeProximoBloque(int* subindice,bool* hayMasBloques);

int avanzarBloquesParaEscribir (int bloqueInicial,int desplazamientoLimite);
void ocuparBloqueSegunElUltimo(int ultimoBloque,int bloqueAOcupar);
#endif /* OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_ */
