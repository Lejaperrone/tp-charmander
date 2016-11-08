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
#include <time.h>
#include <commons/collections/list.h>
#include "commons/declarations.h"
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

int hayBloquesDesocupadosEnElBitmap (int n){
	int i,bloquesNecesarios;
	int resultado=0;
	for (i=0;i<bitarray_get_max_bit(osada_drive.bitmap);i++){
		if(bloquesNecesarios==n){
			printf("OSADA - BITMAP: Hay al menos %d bloques desocupados\n",n);
			resultado=1;
		}else{
			if (!bitarray_test_bit(osada_drive.bitmap,i)){
			bloquesNecesarios++;
			}
		}
	}
	return resultado;
}


int osada_init(char* path){
	initOsada (path);
	return 1;
}

int osada_removeDir(char* path){
	int pudeBorrar=1;
	t_list* directoriosQueComponenElActual=list_create();
	u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	osada_TA_obtenerDirectorios(parent, directoriosQueComponenElActual);
	if (list_is_empty(directoriosQueComponenElActual)){
		osada_TA_borrarDirectorio(parent);
		printf("OSADA - TABLA DE ARCHIVOS: Se ha borrado el directorio %s de la tabla de archivos. El bloque borrado es %d\n",path,parent);
	}else{
		perror("NO se pudo remover el directorio porque no esta vacio");
		pudeBorrar=0;
	}
	list_destroy(directoriosQueComponenElActual);
	return pudeBorrar;
}

int osada_removeFile(char* path){
	u_int16_t parent=osada_TA_obtenerUltimoHijoFromPath(path);
	osada_TA_borrarArchivo(parent);
	printf("OSADA - TABLA DE ARCHIVOS: Pude borrar el archivo %s. Ocupaba el bloque %d\n", path, parent);
	return 1;
}
int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
	u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	//Obtengo los directorios
	osada_TA_obtenerDirectorios(parent, directorios);
	printf("OSADA - TABLA DE ARCHIVOS: Los directorios que contiene %s son: \n",path);
	int i;
	for (i=0;i<list_size(directorios);i++){
		osada_file* d = (osada_file*)list_get(directorios,i);
		printf("%s\n",d->fname);
	}
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

bool superaTamanioArchivo (int indice, off_t offset, size_t size){
	return size>(osada_drive.directorio[indice].file_size-offset);
}
bool elBufferTieneDatosParaEscribir(char* buf){
	return strlen(buf)>0;
}
void actualizarBuffer(char* buffer, char* bufUp, int bytesEscritos){
	int tamanioBufferOriginal=strlen(buffer);
	int i;
	int j=0;
	for (i=bytesEscritos;i<tamanioBufferOriginal;i++){
		bufUp[j]=buffer[i];
		j++;
	}
}

