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
extern pthread_mutex_t mutexTablaAsignaciones;
extern pthread_mutex_t mutexBitmap;
extern pthread_mutex_t mutexDatos;



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
	//log_info(logPokedexServer,"El primer bloque era: %d",osada_drive.directorio[subindice].first_block);
	//log_info(logPokedexServer,"nuevoUltimoBloque vale: %d",*nuevoUltimoBloque);
	for(i=0;i<bloquesQueNecesitoMoverme;i++){
		*nuevoUltimoBloque=osada_drive.asignaciones[*nuevoUltimoBloque];
		log_info(logPokedexServer,"nuevoUltimoBloque vale ahora: %d",*nuevoUltimoBloque);
	}
	log_info(logPokedexServer, "El nuevo ultimo bloque es %d",*nuevoUltimoBloque);
}

int hayBloquesDesocupadosEnElBitmap (int* n, int* bloqueArranque){
	int bloqueReal = *bloqueArranque;
	//log_info(logPokedexServer, "n vale %d",*n);
	int i=osada_drive.header->bitmap_blocks+1025+((osada_drive.header->fs_blocks-1025-osada_drive.header->bitmap_blocks)*4/OSADA_BLOCK_SIZE);
	int bloquesNecesarios=0;

	//log_info(logPokedexServer,"Empiezo a leer desde el bloque %d",i);
	//log_info(logPokedexServer,"El bitmap tiene %d bits",bitarray_get_max_bit(osada_drive.bitmap));
	while(bloquesNecesarios<*n && i<=bitarray_get_max_bit(osada_drive.bitmap)){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			//log_info(logPokedexServer,"El bit %d del bitmap vale %d",i,bitarray_test_bit(osada_drive.bitmap,i));
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
			//log_info(logPokedexServer,"OSADA - Bloques desocupados. Agrego a la lista: %d",list_get(lista,j));
			bloquesNecesarios++;
			//log_info(logPokedexServer,"OSADA - Hay  %d bloques libres",bloquesNecesarios);

		}
		i++;
		//log_info(logPokedexServer,"i vale %d",i);
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

	int resultadoDeBuscarRegistroPorNombre;
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
//Path es la ruta al archivo
//Buf viene vacio y se llena con la info que hay en la path
//size es cuantos bytes del archivo hay que leer
//offset es desde donde tengo que leer

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
//	char* fileName = string_new();
//	char* directoryName = string_new();
	time_t timer=time(0);
//	directoryContainingFile(path,&fileName,&directoryName);
//
//	log_info(logPokedexServer, "OSADA - El lugar libre en la tabla de archivos que nos llega de osada_write es: %d", indice);

	osada_drive.directorio[indice].file_size = size;
	log_info(logPokedexServer, "OSADA - El file_size del path %s es %d, y el size que me llego por parametro es %d",path, osada_drive.directorio[indice].file_size, size);
//	osada_drive.directorio[indice].first_block = bloqueArranque;

//	char* barrasCero=string_repeat('\0',17);
//	memcpy(osada_drive.directorio[indice].fname,barrasCero,17);
//	log_info(logPokedexServer,"El nombre inicial despues del memcpy es %s",osada_drive.directorio[indice].fname);
//	log_info(logPokedexServer,"El directorio padre de %s es %d",fileName,osada_drive.directorio[indice].parent_directory);
//	int i;
//	for (i=0;i<17;i++){
//		osada_drive.directorio[indice].fname[i]=fileName[i];
//	}

//	osada_drive.directorio[indice].state = 1;
	osada_drive.directorio[indice].lastmod = timer;

//	free(fileName);
//	free(directoryName);
}

//int buscarIndiceEnTablaDeArchivos(char* path, int indice){
//	char* fileName = string_new();
//	char* directoryName = string_new();
//	time_t timer=time(0);
//	directoryContainingFile(path,&fileName,&directoryName);
//
//	int i;
//	log_info(logPokedexServer, "OSADA - El indicePadre (dentro de buscarIndiceEnTablaDeArchivos) es: %d", indice);
//	free(directoryName);
//	for(i=0; i<2048; i++){
//		if(strcmp(osada_drive.directorio[indice].fname,fileName) == 0){
//			free(fileName);
//			return i;
//		}
//	}
//
//	return -1;
//}

