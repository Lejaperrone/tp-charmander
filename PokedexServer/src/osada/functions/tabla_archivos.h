/*
 * tabla_archivos.h
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#ifndef OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_
#define OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_

int osada_TA_compareNameToIndex(int indice, char* test2);
int osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent);
void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio);
void osada_TA_splitPathAndName(char* path, char** name, char** pathFrom);
int osada_TA_createNewDirectory(char* path, osada_file_state state);
int osada_TA_cantRegistrosLibres();
bool osada_TA_TArchivo(int subindice);
bool osada_TA_TBorrado(int subindice);
bool osada_TA_TDirectorio(int subindice);
u_int16_t osada_TA_obtenerUltimoHijoFromPath(char* path, int* resultadoDeBuscarRegistroPorNombre);
void osada_TA_obtenerAttr(u_int16_t indice, file_attr* attr);
void osada_TA_deleteDirectory(u_int16_t indice, osada_file_state state);
void osada_TA_renombrarArchivo(char* nombre, u_int16_t parent,char* nuevoNombre);
int osada_TA_obtenerIndiceTA(char* path);
int osada_TA_obtenerDirectorioLibre();

#endif /* OSADA_FUNCTIONS_TABLA_ARCHIVOS_H_ */