int actualizarBytesEscritos (int acum, int bytes){
	return acum+=bytes;
}
int osada_write(char* path,char* buf, size_t size, off_t offset){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	int bytesEscritos=0;
	if (!superaTamanioArchivo(indice,offset,size)){
		int bloque=osada_drive.directorio[indice].first_block;
		printf("OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n",path, bloque);
		double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
		int bloqueArranque=avanzarBloquesParaEscribir(bloque,desplazamientoHastaElBloque);
		printf("OSADA - TABLA DE ASIGNACIONES: Desde el bloque %d me desplace hasta el %d, me movi %f bloques.\n",bloque,bloqueArranque,desplazamientoHastaElBloque);;
		int byteComienzoEscritura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		printf("OSADA - DATOS: Empiezo a leer desde el byte: %d\n",byteComienzoEscritura);
		char* bufUpdated=string_new();
		strcpy(bufUpdated,buf);
		int bloquesQueNecesitoEscribir=ceil((strlen(buf)*sizeof(char))/OSADA_BLOCK_SIZE);
			if (hayBloquesDesocupadosEnElBitmap(bloquesQueNecesitoEscribir)){
				while (elBufferTieneDatosParaEscribir(bufUpdated)){
					printf("El contenido del buffer es %s\n",bufUpdated);
					bitarray_set_bit(osada_drive.bitmap,bloqueArranque);
					printf("OSADA - BITMAP: Marco al bloque %d como ocupado\n",bloqueArranque);
					memcpy(osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoEscritura],buf,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					printf("OSADA - DATOS: Los datos que voy a escribir son: %s\n",bufUpdated);
					actualizarBuffer(buf,bufUpdated,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					actualizarBytesEscritos(bytesEscritos,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					printf("OSADA - DATOS: Se han escrito %d bytes\n",bytesEscritos);
					byteComienzoEscritura=0;
					bloqueArranque=avanzarBloquesParaEscribir(bloqueArranque,1);
					printf("OSADA - TABLA DE ARCHIVOS: Avanzo al bloque %d\n",bloqueArranque);
			}
		}else{
			bytesEscritos=-ENOMEM;
	}
	}
	return bytesEscritos;
}
int osada_read(char *path, char *buf, size_t size, off_t offset){

	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	if (!superaTamanioArchivo(indice,offset,size)){
		//con el indice voy a TA y busco el FB
		int bloque=osada_drive.directorio[indice].first_block;
		printf("OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n", path, bloque);
		//offset/TAMBLQ= R ,rrdondearlo para arriba y restarle 1-->2
		double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
		printf("OSADA - TABLA DE ASIGNACIONES: Tengo que desplazarme %f bloques\n",desplazamientoHastaElBloque);
		//Voy a FB y avanzo 2 dentro de Tasignaciones
		int bloqueArranque=avanzarBloquesParaLeer(bloque,desplazamientoHastaElBloque);
		printf("OSADA - TABLA DE ASIGNACIONES: Comienzo a leer desde el bloque %d\n",bloqueArranque);
		//RDO=ofsset-(RxBSIZE)=cuando llegue al bloque solicitado hago *data (en declarations.h) y me muevo (se sumo) RDO
		int byteComienzoLectura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		printf("Empiezo a leer desde el byte %d\n",byteComienzoLectura);
		while (bloqueArranque!=0xFFFFFFFF){
			//falta chequear inicio
			memcpy(buf,osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoLectura],OSADA_BLOCK_SIZE-byteComienzoLectura);
			printf("OSADA - DATOS: Se leyo esta informacion: %s\n",buf);
			bloqueArranque=osada_drive.asignaciones[bloque];
			printf("OSADA - TABLA DE ASIGNACIONES: El bloque siguiente es: %d\n",bloqueArranque);
			byteComienzoLectura=0;
		}
		printf("OSADA - DATOS: Se han leido %d bytes\n", strlen(buf));
		return strlen(buf);
	}else{
		return -ENOMEM;
	}


}


int osada_open(char* path){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
	u_int16_t child = osada_TA_obtenerUltimoHijoFromPath(path);
	if(child>=0){
		printf("OSADA - TABLA DE ARCHIVOS: La funcion open encontro que el bloque ocupado por %s es %d\n",
				path,child);
		if(osada_drive.directorio[child].state ==2){
			return -EACCES;
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

int buscarLugarLibreEnBitmap(){
	int i,lugarLibre;
	for (i=0;i<osada_drive.bitmap->size;i++){
		if (!bitarray_test_bit(osada_drive.bitmap,i)){
			lugarLibre=i;
			i=osada_drive.bitmap->size;
		}
	}
	return lugarLibre;
}

bool hayPosicionDisponibleEnTablaDeArchivos (int pos){
	return osada_drive.directorio[pos].state==0;
}

void directoryContainingFile(char** pathVectorizado, char* directoryName){
	int i, ult=strlen(*pathVectorizado);
	for (i=0;i<ult-2;i++){
		string_append(&directoryName,pathVectorizado[i]);
	}

}
void generarNuevoArchivoEnTablaDeArchivos(char* path){
	time_t timer=time(0);
	struct tm *tlocal = localtime(&timer);
	char* fecha=string_new();
	char* fileName=string_new();
	char* directoryName=string_new();
	char** fileSplitteado=string_split(path,"/");
	directoryContainingFile(fileSplitteado, directoryName);
	fileName=strcpy(fileName,fileSplitteado[strlen(*fileSplitteado)-2]);
	printf("OSADA - Generacion nuevo archivo: El nombre del archivo es: %s\n",fileName);
	int bloqueInicioArchivo=buscarLugarLibreEnBitmap();
	printf("OSADA - BITMAP: El primer bloque libre es: %d\n",bloqueInicioArchivo);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].file_size=0;
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].first_block=bloqueInicioArchivo;
	strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].lastmod=atoi(fecha);
	printf("OSADA - TABLA DE ARCHIVOS: La fecha de modificacion es %d\n",osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].lastmod);
	strcpy((char*)osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].fname,fileName);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].parent_directory=osada_TA_obtenerUltimoHijoFromPath(directoryName);
	free(fecha);
	free(fileName);
	free(directoryName);
	free(*fileSplitteado);
	free(fileSplitteado);
}
int osada_createFile(char* path, mode_t mode){
	int resultado;
	if (buscarLugarLibreEnBitmap()>=0){
		printf("OSADA - BITMAP: Hay lugar libre para crear archivo\n");
		int posicionEnBitmap=buscarLugarLibreEnBitmap();
		if (hayPosicionDisponibleEnTablaDeArchivos(posicionEnBitmap)){
			printf("OSADA - TABLA DE ARCHIVOS: El archivo %s ocupara la posicion %d\n",path,posicionEnBitmap);
			generarNuevoArchivoEnTablaDeArchivos(path);
			resultado=1;
		}
	}else{
		resultado=0;
	}
	return resultado;
}
int osada_createDir(char* path, char* name){
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path);
	//aca hay que obtener el hijo del ultimo path/ parametro es el path
	darDeAltaDirectorioEnTablaDeArchivos(name, subindice);
	return 1;
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

