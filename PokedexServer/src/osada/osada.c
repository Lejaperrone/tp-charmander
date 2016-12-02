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



int bloquesATruncar(int subindice, off_t offset){
	return ceil((osada_drive.directorio[subindice].file_size-offset)/OSADA_BLOCK_SIZE);
}

int hayBloquesDesocupadosEnElBitmap (int n){
	int i,bloquesNecesarios;
	int resultado=0;
	for (i=0;i<bitarray_get_max_bit(osada_drive.bitmap);i++){
		if(bloquesNecesarios==n){
			log_info(logPokedexServer, "OSADA - BITMAP: Hay al menos %d bloques desocupados\n",n);
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
void actualizarBuffer(char* buffer, char* bufUp, int bytesEscritos){
	int tamanioBufferOriginal=strlen(buffer);
	int i;
	int j=0;
	for (i=bytesEscritos;i<tamanioBufferOriginal;i++){
		bufUp[j]=buffer[i];
		j++;
	}
}

void actualizarBytesEscritos (int* acum, int bytes){
	*acum += bytes;
}
int osada_write(char* path,char* buf, size_t size, off_t offset){
	int resultadoDeBuscarRegistroPorNombre;
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path, &resultadoDeBuscarRegistroPorNombre);
	int bytesEscritos=0;

	if(resultadoDeBuscarRegistroPorNombre != -1){
		if (!superaTamanioArchivo(indice,offset,size)){
			int bloque=osada_drive.directorio[indice].first_block;
			log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n",path, bloque);
			double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
			int bloqueArranque=avanzarBloquesParaEscribir(bloque,desplazamientoHastaElBloque);
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Desde el bloque %d me desplace hasta el %d, me movi %f bloques.\n",bloque,bloqueArranque,desplazamientoHastaElBloque);;
			int byteComienzoEscritura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
			log_info(logPokedexServer, "OSADA - DATOS: Empiezo a leer desde el byte: %d\n",byteComienzoEscritura);
			char* bufUpdated=string_new();
			strcpy(bufUpdated,buf);
			int bloquesQueNecesitoEscribir=ceil((strlen(buf)*sizeof(char))/OSADA_BLOCK_SIZE);
			if (hayBloquesDesocupadosEnElBitmap(bloquesQueNecesitoEscribir)){
				while (elBufferTieneDatosParaEscribir(bufUpdated)){
					log_info(logPokedexServer, "El contenido del buffer es %s\n",bufUpdated);
					bitarray_set_bit(osada_drive.bitmap,bloqueArranque);
					log_info(logPokedexServer, "OSADA - BITMAP: Marco al bloque %d como ocupado\n",bloqueArranque);
					memcpy(osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoEscritura],buf,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					log_info(logPokedexServer, "OSADA - DATOS: Los datos que voy a escribir son: %s\n",bufUpdated);
					actualizarBuffer(buf,bufUpdated,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					actualizarBytesEscritos(&bytesEscritos,OSADA_BLOCK_SIZE-byteComienzoEscritura);
					log_info(logPokedexServer, "OSADA - DATOS: Se han escrito %d bytes\n",bytesEscritos);
					byteComienzoEscritura=0;
					bloqueArranque=avanzarBloquesParaEscribir(bloqueArranque,1);
					log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: Avanzo al bloque %d\n",bloqueArranque);
				}
			}else{
				bytesEscritos=-ENOMEM;
			}
		}
		else{
			return -EFBIG;
		}
	}else{
		return -ENOENT;
	}
	return bytesEscritos;
}
int osada_read(char *path, char* buf, size_t size, off_t offset){

	int indice = osada_TA_obtenerIndiceTA(path);

	if (!superaTamanioArchivo(indice,offset,size) && indice != -1){
		//con el indice voy a TA y busco el FB
		int bloque=osada_drive.directorio[indice].first_block;
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El primer bloque de %s es: %d\n", path, bloque);
		//offset/TAMBLQ= R ,rrdondearlo para arriba y restarle 1-->2
		double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Tengo que desplazarme %f bloques\n",desplazamientoHastaElBloque);
		//Voy a FB y avanzo 2 dentro de Tasignaciones
		int bloqueArranque=avanzarBloquesParaLeer(bloque,desplazamientoHastaElBloque);
		log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: Comienzo a leer desde el bloque %d\n",bloqueArranque);
		//RDO=ofsset-(RxBSIZE)=cuando llegue al bloque solicitado hago *data (en declarations.h) y me muevo (se sumo) RDO
		int byteComienzoLectura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		log_info(logPokedexServer, "Empiezo a leer desde el byte %d\n",byteComienzoLectura);
		while (bloqueArranque!=0xFFFFFFFF){
			//falta chequear inicio
			memcpy(buf,osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoLectura],OSADA_BLOCK_SIZE-byteComienzoLectura);
			log_info(logPokedexServer, "OSADA - DATOS: Se leyo esta informacion: %s\n",buf);
			bloqueArranque=osada_drive.asignaciones[bloque];
			log_info(logPokedexServer, "OSADA - TABLA DE ASIGNACIONES: El bloque siguiente es: %d\n",bloqueArranque);
			byteComienzoLectura=0;
		}
		log_info(logPokedexServer, "OSADA - DATOS: Se han leido %d bytes\n", string_length(buf));

		return 1;
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
	int resultadoDeBuscarRegistroPorNombre;
	time_t timer=time(0);
	struct tm *tlocal = localtime(&timer);
	char* fecha=string_new();
	char* fileName=string_new();
	char* directoryName=string_new();
	char** fileSplitteado=string_split(path,"/");
	directoryContainingFile(fileSplitteado, directoryName);
	fileName=strcpy(fileName,fileSplitteado[strlen(*fileSplitteado)-2]);
	log_info(logPokedexServer, "OSADA - Generacion nuevo archivo: El nombre del archivo es: %s\n",fileName);
	int bloqueInicioArchivo=buscarLugarLibreEnBitmap();
	log_info(logPokedexServer, "OSADA - BITMAP: El primer bloque libre es: %d\n",bloqueInicioArchivo);
	pthread_mutex_lock(&mutexTablaArchivos);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].file_size=0;
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].first_block=bloqueInicioArchivo;
	strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].lastmod=atoi(fecha);
	log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: La fecha de modificacion es %d\n",osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].lastmod);
	strcpy((char*)osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].fname,fileName);
	osada_drive.directorio[bloqueInicioArchivo*OSADA_BLOCK_SIZE].parent_directory=osada_TA_obtenerUltimoHijoFromPath(directoryName, &resultadoDeBuscarRegistroPorNombre);
	free(fecha);
	free(fileName);
	free(directoryName);
	free(*fileSplitteado);
	free(fileSplitteado);
	pthread_mutex_unlock(&mutexTablaArchivos);
}
int osada_createFile(char* path, mode_t mode){
	int resultado;

	if (buscarLugarLibreEnBitmap()>=0){
		log_info(logPokedexServer, "OSADA - BITMAP: Hay lugar libre para crear archivo\n");
		int posicionEnBitmap=buscarLugarLibreEnBitmap();
		log_info(logPokedexServer, "OSADA - TABLA DE ARCHIVOS: El archivo %s ocupara la posicion %d\n",path,posicionEnBitmap);
		generarNuevoArchivoEnTablaDeArchivos(path);
		resultado = 1;
	}else{
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
	//aca hay que obtener el hijo del ultimo path/ parametro es el path
	if(subindice != -1){
		if(darDeAltaDirectorioEnTablaDeArchivos(name, subindice)){
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

int renombrarArchivo (int subindice, char* newFileName){
	int resultado;
	if (esUnArchivo(subindice)){
		if (strlen(strcpy((char*)osada_drive.directorio[subindice].fname,newFileName))==strlen(newFileName)){
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
				log_info(logPokedexServer,"Se ha reemplazado el nombre del directorio por %s",newFileName);
						resultado= 1;
					}else{
						resultado= 0;
					}
		}
	}
	return resultado;

}
void getFileNameFromPath(char* path,  char** nombre){
	char** pathSplitteada=string_split(path,"/");
	//*nombre=pathSplitteada[strlen(*pathSplitteada)-1];
	int j=0;
	while(j!=-1){
		if(pathSplitteada[j+1]==NULL){
			string_append(nombre,pathSplitteada[j]);
			j=-1;
		}else{
			j++;
		}
	}
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
	free(pathSplitteada);
}
//LISTA - FUNCIONA
int osada_rename(char* path, char* nuevaPath){
	int resultado;
	int subindice=osada_TA_obtenerIndiceTA(path);
	log_info(logPokedexServer,"El subindice es %d",subindice);
	if(subindice != -1){
			char* nombre=string_new();
			getFileNameFromPath(nuevaPath, &nombre);
			log_info(logPokedexServer, "OSADA - Renombrando archivo: El nombre del archivo nuevo es: %s\n",nombre);
			if (renombrarArchivo(subindice,nombre)==1){
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


int calcularBloquesQueOcupaDesdeElPrimerBloque (int indice){
	int bloques=0;
	pthread_mutex_lock(&mutexTablaAsignaciones);
	while (indice!=0xFFFFFF){
		bloques++;
		indice=osada_drive.asignaciones[indice];
	}
	pthread_mutex_unlock(&mutexTablaAsignaciones);
	return bloques;
}
void liberarEspacio (int sub,int bq_to_free){

	 int tamanioEnBloquesOriginal=calcularBloquesQueOcupaDesdeElPrimerBloque(sub);
	 int bloquesQueMeMovi=0;
	 int bloqueDesdeDondeEmpiezoALiberar;
	 int i;
	 pthread_mutex_lock(&mutexTablaAsignaciones);
	 while (tamanioEnBloquesOriginal-bq_to_free>bloquesQueMeMovi){
		 bloqueDesdeDondeEmpiezoALiberar=osada_drive.asignaciones[sub];
		 bloquesQueMeMovi++;
	 }
	 pthread_mutex_unlock(&mutexTablaAsignaciones);
	 pthread_mutex_lock(&mutexBitmap);
	 pthread_mutex_lock(&mutexTablaArchivos);
	 pthread_mutex_lock(&mutexTablaAsignaciones);
	 for (i=bloquesQueMeMovi;i<tamanioEnBloquesOriginal;i++){
		 bitarray_clean_bit(osada_drive.bitmap,osada_drive.asignaciones[bloqueDesdeDondeEmpiezoALiberar]);
		 osada_drive.directorio[i].state=0;
		 bloqueDesdeDondeEmpiezoALiberar=osada_drive.asignaciones[bloqueDesdeDondeEmpiezoALiberar];
	 }
	 pthread_mutex_unlock(&mutexBitmap);
	 	 pthread_mutex_unlock(&mutexTablaArchivos);
	 	 pthread_mutex_unlock(&mutexTablaAsignaciones);
}
int irAlUltimoBloqueDeLaTablaDeAsignaciones (int primerBloque){
	int indice=primerBloque;
	int ult;
	while (indice!=0xFFFFFF){
		ult=indice;
		obtenerProximoBloque(&indice);
	}
	return ult;
}
void ocuparEspacio (int sub, int bq_to_set){
	int ultimoBloque=irAlUltimoBloqueDeLaTablaDeAsignaciones(sub);
	int i;
	pthread_mutex_lock(&mutexBitmap);
	if (hayBloquesDesocupadosEnElBitmap(bq_to_set)){
		pthread_mutex_lock(&mutexBitmap);
		pthread_mutex_lock(&mutexTablaAsignaciones);
		for (i=0;i<bq_to_set;i++){
			int bloqueAOcupar=buscarLugarLibreEnBitmap();
			bitarray_set_bit(osada_drive.bitmap,bloqueAOcupar);
			ocuparBloqueSegunElUltimo(ultimoBloque,bloqueAOcupar);
		}
		pthread_mutex_unlock(&mutexTablaAsignaciones);
		pthread_mutex_unlock(&mutexBitmap);
		pthread_mutex_lock(&mutexBitmap);
	}
}

int osada_truncate(char* path, off_t offset){
	int resultadoDeBuscarRegistroPorNombre;
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path, &resultadoDeBuscarRegistroPorNombre);
	int bloquesTruncate;
	if(resultadoDeBuscarRegistroPorNombre != -1){
		if (osada_drive.directorio[subindice].file_size>offset){
			bloquesTruncate=bloquesATruncar(subindice,offset);
			liberarEspacio(subindice,bloquesTruncate);
			return 1;
			log_info(logPokedexServer, "OSADA - Truncate: Se han liberado %d bytes\n",(int)osada_drive.directorio[subindice].file_size-(int)offset);
		}else if(osada_drive.directorio[subindice].file_size<offset){
			bloquesTruncate=bloquesATruncar(subindice,offset);
			if (hayBloquesDesocupadosEnElBitmap(bloquesTruncate)){
				ocuparEspacio(subindice,offset-osada_drive.directorio[subindice].file_size);
				log_info(logPokedexServer, "OSADA - Truncate: Se han ocupado %d bytes\n",(int)offset-(int)osada_drive.directorio[subindice].file_size);
				return 1;
			}else{
				return -ENOMEM;
			}
		}else{
			return -EPERM;
		}
	}
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