//	/README.txt
int osada_write(char* path,char** buf, size_t size, off_t offset){
	//int resultadoDeBuscarRegistroPorNombre;
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
			//char* bufUpdated=malloc(size);
		//	strcpy(bufUpdated,buf);
			int bloquesQueNecesitoEscribir=ceil((float)size/(float)OSADA_BLOCK_SIZE);
			int sizeAux=(int)size;
			log_info(logPokedexServer,"Size aux vale %d y size comienzo vale %d",sizeAux,size);
			if (hayBloquesDesocupadosEnElBitmap(&bloquesQueNecesitoEscribir, &bloqueArranque) && byteComienzoEscritura>=0){
				if(offset==0){
					log_info(logPokedexServer,"Bloque Arranque es: %d",bloqueArranque);
					osada_drive.directorio[indice].first_block=bloqueArranque;
					log_info(logPokedexServer,"El first block del indice %d es: %d",indice, osada_drive.directorio[indice].first_block);
				}
				actualizarTablaDeArchivosParaWrite(path, size, indice);
				while (sizeAux>0 && bloqueArranque != 0xFFFF){
					log_info(logPokedexServer,"Size aux vale %d",sizeAux);
		//			int lugarLibreEnLaTablaDeArchivos = buscarIndiceEnTablaDeArchivos(path, indice);

					log_info(logPokedexServer, "El contenido del buffer es %s\n",*buf);
			//		bitarray_set_bit(osada_drive.bitmap,bloqueArranque);
					log_info(logPokedexServer, "OSADA - BITMAP: Marco al bloque %d como %d\n",bloqueArranque, bitarray_test_bit(osada_drive.bitmap,bloqueArranque));

					if(sizeAux >= (OSADA_BLOCK_SIZE - byteComienzoEscritura)){
						memcpy(osada_drive.data[bloqueArranque]+byteComienzoEscritura,*buf,OSADA_BLOCK_SIZE - byteComienzoEscritura);
					}else{
						memcpy(osada_drive.data[bloqueArranque]+byteComienzoEscritura,*buf,sizeAux);
					}

	//				log_info(logPokedexServer, "OSADA - Acabamos de escribir el buf: %s", osada_drive.data[bloqueArranque]+byteComienzoEscritura);

					//actualizarBuffer(buf,OSADA_BLOCK_SIZE-byteComienzoEscritura);
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
			//		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: Avanzo al bloque %d\n",bloqueArranque);
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

	char* bufAuxiliar = malloc(size);
	if (indice != -1){
		//con el indice voy a TA y busco el FB
		int bloque=osada_drive.directorio[indice].first_block;
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n", path, bloque);
		//offset/TAMBLQ= R ,rrdondearlo para arriba y restarle 1-->2
		double desplazamientoHastaElBloque=floor(offset/OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Tengo que desplazarme %f bloques\n",desplazamientoHastaElBloque);
		//Voy a FB y avanzo 2 dentro de Tasignaciones
		int bloqueArranque=avanzarBloquesParaLeer(bloque,desplazamientoHastaElBloque);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Comienzo a leer desde el bloque %d\n",bloqueArranque);
		//RDO=ofsset-(RxBSIZE)=cuando llegue al bloque solicitado hago *data (en declarations.h) y me muevo (se sumo) RDO
		int byteComienzoLectura = 0;
		if(offset != 0){
			byteComienzoLectura = offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		}

		log_info(logPokedexServer, "Empiezo a leer desde el byte %d\n",byteComienzoLectura);
//		int byteCopiados = size;
		int desplazamiento = 0;
		int sizeParaEscribir = size;
//		char* buf = malloc(osada_drive.directorio[indice].file_size);

		while (bloqueArranque!=0xFFFF && bloqueArranque != -1 && sizeParaEscribir > 0/*&& byteCopiados>0*/){
			//falta chequear inicio
			log_info(logPokedexServer, "OSADA - Quiero leer %d bytes", OSADA_BLOCK_SIZE-byteComienzoLectura);
			log_info(logPokedexServer, "OSADA - El bloque de arranque: %d", bloqueArranque);
	/*		if(byteCopiados >= (OSADA_BLOCK_SIZE-byteComienzoLectura)){
				char* substring = string_substring(osada_drive.data[bloqueArranque], byteComienzoLectura, OSADA_BLOCK_SIZE);
				log_info(logPokedexServer, "OSADA - El substring: %s", substring);
				string_append(buf,substring);
				byteCopiados -= OSADA_BLOCK_SIZE-byteComienzoLectura;
			}else{
				char* substring = string_substring(osada_drive.data[bloqueArranque], byteComienzoLectura, byteCopiados);
				log_info(logPokedexServer, "OSADA - El substring: %s", substring);
				string_append(buf,substring);
				byteCopiados = 0;
			}
	*/

			memcpy(bufAuxiliar+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,OSADA_BLOCK_SIZE);
			desplazamiento += OSADA_BLOCK_SIZE;
			sizeParaEscribir -= OSADA_BLOCK_SIZE;
//			log_info(logPokedexServer, "OSADA - DATOS: Se leyo esta informacion: %s\n",*buf);
			log_info(logPokedexServer, "OSADA - DATOS: Se leyo esta informacion: %s\n",string_duplicate(bufAuxiliar));
			bloqueArranque=osada_drive.asignaciones[bloqueArranque];
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: El bloque siguiente es: %d\n",bloqueArranque);
//			byteComienzoLectura=0;
		}
//		log_info(logPokedexServer, "OSADA - DATOS: Se han leido %d bytes\n", string_length(bufAuxiliar));
		if(osada_drive.directorio[indice].file_size > size){
			memcpy(*buf,bufAuxiliar,size);
		}else{
			memcpy(*buf,bufAuxiliar,osada_drive.directorio[indice].file_size);
		}
//		string_append(buf, '\0');
//		*buf = string_substring(bufAuxiliar, 0, osada_drive.directorio[indice].file_size);
//		memcpy(bufAuxiliar,bufAuxiliar,osada_drive.directorio[indice].file_size);
//		log_info(logPokedexServer, "OSADA - DATOS: Se leyo el bufAuxiliar: %s\n",bufAuxiliar);
//		memcpy(buf,bufAuxiliar,osada_drive.directorio[indice].file_size);
		log_info(logPokedexServer, "OSADA - DATOS: Se leyo el buf posta: %s\n",*buf);
		free(bufAuxiliar);
		log_info(logPokedexServer,"El tamanio del buffer es %d bytes",string_length(*buf));
		return string_length(*buf);
	}

	free(bufAuxiliar);
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
/*int hayBloquesLibres(t_list* listaDeBloques, int bloquesNecesarios){
	bool noMeAlcanzan=true;
	int tam=bitarray_get_max_bit(osada_drive.bitmap);
	int i;
	while(noMeAlcanzan){
		int posicionEnBitmap=0;
		if(bitarray_test_bit(osada_drive.bitmap,posicionEnBitmap)){
			if (posicionEnBitmap>=tam){
				//perror("No hay bloques libres en el bitmap");
				//el for se hace por si no me alcanzan los bloques, los libero
				for (i=0;i<list_size(listaDeBloques);i++){
					bitarray_clean_bit(osada_drive.bitmap,(int)list_get(listaDeBloques,i));
				}
				return -ENOSPC;
			}else{
				posicionEnBitmap++;
			}
		}else{
			list_add(listaDeBloques,(void*)posicionEnBitmap);
			bitarray_set_bit(osada_drive.bitmap,posicionEnBitmap);
			if(list_size(listaDeBloques)==bloquesNecesarios){
				noMeAlcanzan=false;
			}

		}
	}
	return 1;
}*/

void buscarLugarLibreEnBitmap(int* lugarLibre){
	int bloqueInicialDeDatos=osada_drive.header->bitmap_blocks+1025+(osada_drive.header->fs_blocks-osada_drive.header->bitmap_blocks-1025)*4/OSADA_BLOCK_SIZE;
	int i;
	log_info(logPokedexServer, "OSADA - El tamanio del bitmap de osada_drive es: %d", bitarray_get_max_bit(osada_drive.bitmap));

	for (i=bloqueInicialDeDatos;i<=bitarray_get_max_bit(osada_drive.bitmap);i++){
	//	printf("%d", bitarray_test_bit(osada_drive.bitmap,i));
		if (bitarray_test_bit(osada_drive.bitmap,i)){
		}else{
//			log_info(logPokedexServer, "OSADA - El bit del bloque %d es %d", i, bitarray_test_bit(osada_drive.bitmap,i));
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
	int i, resultadoDeBuscarRegistroPorNombre;
	time_t timer=time(0);
	char* fileName=string_new();
	char* directoryName=string_new();
	directoryContainingFile(path,&fileName, &directoryName);
	log_info(logPokedexServer, "OSADA - Generacion nuevo archivo: El nombre del archivo es: %s\n",fileName);
	pthread_mutex_lock(&mutexTablaArchivos);
	osada_drive.directorio[posicionEnTablaArchivos].file_size=0;
	log_info(logPokedexServer,"El archivo %s de la path %s ocupa %d bytes",fileName,directoryName,osada_drive.directorio[posicionEnTablaArchivos].file_size);
	osada_drive.directorio[posicionEnTablaArchivos].first_block=0xFFFF;
	/*strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].lastmod=atoi(fecha);
	log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: La fecha de modificacion es %d\n",osada_drive.directorio[posicionEnTablaArchivos].lastmod);*/
	log_info(logPokedexServer,"El nombre inicial es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	char* barrasCero=string_repeat('\0',17);
	memcpy(osada_drive.directorio[posicionEnTablaArchivos].fname,barrasCero,17);
	log_info(logPokedexServer,"El nombre inicial despues del memcpy es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].parent_directory=osada_TA_obtenerIndiceTA(directoryName);
	log_info(logPokedexServer,"El directorio padre de %s es %d",fileName,osada_drive.directorio[posicionEnTablaArchivos].parent_directory);
	for (i=0;i<17;i++){
		osada_drive.directorio[posicionEnTablaArchivos].fname[i]=fileName[i];
	}
	log_info(logPokedexServer,"El nombre final es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].state=REGULAR;
	osada_drive.directorio[posicionEnTablaArchivos].lastmod=timer;
//	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].parent_directory=osada_TA_obtenerUltimoHijoFromPath(directoryName, &resultadoDeBuscarRegistroPorNombre);
	/*free(fecha);*/
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
//	log_info(logPokedexServer, "OSADA - El subindice obtenido es: %d", subindice);
	free(directorioPadre);
	//aca hay que obtener el hijo del ultimo path/ parametro es el path
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
	//ciudad/pokemon/archivo.txt

	//[ciudad,pokemon,archivo.txt,null]
	//*nombre=pathSplitteada[strlen(*pathSplitteada)-1];
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
//	pthread_mutex_lock(&mutexTablaAsignaciones);
	log_info(logPokedexServer, "OSADA - Antes de entrar al while de calcularBloquesQueOcupaDesdeElPrimerBloque, con el bloque: %d", primerBloque);
	while (primerBloque!=0xFFFFFF && primerBloque != -1){
		bloques++;
		primerBloque=osada_drive.asignaciones[primerBloque];
		log_info(logPokedexServer, "OSADA - El bloque dentro del while es: %d", primerBloque);
	}
//	pthread_mutex_unlock(&mutexTablaAsignaciones);
	return bloques;
}

void actualizarTablaDeArchivos(int subindice, off_t offset){
//	time_t timer=time(0);
//	struct tm *tlocal = localtime(&timer);
//	char* fecha=string_new();

	log_info(logPokedexServer, "OSADA - Antes del strftime");
//	strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
//	osada_drive.directorio[subindice].lastmod=atoi(fecha);
//	log_info(logPokedexServer, "OSADA - La nueva fecha de modificacion es: %d", osada_drive.directorio[subindice].lastmod);
	osada_drive.directorio[subindice].file_size = offset;
	log_info(logPokedexServer, "OSADA - El nuevo size del archivo es: %d",osada_drive.directorio[subindice].file_size);

//	log_info(logPokedexServer, "OSADA - La fecha es: %s", fecha);
//	log_info(logPokedexServer, "OSADA - E tamanio nuevo es: %d", offset);

//	free(fecha);
//	free(tlocal);
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




//	 pthread_mutex_unlock(&mutexBitmap);
//	 	 pthread_mutex_unlock(&mutexTablaArchivos);
//	 	 pthread_mutex_unlock(&mutexTablaAsignaciones);
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

//--------------- ARREGLAR EL MEMCPY DE ESTA FUNCION ---------------
/*void ocuparEspacioEnDatos(int bloqueAOcupar, off_t offset, int subindice){
	int byteComienzo = osada_drive.directorio[subindice].file_size % OSADA_BLOCK_SIZE;
	log_info(logPokedexServer, "OSADA - El bloqueAOcupar es: %d", bloqueAOcupar);
	log_info(logPokedexServer, "OSADA - El byteComienzo es: %d", byteComienzo);
	log_info(logPokedexServer, "OSADA - El data de bloqueAOcupar es: %s", (char*)osada_drive.data[bloqueAOcupar]);

	char barraCero = '\0';
	if(byteComienzo == 0){
		log_info(logPokedexServer, "OSADA - Pasa el memcpy cuando byteComienzo == 0");
		memcpy(osada_drive.data[bloqueAOcupar], &barraCero, OSADA_BLOCK_SIZE);
	}else{
		log_info(logPokedexServer, "OSADA - Pasa el memcpy cuando byteComienzo != 0");
		memcpy(osada_drive.data[bloqueAOcupar]+byteComienzo, &barraCero, OSADA_BLOCK_SIZE-byteComienzo);
	}

}*/
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
	int i;
	log_info(logPokedexServer,"El ultimo bloque es %d",ultimoBloque);
	char* barrasCero=string_repeat('\0',bytesConBarraCeroDelUltimoBloque);
	memcpy(osada_drive.data[ultimoBloque]+bytesOcupadosDelUltimoBloque,barrasCero,bytesConBarraCeroDelUltimoBloque);
	ultimoBloque=osada_drive.asignaciones[ultimoBloque];
	while(ultimoBloque!=0xFFFF && ultimoBloque!=-1){
		barrasCero=string_repeat('\0',OSADA_BLOCK_SIZE);
		memcpy(osada_drive.data[ultimoBloque]+OSADA_BLOCK_SIZE,barrasCero,OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "Lleno el bloque %d con",ultimoBloque);
		//log_info(logPokedexServer,"El contenido del bloque %d es: %s",ultimoBloque,osada_drive.data[ultimoBloque]);
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
//		ocuparEspacioEnDatos(ultimoBloque, offset, sub);
		//int bloqueAOcupar;
		/*for (i=0;i<bq_to_set;i++){
			buscarLugarLibreEnBitmap(&bloqueAOcupar);
			log_info(logPokedexServer, "OSADA - El bloque a ocupar es: %d", bloqueAOcupar);
			bitarray_set_bit(osada_drive.bitmap,bloqueAOcupar);
			log_info(logPokedexServer, "OSADA - El bit de ocupado en el bloque %d es %d", bloqueAOcupar, bitarray_test_bit(osada_drive.bitmap,bloqueAOcupar));
			ocuparBloqueSegunElUltimo(ultimoBloque,bloqueAOcupar);
	//		ocuparEspacioEnDatos(bloqueAOcupar, offset, sub);

			int byteComienzo = osada_drive.directorio[sub].file_size % OSADA_BLOCK_SIZE;
			log_info(logPokedexServer, "OSADA - El bloqueAOcupar es: %d", bloqueAOcupar);
			log_info(logPokedexServer, "OSADA - El byteComienzo es: %d", byteComienzo);
			log_info(logPokedexServer, "OSADA - El data de bloqueAOcupar es: %s", osada_drive.data[bloqueAOcupar]);

			char barraCero = '\0';
			if(byteComienzo == 0){
				log_info(logPokedexServer, "OSADA - Pasa el memcpy cuando byteComienzo == 0");
				memcpy(osada_drive.data[bloqueAOcupar], &barraCero, OSADA_BLOCK_SIZE);
			}else{
				log_info(logPokedexServer, "OSADA - Pasa el memcpy cuando byteComienzo != 0");
				memcpy(osada_drive.data[bloqueAOcupar]+byteComienzo, &barraCero, OSADA_BLOCK_SIZE-byteComienzo);
			}

		}*/
//		pthread_mutex_unlock(&mutexTablaAsignaciones);
//		pthread_mutex_unlock(&mutexBitmap);
//		pthread_mutex_lock(&mutexBitmap);

}
void contarBloquesSegun(int originalFileSize,int offset, int* bloques){
	/*log_info(logPokedexServer,"Necesito %d libres",ceil((float)offset-(float)originalFileSize/(float)OSADA_BLOCK_SIZE));
	printf("offset vale %d, originalFileSize vale %d, el ceil vale %d, y antes valia %d",offset
			,originalFileSize,ceil((float)offset-(float)originalFileSize/(float)OSADA_BLOCK_SIZE)
			,ceil((float)offset-(float)originalFileSize/(float)OSADA_BLOCK_SIZE));
	*bloques=ceil((float)offset-(float)originalFileSize/(float)OSADA_BLOCK_SIZE);*/
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
