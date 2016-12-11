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
#include <commons/bitarray.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <math.h>
#include <commons/log.h>
#include <pthread.h>

#include "commons/osada.h"
#include "osada.h"
#include "basura.h"
#include "mapp.h"
#include "commons/declarations.h"
#include "../commons/definitions.h"
#include "../commons/structures.h"
#include "functions/tabla_archivos.h"
#include "functions/tabla_asignaciones.h"
#include "functions/bitmap.h"

int osada_init(char* path){
	initOsada (path);
	return 1;
}

int osada_getattr(char* path, file_attr* attrs){
	if(strcmp(path,"/") == 0){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		int indice = osada_TA_obtenerIndiceTA(path);
		log_info(logPokedexServer, "GETATTR - El indice obtenido para el path %s es %d", path, indice);
		if(indice>=0){
			osada_TA_obtenerAttr(indice, attrs);
			return 1;
		}
		return 0;
	}
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

int osada_read(char *path, char** buf, size_t size, off_t offset){
	int indice = osada_TA_obtenerIndiceTA(path);

	if (indice != -1){
		int bloque=osada_drive.directorio[indice].first_block;

		double desplazamientoHastaElBloque=floor(offset/OSADA_BLOCK_SIZE);
		int bloqueArranque=osada_TG_avanzarNBloques(bloque,desplazamientoHastaElBloque);

		int byteComienzoLectura = offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		int desplazamiento = 0;
		int iSize = size;
		int fileSize = osada_drive.directorio[indice].file_size;

		while (bloqueArranque!=0xFFFF && bloqueArranque != -1 && (iSize-desplazamiento)>0 && (fileSize-desplazamiento-offset)>0){

			if((fileSize-desplazamiento-offset)>=(OSADA_BLOCK_SIZE-byteComienzoLectura)){
				if((iSize-desplazamiento)>=(OSADA_BLOCK_SIZE-byteComienzoLectura)){
					*buf = realloc(*buf, desplazamiento+OSADA_BLOCK_SIZE-byteComienzoLectura);
					memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,OSADA_BLOCK_SIZE-byteComienzoLectura);
					desplazamiento += OSADA_BLOCK_SIZE-byteComienzoLectura;
				}else{
					*buf = realloc(*buf, iSize);
					memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,iSize-desplazamiento);
					desplazamiento += iSize-desplazamiento;
				}
			}else{
				*buf = realloc(*buf, fileSize-offset);
				memcpy(*buf+desplazamiento,osada_drive.data[bloqueArranque]+byteComienzoLectura,fileSize-desplazamiento-offset);
				desplazamiento += fileSize-desplazamiento-offset;
			}

			bloqueArranque=osada_drive.asignaciones[bloqueArranque];
			byteComienzoLectura=0;
		}

		return desplazamiento;
	}


	return -ENOMEM;
}

int osada_createFile(char* path){

	int posicionLibreEnBitmap = -1;
	osada_B_findFreeBlock(&posicionLibreEnBitmap);
	int posicionEnTablaDeArchivos=osada_TA_obtenerDirectorioLibre();

	char* fileName=string_new();
	char* directoryName=string_new();
	osada_TA_splitPathAndName(path,&fileName,&directoryName);

	int resultado;
	if (posicionLibreEnBitmap != -1 && posicionEnTablaDeArchivos>=0 && string_length(fileName)<=17){
		osada_TA_createNewDirectory(path,posicionEnTablaDeArchivos);
		resultado = 1;
	}else{
		resultado = -ENOSPC;
	}

	return resultado;
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
//			log_info(logPokedexServer,"%s ocupa estos bloques:",path);
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
		return -ENOSPC;
	}
	log_info(logPokedexServer, "OSADA - No existe el path indicado");
	return -ENOENT;
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

int osada_removeFile(char* path){

	int parent;
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

//	perror("NO se pudo remover el directorio porque no existe");
	return -ENOENT;

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

int osada_write(char* path,char** buf, size_t size, off_t offset){
//	log_info(logPokedexServer,"El buf que me llega es: %s",*buf);
	int indice = osada_TA_obtenerIndiceTA(path);
	int bytesEscritos=0;
	log_info(logPokedexServer,"El path es %s y el indice es %d",path,indice);
	log_info(logPokedexServer, "El offset es %d", offset);
	if(indice != -1){
			int bloque=osada_drive.directorio[indice].first_block;
			log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n",path, bloque);
			double desplazamientoHastaElBloque=floor(offset/OSADA_BLOCK_SIZE);
			log_info(logPokedexServer, "OSADA - El desplazamientoHastaElBloque es: %f", desplazamientoHastaElBloque);
			int bloqueArranque=avanzarBloquesParaEscribir(bloque,desplazamientoHastaElBloque);
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Desde el bloque %d me desplace hasta el %d, me movi %f bloques.\n",bloque,bloqueArranque,desplazamientoHastaElBloque);
			int byteComienzoEscritura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
			log_info(logPokedexServer, "OSADA - DATOS: Empiezo a escribir desde el byte: %d\n",byteComienzoEscritura);
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

			if (hayBloquesDesocupadosEnElBitmapParaWrite(&bloquesQueNecesitoEscribir, &bloqueArranque, indice) && byteComienzoEscritura>=0){
				if(ultimoBloqueIndice == 0xFFFF && osada_drive.directorio[indice].first_block != 0xFFFF){
					osada_drive.asignaciones[ultimoBloqueIndice]=bloqueArranque;
				}
//				else if(offset==0){
//					log_info(logPokedexServer,"Bloque Arranque es: %d",bloqueArranque);
//					osada_drive.directorio[indice].first_block=bloqueArranque;
//					log_info(logPokedexServer,"El first block del indice %d es: %d",indice, osada_drive.directorio[indice].first_block);
//				}
				actualizarTablaDeArchivosParaWrite(path, size+offset, indice);
				while (sizeAux>0 && bloqueArranque != 0xFFFF){
					log_info(logPokedexServer,"Size aux vale %d",sizeAux);

	//				log_info(logPokedexServer, "El contenido del buffer es %s\n",*buf);
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
