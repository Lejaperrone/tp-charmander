/*
 * osada.c
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <commons/collections/list.h>
#include "commons/declarations.h"
#include "functions/tabla_archivos.h"
#include "commons/osada.h"
#include "osada.h"
#include "mapp.h"
#include <commons/bitarray.h>
#include <sys/stat.h>
#include "functions/tabla_asignaciones.h"
int osada_init(char* path){
	initOsada (path);
	return 1;
}

int osada_removeDir(char* path){
	t_list* directoriosQueComponenElActual=list_create();
	u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	osada_TA_obtenerDirectorios(parent, directoriosQueComponenElActual);
	if (list_is_empty(directoriosQueComponenElActual)){
		bool* hayMasBloques=true;
		//El while existe porque no se si un directorio puede ocupar mas de un bloque
		while(hayMasBloques){
		bitarray_clean_bit(osada_drive.bitmap,parent);
		actualizarTablaDeAsignaciones_porBaja(&parent,&hayMasBloques);
		}
		rmdir(path);
	}else{
		perror("NO se pudo remover el directorio porque no esta vacio");
	}
	list_destroy(directoriosQueComponenElActual);
}

int osada_removeFile(char* path){
	u_int16_t parent=osada_TA_obtenerUltimoHijoFromPath(path);
	 osada_TA_borrarArchivo(path,parent);
}
int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	//Obtengo los directorios
		osada_TA_obtenerDirectorios(parent, directorios);
	//Return
		return 1;


}

/*int osada_createFile(char* path, char* nombreArchivo){

}
int osada_listarArchivos(char* path){

}*/

int osada_getattr(char* path, file_attr* attrs){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);

	if(indice>=0){
		//el indice es numero de posicion en la que esta en la tabla de archivos, si indice vale 6, esta en la
		//posicion 6 de la tabla de achivos
		osada_TA_obtenerAttr(indice, attrs);
		return 1;
	}else if(strcmp(path,"/")){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		return -ENOENT;
	}
}
//Path es la ruta al archivo
//Buf viene vacio y se llena con la info que hay en la path
//size es cuantos bytes del archivo hay que leer
//offset es desde donde tengo que leer
int osada_read(char *path, char *buf, size_t size, off_t offset){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	if (indice >=0){
		u_int16_t directorio =osada_TA_buscarRegistroPorNombre(path,indice);
	}
	if(strcmp(path, "/") != 0){
		return 1;
	}else{
		return -ENOENT;
	}
}

int osada_open(char* path){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t child = osada_TA_obtenerUltimoHijoFromPath(path);
		if(child>=0){
			if(osada_drive.directorio[child].state ==2){
				return -EACCES;
			}
		}

	return -ENOENT;
}
int hayBloquesLibres(t_list* listaDeBloques, int bloquesNecesarios){
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
}

int osada_createDir(char* path, char* name){
	//Creamos lista de los bloques que vamos a necesitar
	t_list* listaDeBloques=list_create();
	int numeroDeBloque;
	if(hayBloquesLibres(listaDeBloques, 64)){
		char* nombre=string_new();
		string_append(&nombre,path);
		string_append(&nombre,name);
		//S_IROTH | S_IWOTH son modos de lectura y escritura para todos los usuarios
		if (mkdir(nombre,(S_IROTH | S_IWOTH))==-1){
			perror("No se pudo crear la carpeta");
			return -ENOENT;
		}
		darDeAltaDirectorioEnTablaDeArchivos(nombre, listaDeBloques);
		modificarBloquesAsignadosATablaDeAsignaciones(listaDeBloques);
		//queremos que lo ponga en 1
		for (numeroDeBloque=0;numeroDeBloque<list_size(listaDeBloques);numeroDeBloque++){
			bitarray_set_bit(osada_drive.bitmap,(int)list_get(listaDeBloques,numeroDeBloque));
		}
		return 1;
	}else{
		if (hayBloquesLibres(listaDeBloques, 64)==28){
			perror("No se pudo crear la carpeta porque no hay bloques libres");
			return -ENOSPC;
		}
	}

}