int renombrarArchivo (int subindice, char* newFileName){
	int resultado;
	if (esUnArchivo(subindice)){
		if (strlen(strcpy((char*)osada_drive.directorio[subindice].fname,newFileName))==strlen(newFileName)){
			resultado= 1;
		}else{
			resultado= 0;
		}
	}else{
		if (estaBorrado(subindice)){
			resultado=ENOENT;
		}
		if (esUnDirectorio(subindice)){
			resultado=EISDIR;
		}
	}
	return resultado;

}
char* getFileNameFromPath(char* path, char** pathSplitteada, char* nombre){
	pathSplitteada=string_split(path,"/");
	nombre=pathSplitteada[strlen(*pathSplitteada)-2];
	return nombre;
}
int osada_rename(char* path, char* nuevaPath){
	int resultado;
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path);
	char* nombre=string_new();
	char** pathSplitteada=(char**)malloc(sizeof(char*));
	getFileNameFromPath(nuevaPath,pathSplitteada, nombre);
	printf("OSADA - Renombrando archivo: El nombre del archivo original es: %s\n",nombre);
	if (renombrarArchivo(subindice,nombre)==1){
		printf("OSADA - Renombrando archivo: Se ha renombrado el archivo correctamente\n");
		resultado= 1;
	}else{
		resultado=ENOMEM;
	}
	free(pathSplitteada);
	free(nombre);
	return resultado;
}

int calcularBloquesQueOcupaDesdeElPrimerBloque (int indice){
	int bloques=0;
	while (indice!=0xFFFFFF){
		bloques++;
		indice=osada_drive.asignaciones[indice];
	}
	return bloques;
}
void liberarEspacio (int sub,int bq_to_free){
	 int tamanioEnBloquesOriginal=calcularBloquesQueOcupaDesdeElPrimerBloque(sub);
	 int bloquesQueMeMovi=0;
	 int bloqueDesdeDondeEmpiezoALiberar;
	 int i;
	 while (tamanioEnBloquesOriginal-bq_to_free>bloquesQueMeMovi){
		 bloqueDesdeDondeEmpiezoALiberar=osada_drive.asignaciones[sub];
		 bloquesQueMeMovi++;
	 }
	 for (i=bloquesQueMeMovi;i<tamanioEnBloquesOriginal;i++){
		 bitarray_clean_bit(osada_drive.bitmap,osada_drive.asignaciones[bloqueDesdeDondeEmpiezoALiberar]);
		 osada_drive.directorio[i].state=0;
		 bloqueDesdeDondeEmpiezoALiberar=osada_drive.asignaciones[bloqueDesdeDondeEmpiezoALiberar];
	 }
}
int irAlUltimoBloqueDeLaTablaDeAsignaciones (int primerBloque){
	int indice=primerBloque;
	int ult;
	while (indice!=0xFFFFFF){
		ult=indice;
		indice=osada_drive.asignaciones[indice];
	}
	return ult;
}
void ocuparEspacio (int sub, int bq_to_set){
	int ultimoBloque=irAlUltimoBloqueDeLaTablaDeAsignaciones(sub);
	int i;
	if (hayBloquesDesocupadosEnElBitmap(bq_to_set)){
	for (i=0;i<bq_to_set;i++){
		int bloqueAOcupar=buscarLugarLibreEnBitmap();
		bitarray_set_bit(osada_drive.bitmap,bloqueAOcupar);
		osada_drive.asignaciones[ultimoBloque]=bloqueAOcupar;
	}
}
}
int osada_truncate(char* path, off_t offset){
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path);
	int resultado=0;
	int bloquesTruncate;
	if (osada_drive.directorio[subindice].file_size>offset){
		bloquesTruncate=ceil((osada_drive.directorio[subindice].file_size-offset)/OSADA_BLOCK_SIZE);
		liberarEspacio(subindice,bloquesTruncate);
		resultado=1;
		printf("OSADA - Truncate: Se han liberado %d bytes\n",(int)osada_drive.directorio[subindice].file_size-(int)offset);
	}else{
		bloquesTruncate=ceil((offset-osada_drive.directorio[subindice].file_size)/OSADA_BLOCK_SIZE);
			if (hayBloquesDesocupadosEnElBitmap(bloquesTruncate)){
				ocuparEspacio(subindice,offset-osada_drive.directorio[subindice].file_size);
				printf("OSADA - Truncate: Se han ocupado %d bytes\n",(int)offset-(int)osada_drive.directorio[subindice].file_size);
			}else{
				return -ENOMEM;
			}
	}
	return resultado;
}
void statfs(){

}
