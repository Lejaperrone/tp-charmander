/*
 * basura.c
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

void bloquesATruncar(int subindice, off_t offset, int* nuevoUltimoBloque){
	int i;
	div_t bytes=div((int)offset,OSADA_BLOCK_SIZE);
	div_t bloques=div((int)osada_drive.directorio[subindice].file_size,OSADA_BLOCK_SIZE);
	int bloquesOriginales=bloques.quot;
	int b=osada_drive.directorio[subindice].first_block;
	for (i=1;i<bloquesOriginales;i++){
		log_info(logPokedexServer,"Bloque %d",b);
		b=osada_drive.asignaciones[b];
	}
	int bloquesQueNecesitoMoverme=bytes.quot;
	log_info(logPokedexServer,"Necesito moverme %d bloques para llegar al nuevo ultimo",bloquesQueNecesitoMoverme);
	*nuevoUltimoBloque=osada_drive.directorio[subindice].first_block;

	for(i=0;i<bloquesQueNecesitoMoverme;i++){
		*nuevoUltimoBloque=osada_drive.asignaciones[*nuevoUltimoBloque];
		log_info(logPokedexServer,"nuevoUltimoBloque vale ahora: %d",*nuevoUltimoBloque);
	}
	log_info(logPokedexServer, "El nuevo ultimo bloque es %d",*nuevoUltimoBloque);
}

int hayBloquesDesocupadosEnElBitmap (int* n, int* bloqueArranque){
	int bloqueReal = *bloqueArranque;
	int i=osada_drive.header->bitmap_blocks+1025+((osada_drive.header->fs_blocks-1025-osada_drive.header->bitmap_blocks)*4/OSADA_BLOCK_SIZE);
	int bloquesNecesarios=0;

	while(bloquesNecesarios<*n && i<=bitarray_get_max_bit(osada_drive.bitmap)){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			bitarray_set_bit(osada_drive.bitmap,i);
			log_info(logPokedexServer, "OSADA - El bloque arranque en hayBloquesDesocupadosEnElBitmap es: %d", *bloqueArranque);
			if(*bloqueArranque != 0xFFFF){
				osada_drive.asignaciones[bloqueReal] = i;
				log_info(logPokedexServer, "OSADA - El proximo bloque es: %d", i);
			}else{
				*bloqueArranque = i;
				log_info(logPokedexServer, "OSADA - El bloque arranque en hayBloquesDesocupadosEnElBitmap es: %d", *bloqueArranque);
			}

			bloqueReal = i;
			osada_drive.asignaciones[i] = 0xFFFF;
			bloquesNecesarios++;

		}
		i++;
	}
	if(bloquesNecesarios==*n){
		return 1;
	}else{
		return 0;
	}
}

int buscarLugarLibreEnTablaArchivos(){
	int i;
	for (i=0;i<2048;i++){
		if(osada_drive.directorio[i].state!=DIRECTORY && osada_drive.directorio[i].state!=REGULAR){
			return i;
		}
	}
	return -ENOMEM;
}

void directoryContainingFile(char* path, char** fileName, char** father){
	char** vectorPath=string_split(path,"/");
	if (vectorPath[1]==NULL){
		string_append(father,"/");
		string_append(fileName,vectorPath[0]);
	}else{
		int i=0;
		while(vectorPath[i]!=NULL){
			if(vectorPath[i+1]==NULL){
				string_append(fileName,vectorPath[i]);
			}
			string_append(father,vectorPath[i]);
			string_append(father,"/");
		}
	}
	log_info(logPokedexServer,"OSADA - Directory Containing File: El nombre del archivo es %s y su path es %s",*fileName,*father);
}

void actualizarBytesEscritos (int* acum, int bytes){
	*acum += bytes;
}

void actualizarTablaDeArchivosParaWrite(char* path, size_t size, int indice){
	time_t timer=time(0);

	osada_drive.directorio[indice].file_size = size;
	log_info(logPokedexServer, "OSADA - El file_size del path %s es %d, y el size que me llego por parametro es %d",path, osada_drive.directorio[indice].file_size, size);

	osada_drive.directorio[indice].lastmod = timer;
}

void buscarLugarLibreEnBitmap(int* lugarLibre){
	int bloqueInicialDeDatos=osada_drive.header->bitmap_blocks+1025+(osada_drive.header->fs_blocks-osada_drive.header->bitmap_blocks-1025)*4/OSADA_BLOCK_SIZE;
	int i;
	log_info(logPokedexServer, "OSADA - El tamanio del bitmap de osada_drive es: %d", bitarray_get_max_bit(osada_drive.bitmap));

	for (i=bloqueInicialDeDatos;i<=bitarray_get_max_bit(osada_drive.bitmap);i++){
		if (bitarray_test_bit(osada_drive.bitmap,i)){
		}else{
			*lugarLibre=i;
			i=bitarray_get_max_bit(osada_drive.bitmap);

			log_info(logPokedexServer,"Encontre lugar libre en %d",*lugarLibre);
		}
	}
}

int obtenerLongitudDelNombreDelArchivo(char* path){
	char* fileName=string_new();
	char* directoryName=string_new();
	directoryContainingFile(path,&fileName,&directoryName);
	if (string_length(fileName)>17){
		return -E2BIG;
	}else{
		return 1;
	}
}

void generarNuevoArchivoEnTablaDeArchivos(char* path, int posicionEnTablaArchivos){
	time_t timer=time(0);
	char* fileName=string_new();
	char* directoryName=string_new();
	directoryContainingFile(path,&fileName, &directoryName);
	log_info(logPokedexServer, "OSADA - Generacion nuevo archivo: El nombre del archivo es: %s\n",fileName);
	osada_drive.directorio[posicionEnTablaArchivos].file_size=0;
	log_info(logPokedexServer,"El archivo %s de la path %s ocupa %d bytes",fileName,directoryName,osada_drive.directorio[posicionEnTablaArchivos].file_size);
	osada_drive.directorio[posicionEnTablaArchivos].first_block=0xFFFF;
	log_info(logPokedexServer,"El nombre inicial es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	char* barrasCero=string_repeat('\0',17);
	memcpy(osada_drive.directorio[posicionEnTablaArchivos].fname,barrasCero,17);
	log_info(logPokedexServer,"El nombre inicial despues del memcpy es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].parent_directory=osada_TA_obtenerIndiceTA(directoryName);
	log_info(logPokedexServer,"El directorio padre de %s es %d",fileName,osada_drive.directorio[posicionEnTablaArchivos].parent_directory);
	int i;
	for (i=0;i<17;i++){
		osada_drive.directorio[posicionEnTablaArchivos].fname[i]=fileName[i];
	}
	log_info(logPokedexServer,"El nombre final es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].state=REGULAR;
	osada_drive.directorio[posicionEnTablaArchivos].lastmod=timer;
	free(fileName);
	free(directoryName);
}

bool esUnArchivo(int subindice){
	return osada_drive.directorio[subindice].state==1;
}

bool estaBorrado(int subindice){
	return osada_drive.directorio[subindice].state==0;
}

bool esUnDirectorio(int subindice){
	return osada_drive.directorio[subindice].state==2;
}

int renombrarArchivo (int subindice, char* newFileName, int subindicePath){
	int resultado;
	if (esUnArchivo(subindice)){
		if (strlen(strcpy((char*)osada_drive.directorio[subindice].fname,newFileName))==strlen(newFileName)){
			osada_drive.directorio[subindice].parent_directory=subindicePath;
			log_info(logPokedexServer,"Se ha reemplazado el nombre del archivo por %s",newFileName);
			resultado= 1;
		}else{
			resultado= 0;
		}
	}else{
		if (estaBorrado(subindice)){
			resultado=ENOENT;
		}
		if (esUnDirectorio(subindice)){
			if (strlen(strcpy((char*)osada_drive.directorio[subindice].fname,newFileName))==strlen(newFileName)){
				osada_drive.directorio[subindice].parent_directory=subindicePath;
				log_info(logPokedexServer,"Se ha reemplazado el nombre del directorio por %s",newFileName);
						resultado= 1;
					}else{
						resultado= 0;
					}
		}
	}
	return resultado;

}

int getFileNameFromPath(char* path,  char** nombre){
	char** pathSplitteada=string_split(path,"/");
	int j=0;
	char* pathNueva=string_new();
	while(j!=-1){
		if(pathSplitteada[j+1]==NULL){
			string_append(nombre,pathSplitteada[j]);
			j=-1;
		}else{
			string_append(&pathNueva,"/");
			string_append(&pathNueva,pathSplitteada[j]);
			j++;
		}
	}
	log_info(logPokedexServer,"La path nueva es: %s",pathNueva);
	int subindicePath=osada_TA_obtenerIndiceTA(pathNueva);
	log_info(logPokedexServer,"El nombre a modificar es %s",*nombre );
	log_info(logPokedexServer,"La path nueva es %s, el nombre a actualizar es %s",path,*nombre);
	int i=0;
	while(i!=-1){
		free(pathSplitteada[i]);
		if (pathSplitteada[i]==NULL){
			i=-1;
		}else{
			i++;
		}
	}
	free(pathNueva);
	free(pathSplitteada);
	return subindicePath;
}

void actualizarTablaDeArchivos(int subindice, off_t offset){

	log_info(logPokedexServer, "OSADA - Antes del strftime");
	osada_drive.directorio[subindice].file_size = offset;
	log_info(logPokedexServer, "OSADA - El nuevo size del archivo es: %d",osada_drive.directorio[subindice].file_size);

}

void limpiarBitmapParaTruncate(int bloque){
	log_info(logPokedexServer,"Bloque vale %d",bloque);
	while(bloque!=0xFFFF && bloque!=-1){
			 bitarray_clean_bit(osada_drive.bitmap,osada_drive.asignaciones[bloque]);
			 log_info(logPokedexServer,"El bloque %d vale %d",bloque, bitarray_test_bit(osada_drive.bitmap,bloque));
			 bloque=osada_drive.asignaciones[bloque];
			 log_info(logPokedexServer,"Bloque ahora vale %d",bloque);
		}
}

void asignarFFFFaNuevoUltimoBloque(int nuevoUltimoBloque){
	osada_drive.asignaciones[nuevoUltimoBloque]=0xFFFF;
}

void limpiarDatos(int nuevoUltimoBloque,int bytesConBarraCeroDelUltimoBloque, int bytesOcupados){
	char* barrasCero=string_repeat('\0',bytesConBarraCeroDelUltimoBloque);
	memcpy(osada_drive.data[nuevoUltimoBloque]+bytesOcupados,barrasCero,bytesConBarraCeroDelUltimoBloque);
	log_info(logPokedexServer,"OSADA - Limpieza de datos: Se han limpiado %d bytes del bloque %d",bytesConBarraCeroDelUltimoBloque,nuevoUltimoBloque);
	nuevoUltimoBloque=osada_drive.asignaciones[nuevoUltimoBloque];
	while (nuevoUltimoBloque!=0xFFFF && nuevoUltimoBloque!=-1){
		barrasCero=string_repeat('\0',OSADA_BLOCK_SIZE);
		memcpy(osada_drive.data[nuevoUltimoBloque],barrasCero,OSADA_BLOCK_SIZE);
		log_info(logPokedexServer,"OSADA - Limpieza de datos: Se han limpiado %d bytes del bloque %d",OSADA_BLOCK_SIZE,nuevoUltimoBloque);
		nuevoUltimoBloque=osada_drive.asignaciones[nuevoUltimoBloque];
		log_info(logPokedexServer,"El proximo bloque es: %d",nuevoUltimoBloque);
	}
}

void liberarEspacio (int subindice, off_t offset, int nuevoUltimoBloque){
	int bytesOcupados=(int)offset%(int)OSADA_BLOCK_SIZE;
	log_info(logPokedexServer,"bytesOcupados vale: %d",bytesOcupados);
	int bytesConBarraCeroDelUltimoBloque=OSADA_BLOCK_SIZE-bytesOcupados;
	log_info(logPokedexServer,"bytesConBarraCeroDelUltimoBloque vale: %d",bytesConBarraCeroDelUltimoBloque);
	limpiarDatos(nuevoUltimoBloque,bytesConBarraCeroDelUltimoBloque, bytesOcupados);
	limpiarBitmapParaTruncate(nuevoUltimoBloque);
	asignarFFFFaNuevoUltimoBloque(nuevoUltimoBloque);
	log_info(logPokedexServer,"Del ultimo bloque tengo que llenar con %d barras cero porque hay ocupados %d bytes",
			bytesConBarraCeroDelUltimoBloque, bytesOcupados);
	actualizarTablaDeArchivos(subindice, offset);



}

int irAlUltimoBloqueDeLaTablaDeAsignaciones (int subindice){
	int indice=osada_drive.directorio[subindice].first_block;
	log_info(logPokedexServer,"El primer bloque es %d", indice);
	int ult=indice;
	while (indice!=0xFFFFFF && indice != -1){
		log_info(logPokedexServer,"El bloque es %d",ult);
		ult=indice;
		obtenerProximoBloque(&indice);
	}
	return ult;
}

void marcarComoOcupadosEnElBitmap (t_list* lista){
	int i;
	for (i=0;i<list_size(lista);i++){
		bitarray_set_bit(osada_drive.bitmap,(int)list_get(lista,i));
		log_info(logPokedexServer,"OSADA - Marco como %d a %d",bitarray_test_bit(osada_drive.bitmap,(int)list_get(lista,i)),(int)list_get(lista,i));
	}
}

void actualizarTablaDeAsignaciones(int ultimoBloqueOriginal, t_list* listaDeBloquesNuevos){
	int i;
	for (i=0;i<list_size(listaDeBloquesNuevos);i++){
		log_info(logPokedexServer,"A %d le asigno como proximo bloque %d",ultimoBloqueOriginal,(int)list_get(listaDeBloquesNuevos,i));
		osada_drive.asignaciones[ultimoBloqueOriginal]=(int)list_get(listaDeBloquesNuevos,i);
		ultimoBloqueOriginal=(int)list_get(listaDeBloquesNuevos,i);
		log_info(logPokedexServer,"El nuevo ultimo bloque es %d", (int)list_get(listaDeBloquesNuevos,i));
	}
}

void actualizarDatos (int ultimoBloque, t_list* lista, int bytesOcupadosDelUltimoBloque, int bytesConBarraCeroDelUltimoBloque){

	log_info(logPokedexServer,"El ultimo bloque es %d",ultimoBloque);
	char* barrasCero=string_repeat('\0',bytesConBarraCeroDelUltimoBloque);
	memcpy(osada_drive.data[ultimoBloque]+bytesOcupadosDelUltimoBloque,barrasCero,bytesConBarraCeroDelUltimoBloque);
	ultimoBloque=osada_drive.asignaciones[ultimoBloque];
	while(ultimoBloque!=0xFFFF && ultimoBloque!=-1){
		barrasCero=string_repeat('\0',OSADA_BLOCK_SIZE);
		memcpy(osada_drive.data[ultimoBloque]+OSADA_BLOCK_SIZE,barrasCero,OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "Lleno el bloque %d con",ultimoBloque);
		ultimoBloque=osada_drive.asignaciones[ultimoBloque];
	}
}

void ocuparEspacio (int sub, int bq_to_set, off_t offset, t_list* lista){
	int ultimoBloque=irAlUltimoBloqueDeLaTablaDeAsignaciones(sub);
	div_t calculoDeBytesOcupados=div(osada_drive.directorio[sub].file_size,OSADA_BLOCK_SIZE);
	int bytesOcupadosDelUltimoBloque=calculoDeBytesOcupados.rem;
	log_info(logPokedexServer,"Los bytes ocupados del ultimo bloque son: %d",bytesOcupadosDelUltimoBloque);
	int bytesConBarraCerosDelUltimoBloque=OSADA_BLOCK_SIZE-bytesOcupadosDelUltimoBloque;
	log_info(logPokedexServer, "OSADA - El ultimoBloque original es: %d", ultimoBloque);
	log_info(logPokedexServer,"Los bytes con barra cero del ultimo bloque son: %d",bytesConBarraCerosDelUltimoBloque);
	marcarComoOcupadosEnElBitmap(lista);
	actualizarTablaDeArchivos(sub,offset);
	actualizarTablaDeAsignaciones(ultimoBloque,lista);
	actualizarDatos(ultimoBloque,lista,bytesOcupadosDelUltimoBloque,bytesConBarraCerosDelUltimoBloque);


}

void contarBloquesSegun(int originalFileSize,int offset, int* bloques){

	div_t bloquesAux=div(offset-originalFileSize,OSADA_BLOCK_SIZE);
	*bloques=bloquesAux.quot;
	if(bloquesAux.rem>0){
		*bloques=*bloques+1;
	}
	log_info(logPokedexServer, "Necesito ocupar %d mas",*bloques);
}

float calcularEspacioDisponibleEnDisco(){
	int i, bloquesVacios=0;
	for (i=0;i<bitarray_get_max_bit(osada_drive.bitmap);i++){
		if(!bitarray_test_bit(osada_drive.bitmap,i)){
			bloquesVacios++;
		}
	}
	log_info(logPokedexServer,"Hay %f bytes vacios",((((float)osada_drive.header->data_blocks)*OSADA_BLOCK_SIZE)/((float)bloquesVacios)*OSADA_BLOCK_SIZE));
	return ((((float)osada_drive.header->data_blocks)*OSADA_BLOCK_SIZE)/((float)bloquesVacios)*OSADA_BLOCK_SIZE);
}

int contarBloquesLibresTotales(){
	int t=bitarray_get_max_bit(osada_drive.bitmap);
	int i,tot=0;
	for (i=0;i<t;i++){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			tot++;
		}
	}
	return tot;
}

int contarOsadaFilesLibres(){
	int i, tot=0;
	for (i=0;i<2048;i++){
		if(estaBorrado(i)){
			tot++;
		}
	}
	return tot;
}
