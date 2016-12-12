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
#include "functions/data.h"
#include "functions/bitmap.h"
#include "functions/mutex.h"

int osada_init(char* path){
	initOsada (path);
	mutex_init();
	return 1;
}

/*------------REVISAR SEMAFOROS DEL GETATTR PORQUE AL DESCONECTAR EL CLIENTE TIRA VIOLACION DE SEGMENTO EN EL SERVIDOR-------*/
int osada_getattr(char* path, file_attr* attrs){
	if(strcmp(path,"/") == 0){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		int indice = osada_TA_obtenerIndiceTA(path);
		log_info(logPokedexServer, "GETATTR - El indice obtenido para el path %s es %d", path, indice);
		if(indice>=0 && strcmp(path, "") != 0){
			osada_TA_obtenerAttr(indice, attrs);
			return 1;
		}
		return 0;
	}
}

int osada_readdir(char* path, t_list* directorios){
	int parent;
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
		if(osada_drive.directorio[child].state == 2 || osada_drive.directorio[child].state == 1){
			return 1;
		}
	}

	return -ENOENT;
}

int osada_read(char *path, char** buf, size_t size, off_t offset){
	int indice = osada_TA_obtenerIndiceTA(path);


	if (indice != -1){
		mutex_lockFile(indice);
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
		mutex_unlockFile(indice);
		return desplazamiento;
	}

	mutex_unlockFile(indice);
	return -ENOMEM;
}

int osada_createFile(char* path){
	if(osada_TA_createNewDirectory(path,REGULAR)){
		return 1;
	}

	return -ENOSPC;
}

int osada_truncate(char* path, off_t offset){
	int indice=osada_TA_obtenerIndiceTA(path);

	float espacioDisponible= osada_B_cantBloquesLibres() * OSADA_BLOCK_SIZE;
	if(indice != -1){
		mutex_lockFile(indice);
		int cantBlockOffset = ceil(offset/OSADA_BLOCK_SIZE);
		int cantBlockFileSize =  ceil(osada_drive.directorio[indice].file_size/OSADA_BLOCK_SIZE);

		if(osada_drive.directorio[indice].file_size>offset){
			int nuevoFinBlock = osada_TG_avanzarNBloques(osada_drive.directorio[indice].first_block, cantBlockOffset);
			int bloqueLiberar = osada_drive.asignaciones[nuevoFinBlock];
			osada_drive.asignaciones[nuevoFinBlock] = 0xFFFF;

			while(bloqueLiberar != 0xFFFF && bloqueLiberar != -1){
				bitarray_clean_bit(osada_drive.bitmap, bloqueLiberar);
				pthread_mutex_unlock(&osada_mutex.block[bloqueLiberar]);
				bloqueLiberar = osada_drive.asignaciones[bloqueLiberar];
			}

			if(offset>0){
				int offBlock = offset - (cantBlockOffset * OSADA_BLOCK_SIZE);
				osada_D_truncateBlock(nuevoFinBlock, offBlock);
			}

			osada_drive.directorio[indice].file_size = offset;

			mutex_unlockFile(indice);
			return 1;
		}else if(osada_drive.directorio[indice].file_size<offset){
			if ((offset-osada_drive.directorio[indice].file_size)<=espacioDisponible){
				int bloquePadre = osada_TG_avanzarNBloques (osada_drive.directorio[indice].first_block, cantBlockFileSize);
				int cantBlocksToReserve = cantBlockOffset-cantBlockFileSize;

				log_info(logPokedexServer, "cantBlockOffset: %d", cantBlockOffset);
				log_info(logPokedexServer, "cantBlockFileSize: %d", cantBlockFileSize);

				if(cantBlocksToReserve>0){
					osada_B_reserveNewBlocks(&cantBlocksToReserve, &bloquePadre, indice);
				}

				if(osada_drive.directorio[indice].file_size>0){
					int offBlock = osada_drive.directorio[indice].file_size - (cantBlockFileSize * OSADA_BLOCK_SIZE);
					log_info(logPokedexServer, "Offset las block: %d", offBlock);
					osada_D_truncateBlock(bloquePadre, offBlock);
				}

				osada_drive.directorio[indice].file_size = offset;

				mutex_unlockFile(indice);
				return 1;
			}else{
				mutex_unlockFile(indice);
				log_info(logPokedexServer,"El archivo es muy grande, quiere agrandarse en %fl bytes y hay disponibles %fl bytes",(float)(offset-osada_drive.directorio[indice].file_size),espacioDisponible);
				return -ENOSPC;
			}
		}else{
			mutex_unlockFile(indice);
			return 1;
		}
	}else{
		mutex_unlockFile(indice);
		log_info(logPokedexServer, "OSADA - No existe el path indicado");
		return -ENOENT;
	}
}

int osada_createDir(char* path){
	if(osada_TA_createNewDirectory(path, DIRECTORY)){
		return 1;
	}

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
	int indice = osada_TA_obtenerIndiceTA(path);

	if(indice >= 0 || indice == 0xFFFF){
		osada_TA_deleteDirectory(indice, REGULAR);
		return 1;
	}

	return -ENOENT;

}

int osada_removeDir(char* path){
	int indice = osada_TA_obtenerIndiceTA(path);

	if(indice >= 0 || indice == 0xFFFF){
		t_list* directories=list_create();
		osada_TA_obtenerDirectorios(indice, directories);
		if (list_is_empty(directories)){
			osada_TA_deleteDirectory(indice, DIRECTORY);
			list_destroy(directories);
			return 1;
		}else{
			list_destroy(directories);
			return -EBUSY;
		}
		list_destroy(directories);
	}

	return -ENOENT;
}

int osada_write(char* path,char** buf, size_t size, off_t offset){
//	log_info(logPokedexServer,"El buf que me llega es: %s",*buf);
	int indice = osada_TA_obtenerIndiceTA(path);
	int bytesEscritos=0;
	log_info(logPokedexServer,"El path es %s y el indice es %d",path,indice);
	log_info(logPokedexServer, "El offset es %d", offset);


	if(indice != -1){
		//mutex_lockFile(indice);
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

			if (osada_B_reserveNewBlocksForWriteDeleteLatter(&bloquesQueNecesitoEscribir, &bloqueArranque, indice) && byteComienzoEscritura>=0){
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

	//mutex_unlockFile(indice);
	return bytesEscritos;
}
