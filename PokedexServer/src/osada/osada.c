/*
 * osada.c
 *
 *  Created on: 20/9/2016
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

extern pthread_mutex_t mutexTablaArchivos;
extern pthread_mutex_t mutexBitmap;



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


int osada_init(char* path){
	initOsada (path);
	return 1;
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

int osada_removeDir(char* path){

	t_list* directoriosQueComponenElActual=list_create();
	u_int16_t parent = osada_TA_obtenerIndiceTA(path);

	if(strcmp(path, "/") != 0){
		osada_TA_obtenerDirectorios(parent, directoriosQueComponenElActual);
		if (list_is_empty(directoriosQueComponenElActual)){
			osada_TA_borrarDirectorio(parent);
			log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: Se ha borrado el directorio %s de la tabla de archivos. El bloque borrado es %d\n",path,parent);
			list_destroy(directoriosQueComponenElActual);
			return 1;
		}else{
			perror("NO se pudo remover el directorio porque no esta vacio");
			list_destroy(directoriosQueComponenElActual);
			return -EBUSY;
		}
	}
	list_destroy(directoriosQueComponenElActual);
	perror("NO se pudo remover el directorio porque no existe");
	return -ENOENT;
}

int osada_removeFile(char* path){

	u_int16_t parent;
	if (strcmp(path, "/") != 0){
		log_info(logPokedexServer, "OSADA - El path es != de /, se va a obtenerIndiceTA");
		parent = osada_TA_obtenerIndiceTA(path);
		log_info(logPokedexServer, "El indice devuelto (parent) es: %d", parent);
	}else{
		parent = 0xFFFF;
	}

	if(parent >= 0 || parent == 0xFFFF){
		log_info(logPokedexServer, "OSADA - Se procede a borrar el archivo del bloque %d", parent);
		osada_TA_borrarArchivo(parent);
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: Pude borrar el archivo %s. Ocupaba el bloque %d\n", path, parent);
		return 1;
	}

	perror("NO se pudo remover el directorio porque no existe");
	return -ENOENT;

}

int osada_readdir(char* path, t_list* directorios){
	u_int16_t parent;
	if(strcmp(path,"/") != 0){
		parent = osada_TA_obtenerIndiceTA(path);
	}else{
		parent =0xFFFF;
	}

	if(parent>=0 || parent == 0xFFFF){
		osada_TA_obtenerDirectorios(parent, directorios);
		return 1;
	}


	return 0;
}


int osada_getattr(char* path, file_attr* attrs){
	if(strcmp(path,"/") == 0){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		int indice = osada_TA_obtenerIndiceTA(path);
		if(indice>=0){
			osada_TA_obtenerAttr(indice, attrs);
			return 1;
		}
		return 0;
	}
}

bool superaTamanioArchivo (int indice, off_t offset, size_t size){
	return size>(osada_drive.directorio[indice].file_size-offset);
}
bool elBufferTieneDatosParaEscribir(char* buf){
	return strlen(buf)>0;
}
void actualizarBuffer(char* buffer, int bytesEscritos){
	log_info(logPokedexServer,"El buffer tenia antes: %s", buffer);
	buffer=string_substring(buffer,bytesEscritos,string_length(buffer)-bytesEscritos);
	log_info(logPokedexServer,"El buffer tiene ahora: %s", buffer);
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

int osada_write(char* path,char** buf, size_t size, off_t offset){
	log_info(logPokedexServer,"El buf que me llega es: %s",*buf);
	int indice = osada_TA_obtenerIndiceTA(path);
	int bytesEscritos=0;
	log_info(logPokedexServer,"El path es %s y el indice es %d",path,indice);
	log_info(logPokedexServer, "El offset es %d", offset);
	if(indice != -1){
			int bloque=osada_drive.directorio[indice].first_block;
			log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n",path, bloque);
			double desplazamientoHastaElBloque=floor(offset/OSADA_BLOCK_SIZE);
			int bloqueArranque=avanzarBloquesParaEscribir(bloque,desplazamientoHastaElBloque);
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Desde el bloque %d me desplace hasta el %d, me movi %f bloques.\n",bloque,bloqueArranque,desplazamientoHastaElBloque);;
			int byteComienzoEscritura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
			log_info(logPokedexServer, "OSADA - DATOS: Empiezo a leer desde el byte: %d\n",byteComienzoEscritura);
			int bloquesQueNecesitoEscribir=ceil((float)size/(float)OSADA_BLOCK_SIZE);
			int sizeAux=(int)size;
			log_info(logPokedexServer,"Size aux vale %d y size comienzo vale %d",sizeAux,size);

			int ultimoBloqueIndice = 0xFFFF;
			if(bloqueArranque == 0xFFFF && osada_drive.directorio[indice].first_block != 0xFFFF){
				ultimoBloqueIndice = osada_drive.directorio[indice].first_block;
				while(osada_drive.asignaciones[ultimoBloqueIndice] != 0xFFFF){
					ultimoBloqueIndice = osada_drive.asignaciones[ultimoBloqueIndice];
				}
			}

			if (hayBloquesDesocupadosEnElBitmap(&bloquesQueNecesitoEscribir, &bloqueArranque) && byteComienzoEscritura>=0){
				if(ultimoBloqueIndice == 0xFFFF && osada_drive.directorio[indice].first_block != 0xFFFF){
					osada_drive.asignaciones[ultimoBloqueIndice]=bloqueArranque;
				}else if(offset==0){
					log_info(logPokedexServer,"Bloque Arranque es: %d",bloqueArranque);
					osada_drive.directorio[indice].first_block=bloqueArranque;
					log_info(logPokedexServer,"El first block del indice %d es: %d",indice, osada_drive.directorio[indice].first_block);
				}
				actualizarTablaDeArchivosParaWrite(path, size+offset, indice);
				while (sizeAux>0 && bloqueArranque != 0xFFFF){
					log_info(logPokedexServer,"Size aux vale %d",sizeAux);

					log_info(logPokedexServer, "El contenido del buffer es %s\n",*buf);
					log_info(logPokedexServer, "OSADA - BITMAP: Marco al bloque %d como %d\n",bloqueArranque, bitarray_test_bit(osada_drive.bitmap,bloqueArranque));

					if(sizeAux >= (OSADA_BLOCK_SIZE - byteComienzoEscritura)){
						memcpy(osada_drive.data[bloqueArranque]+byteComienzoEscritura,*buf+((int)size-sizeAux),OSADA_BLOCK_SIZE - byteComienzoEscritura);
					}else{
						memcpy(osada_drive.data[bloqueArranque]+byteComienzoEscritura,*buf+((int)size-sizeAux),sizeAux);
					}

					if(sizeAux >= (OSADA_BLOCK_SIZE - byteComienzoEscritura)){
						log_info(logPokedexServer,"Size aux vale %d y byte comienzo vale %d",sizeAux,byteComienzoEscritura);
						actualizarBytesEscritos(&bytesEscritos,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					}else{
						log_info(logPokedexServer,"Size aux vale %d y byte comienzo vale %d",sizeAux,byteComienzoEscritura);
						actualizarBytesEscritos(&bytesEscritos,sizeAux);
					}
					sizeAux=sizeAux-(OSADA_BLOCK_SIZE-byteComienzoEscritura);

					log_info(logPokedexServer, "OSADA - DATOS: Se han escrito %d bytes\n",bytesEscritos);
					byteComienzoEscritura=0;
					bloqueArranque=avanzarBloquesParaEscribir(bloqueArranque,1);
				}

			}else{
				bytesEscritos=-ENOMEM;
			}
	}else{
		return -ENOENT;
	}
	return bytesEscritos;
}

int osada_read(char *path, char** buf, size_t size, off_t offset){

	int indice = osada_TA_obtenerIndiceTA(path);
	log_info(logPokedexServer, "OSADA - El indice que se obtiene es: %d", indice);


	if (indice != -1){
		int bloque=osada_drive.directorio[indice].first_block;
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n", path, bloque);

		double desplazamientoHastaElBloque=floor(offset/OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Tengo que desplazarme %f bloques\n",desplazamientoHastaElBloque);

		int bloqueArranque=avanzarBloquesParaLeer(bloque,desplazamientoHastaElBloque);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Comienzo a leer desde el bloque %d\n",bloqueArranque);

		int byteComienzoLectura = 0;
		if(offset != 0){
			byteComienzoLectura = offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		}

		log_info(logPokedexServer, "Empiezo a leer desde el byte %d\n",byteComienzoLectura);
		int desplazamiento = 0;
		int iSize = size;
		int fileSize = osada_drive.directorio[indice].file_size;

		while (bloqueArranque!=0xFFFF && bloqueArranque != -1 && (iSize-desplazamiento)>0 && (fileSize-desplazamiento-offset)>0){
			//falta chequear inicio
			log_info(logPokedexServer, "OSADA - Quiero leer %d bytes", OSADA_BLOCK_SIZE-byteComienzoLectura);
			log_info(logPokedexServer, "OSADA - El bloque de arranque: %d", bloqueArranque);

			if((fileSize-desplazamiento-offset)>=(OSADA_BLOCK_SIZE-byteComienzoLectura)){
				if((iSize-desplazamiento)>=(OSADA_BLOCK_SIZE-byteComienzoLectura)){
					log_info(logPokedexServer, "OSADA - El realloc va a ser: %d", desplazamiento+OSADA_BLOCK_SIZE-byteComienzoLectura);
					*buf = realloc(*buf, desplazamiento+OSADA_BLOCK_SIZE-byteComienzoLectura);
					memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,OSADA_BLOCK_SIZE-byteComienzoLectura);
					desplazamiento += OSADA_BLOCK_SIZE-byteComienzoLectura;
				}else{
					log_info(logPokedexServer, "OSADA - El realloc va a ser: %d", iSize);
					*buf = realloc(*buf, iSize);
					memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,iSize-desplazamiento);
					desplazamiento += iSize-desplazamiento;
				}
			}else{
				log_info(logPokedexServer, "OSADA - El realloc va a ser: %d", fileSize-offset);
				*buf = realloc(*buf, fileSize-offset);
				memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,fileSize-desplazamiento-offset);
				desplazamiento += fileSize-desplazamiento-offset;
			}

			log_info(logPokedexServer, "OSADA - DATOS: Se leyo esta informacion: %s\n",string_duplicate(*buf));
			bloqueArranque=osada_drive.asignaciones[bloqueArranque];
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: El bloque siguiente es: %d\n",bloqueArranque);
			byteComienzoLectura=0;
		}
		log_info(logPokedexServer, "OSADA - DATOS: Se leyo el buf posta: %s\n",*buf);

		log_info(logPokedexServer,"El tamanio del buffer es %d bytes",desplazamiento);
		return desplazamiento;
	}


	return -ENOMEM;
}


int osada_open(char* path){

	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
	int child = osada_TA_obtenerIndiceTA(path);
	if(child>=0 && child != -1){
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: La funcion open encontro que el bloque ocupado por %s es %d\n",path,child);
		if(osada_drive.directorio[child].state == 2 || osada_drive.directorio[child].state == 1){
			return 1;
		}
	}

	return -ENOENT;
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

bool hayPosicionDisponibleEnTablaDeArchivos (int pos){
	return osada_drive.directorio[pos].state==0;
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
	pthread_mutex_lock(&mutexTablaArchivos);
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

	pthread_mutex_unlock(&mutexTablaArchivos);
}

int osada_createFile(char* path, mode_t mode){
	int resultado;
	int posicionLibreEnBitmap = -1;
	buscarLugarLibreEnBitmap(&posicionLibreEnBitmap);
	int posicionEnTablaDeArchivos=buscarLugarLibreEnTablaArchivos();
	int tamanioNombre=obtenerLongitudDelNombreDelArchivo(path);
	if (posicionLibreEnBitmap != -1 && posicionEnTablaDeArchivos>=0 && tamanioNombre>0){
		log_info(logPokedexServer, "OSADA - BITMAP: Hay lugar libre para crear archivo\n");
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El archivo %s ocupara la posicion %d\n",path,posicionEnTablaDeArchivos);
		generarNuevoArchivoEnTablaDeArchivos(path,posicionEnTablaDeArchivos);
		resultado = 1;
	}else{
		log_info(logPokedexServer, "OSADA - No se encontro lugar libre en el bitmap");
		resultado = -ENOSPC;
	}

	return resultado;
}

int osada_createDir(char* path){
	char* name;
	int i=0;
	char** directorio=string_split(path,"/");
	char* directorioPadre=string_new();
	while(directorio[i]!=NULL){
		if(directorio[i+1]!=NULL){
			string_append(&directorioPadre,directorio[i]);
			string_append(&directorioPadre,"/");
		}else{
			name=string_duplicate(directorio[i]);
		}
		free(directorio[i]);
		i++;
	}
	free(directorio);
	int subindice=osada_TA_obtenerIndiceTA(directorioPadre);
	free(directorioPadre);
	if(subindice != -1){
		if(darDeAltaDirectorioEnTablaDeArchivos(name, subindice) == 1){
			free(name);
			return 1;
		}else{
			free(name);
			return -ENOENT;
		}
	}
	free(name);
	return -ENOENT;
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

int osada_rename(char* path, char* nuevaPath){
	int resultado;
	int subindice=osada_TA_obtenerIndiceTA(path);
	log_info(logPokedexServer,"El subindice es %d",subindice);
	if(subindice != -1){
			char* nombre=string_new();
			int subindicePath=getFileNameFromPath(nuevaPath, &nombre);
			log_info(logPokedexServer, "OSADA - Renombrando archivo: El nombre del archivo nuevo es: %s\n",nombre);
			if (renombrarArchivo(subindice,nombre, subindicePath)==1){
				log_info(logPokedexServer, "OSADA - Renombrando archivo: Se ha renombrado el archivo correctamente\n");
				resultado = 1;
			}else{
				resultado = -EACCES;
			}
			free(nombre);
	}else{
			resultado = -ENOENT;
			perror("NO se pudo cambiar de nombre al archivo porque no existe");
		}


	return resultado;
}


int calcularBloquesQueOcupaDesdeElPrimerBloque (int primerBloque){
	int bloques=0;
	log_info(logPokedexServer, "OSADA - Antes de entrar al while de calcularBloquesQueOcupaDesdeElPrimerBloque, con el bloque: %d", primerBloque);
	while (primerBloque!=0xFFFFFF && primerBloque != -1){
		bloques++;
		primerBloque=osada_drive.asignaciones[primerBloque];
		log_info(logPokedexServer, "OSADA - El bloque dentro del while es: %d", primerBloque);
	}
	return bloques;
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
int osada_truncate(char* path, off_t offset){
	float espacioDisponible=calcularEspacioDisponibleEnDisco();
	if (offset<=espacioDisponible){
		int subindice=osada_TA_obtenerIndiceTA(path);
			log_info(logPokedexServer, "OSADA - El subindice que se obtuvo es: %d", subindice);
			int originalFileSize=osada_drive.directorio[subindice].file_size;
			log_info(logPokedexServer,"El size original es %d",originalFileSize);
			int bloquesQueOcupa=ceil((float)osada_drive.directorio[subindice].file_size/(float)OSADA_BLOCK_SIZE);
			log_info(logPokedexServer,"%s ocupa %d bloques",path,bloquesQueOcupa);
			int nuevoUltimoBloque, bloquesNecesarios;
			t_list* listaDeBloquesParaLlenar=list_create();
			int block=osada_drive.directorio[subindice].first_block;
			log_info(logPokedexServer,"%s ocupa estos bloques:",path);
			log_info(logPokedexServer,"Bloque %d",block);
		if(subindice != -1) {
			if (osada_drive.directorio[subindice].file_size>offset){
				log_info(logPokedexServer, "OSADA - El file_size es mayor al offset");
				bloquesATruncar(subindice,offset,&nuevoUltimoBloque);
				liberarEspacio(subindice, offset, nuevoUltimoBloque);
				log_info(logPokedexServer, "OSADA - Truncate: Se han liberado %d bytes\n",osada_drive.directorio[subindice].file_size-offset);
				return 1;
			}else if(osada_drive.directorio[subindice].file_size<offset){
				log_info(logPokedexServer,"El offset es mayor que el file_Size");
				contarBloquesSegun(originalFileSize,offset,&bloquesNecesarios);
				if (hayBloquesDesocupadosEnElBitmap(&bloquesNecesarios,listaDeBloquesParaLlenar)){
					ocuparEspacio(subindice,offset-osada_drive.directorio[subindice].file_size, offset, listaDeBloquesParaLlenar);
					log_info(logPokedexServer, "OSADA - Truncate: Se han ocupado %d bytes\n",(int)offset-(int)originalFileSize);
					return 1;
				}else{
					log_info(logPokedexServer, "OSADA - No hay bloques desocupados en el bitmap");
					return -ENOMEM;
				}
			}
			else{
			log_info(logPokedexServer, "OSADA - El file_size del subindice es igual al offset");
			return 1;
		}
	}
	}else{
		log_info(logPokedexServer,"El archivo es muy grande, pesa %fl y hay %fl bytes disponibles",(float)offset,espacioDisponible);
		return -EFBIG;
	}
	log_info(logPokedexServer, "OSADA - No existe el path indicado");
	return -ENOENT;
}

int contarBloquesLibresTotales(){
	pthread_mutex_lock(&mutexBitmap);
	int t=bitarray_get_max_bit(osada_drive.bitmap);
	int i,tot=0;
	for (i=0;i<t;i++){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			tot++;
		}
	}
	pthread_mutex_unlock(&mutexBitmap);
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

int osada_statfs(const char * path, t_statfs* stats){
	stats->f_bavail=contarBloquesLibresTotales();
	stats->f_bfree=stats->f_bavail;
	stats->f_blocks=osada_drive.header->fs_blocks;
	stats->f_bsize=osada_drive.header->fs_blocks;
	stats->f_favail=contarOsadaFilesLibres();
	stats->f_ffree=stats->f_favail;
	stats->f_files=2048;
	stats->f_namemax=OSADA_FILENAME_LENGTH;
	stats->f_fsid=(int)osada_drive.header->magic_number;
	stats->f_frsize=OSADA_BLOCK_SIZE;
	return 1;
}
int osada_fallocate(const char* path, int amount, off_t sizeh, off_t sizef){
	return 0;
}
