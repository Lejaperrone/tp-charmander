/*
 * tabla_asignaciones.h
 *
 *  Created on: 29/10/2016
 *      Author: utnso
 */

#ifndef OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_
#define OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_




void actualizarTablaDeAsignaciones_porBaja(int* subindice, bool* hayMasBloques);
void modificarBloquesAsignadosATablaDeAsignaciones(t_list* bloques);
int existeProximoBloque(int* subindice,bool* hayMasBloques);
#endif /* OSADA_FUNCTIONS_TABLA_ASIGNACIONES_H_ */
