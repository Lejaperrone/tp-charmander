/*
 * basura.h
 *
 *  Created on: 10/12/2016
 *      Author: utnso
 */

#include <sys/statvfs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <commons/collections/list.h>
#include "commons/declarations.h"
#include "../commons/definitions.h"
#include "../commons/structures.h"
#include "functions/tabla_archivos.h"
#include "commons/osada.h"
#include "osada.h"
#include "mapp.h"
#include <commons/bitarray.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <math.h>
#include <../commons/log.h>
#include "functions/tabla_asignaciones.h"
#include <pthread.h>

#ifndef OSADA_BASURA_H_
#define OSADA_BASURA_H_

void bloquesATruncar(int subindice, off_t offset, int* nuevoUltimoBloque);
int hayBloquesDesocupadosEnElBitmap (int* n, int* bloqueArranque);
void actualizarBytesEscritos (int* acum, int bytes);
void actualizarTablaDeArchivosParaWrite(char* path, size_t size, int indice);
int obtenerLongitudDelNombreDelArchivo(char* path);
int renombrarArchivo (int subindice, char* newFileName, int subindicePath);
int getFileNameFromPath(char* path,  char** nombre);
void actualizarTablaDeArchivos(int subindice, off_t offset);
void limpiarBitmapParaTruncate(int bloque);
void asignarFFFFaNuevoUltimoBloque(int nuevoUltimoBloque);
void limpiarDatos(int nuevoUltimoBloque,int bytesConBarraCeroDelUltimoBloque, int bytesOcupados);
void liberarEspacio (int subindice, off_t offset, int nuevoUltimoBloque);
int irAlUltimoBloqueDeLaTablaDeAsignaciones (int subindice);
void marcarComoOcupadosEnElBitmap (t_list* lista);
void actualizarTablaDeAsignaciones(int ultimoBloqueOriginal, t_list* listaDeBloquesNuevos);
void actualizarDatos (int ultimoBloque, t_list* lista, int bytesOcupadosDelUltimoBloque, int bytesConBarraCeroDelUltimoBloque);
void ocuparEspacio (int sub, int bq_to_set, off_t offset, t_list* lista);
void contarBloquesSegun(int originalFileSize,int offset, int* bloques);
float calcularEspacioDisponibleEnDisco();


#endif /* OSADA_BASURA_H_ */
