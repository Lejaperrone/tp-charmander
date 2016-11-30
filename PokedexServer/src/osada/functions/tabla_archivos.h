/*
 * tabla_archivos.h
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#ifndef OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_
#define OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_

int osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent);
void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio);
u_int16_t osada_TA_obtenerUltimoHijoFromPath(char* path, int* resultadoDeBuscarRegistroPorNombre);
void osada_TA_obtenerAttr(u_int16_t indice, file_attr* attr);
void osada_TA_borrarArchivo( u_int16_t parent);
void osada_TA_borrarDirectorio(u_int16_t parent);
void osada_TA_renombrarArchivo(char* nombre, u_int16_t parent,char* nuevoNombre);
void darDeAltaDirectorioEnTablaDeArchivos(char* nombre,int indice);

#endif /* OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_ */
