/*
 * fuse.c
 *
 *  Created on: 1/11/2016
 *      Author: utnso
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <tad_items.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stddef.h>
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "../commons/structures.h"

int sendValue(void* parameter, int size){
	char* sizeStr=malloc(sizeof(char)*11);

	sprintf(sizeStr,"%i",size);
	if(send(pokedexServer, sizeStr, 11, 0) == 11){
		if(send(pokedexServer, parameter, size, 0) == size){
			free(sizeStr);
			return 1;
		}
	}
	free(sizeStr);
	return 0;
}

int sendPrimerByteBuffer(char* buffer, int size){
	char* bufferDe1Byte = malloc(1);

	memcpy(bufferDe1Byte, buffer, 1);

	send(pokedexServer, bufferDe1Byte, 1, 0);

	free(bufferDe1Byte);
	return 0;
}
int sendBufferParaElWrite(char* bufferLlenoDeFuse, int size){

	if(size <= 32768){
		log_info(archivoLog, "IMPORTANTE - enviarBuffer - Envio al servidor la cant de bytes para el buffer: %d", size);
		send(pokedexServer, bufferLlenoDeFuse, size, 0);
		return 1;
	}
	else{
		div_t divisionEnPartes = div(size, 32768);
		int vecesQueHayQueHacerSend = divisionEnPartes.quot;

		int i;
		int desplazam = 0;
		for(i=0 ; i<vecesQueHayQueHacerSend; i++){
			char* bufferDe32kb = malloc(32768*sizeof(char));
			memcpy(bufferDe32kb, bufferLlenoDeFuse+desplazam, 32768);
			desplazam += 32768;
			log_info(archivoLog, "IMPORTANTE - enviarBuffer - La cant de veces que voy a enviar 32768 bytes al servidor es: %d", vecesQueHayQueHacerSend);
			send(pokedexServer, bufferDe32kb, 32768, 0);
			free(bufferDe32kb);
		}
		if(divisionEnPartes.rem > 0){
			char* bufferDelRestoDeBytes = malloc((divisionEnPartes.rem)*sizeof(char));
			memcpy(bufferDelRestoDeBytes, bufferLlenoDeFuse+desplazam, divisionEnPartes.rem);
			log_info(archivoLog, "IMPORTANTE - enviarBuffer - Envio al servidor la cant de bytes para el buffer: %d", divisionEnPartes.rem);
			send(pokedexServer, bufferLlenoDeFuse+desplazam, divisionEnPartes.rem, 0);
			free(bufferDelRestoDeBytes);
		}
		return 1;
	}
	return 1;
}

int recibirBuffer(char* buffer, int bytesLeidosEnOsada){

//	div_t divisionEnPartes = div(bytesLeidosEnOsada, 32768);
//	int vecesQueHayQueHacerRecv = divisionEnPartes.quot;
//
//	char* bufferAuxiliar = malloc(bytesLeidosEnOsada);
//	if(bytesLeidosEnOsada <= 32768){
//		recv(pokedexServer, bufferAuxiliar, bytesLeidosEnOsada, 0);
//		log_info(archivoLog, "IMPORTANTE - recibirBuffer - Recibo del servidor la cant de bytes: %d", bytesLeidosEnOsada);
//		memcpy(buffer, bufferAuxiliar, bytesLeidosEnOsada);
//		return 1;
//	}
//	else{
//		int i;
//		int desplazam = 0;
//		for(i=0 ; i<vecesQueHayQueHacerRecv; i++){
//			char* bufferDe32kb = malloc(32768*sizeof(char));
//			log_info(archivoLog, "IMPORTANTE - recibirBuffer - Las veces que hay que hacer recvs de 32768 es: %d",vecesQueHayQueHacerRecv);
//			recv(pokedexServer, bufferDe32kb, 32768, 0);
//			memcpy(bufferAuxiliar+desplazam, bufferDe32kb, 32768);
//			desplazam += 32768;
//
//			free(bufferDe32kb);
//		}
//		if(divisionEnPartes.rem > 0){
//			char* bufferDelRestoDeBytes = malloc((divisionEnPartes.rem)*sizeof(char));
//			log_info(archivoLog, "IMPORTANTE - recibirBuffer - Recibo del servidor la cant de bytes: %d", divisionEnPartes.rem);
//			recv(pokedexServer, bufferDelRestoDeBytes, divisionEnPartes.rem, 0);
//			memcpy(bufferAuxiliar+desplazam, bufferDelRestoDeBytes, divisionEnPartes.rem);
//			free(bufferDelRestoDeBytes);
//		}
//		memcpy(buffer, bufferAuxiliar, bytesLeidosEnOsada);
//	}
//
//	log_info(archivoLog,"----------IMPRIMO EL BUFFER RECIBIDO----------");
//			int i;
//			for (i=0; i<bytesLeidosEnOsada; i++){
//				log_info(archivoLog,"%d", buffer[i]);
//			}
//
//	return 1;

	recv(pokedexServer, buffer, bytesLeidosEnOsada, MSG_WAITALL);

	return 1;
}
int recvValue(char* buffer){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(pokedexServer, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		log_info(archivoLog,"Size to recv: %d", size);
//		char* temp=malloc(sizeof(char)*17);
//		if (recv(pokedexServer, temp, size,  0) == size){
//			buffer = string_substring(temp, 0, size);
//			return 1;
//		}
		char* bufferRecibido = malloc(size);
		if(recv(pokedexServer, bufferRecibido, size, 0) == size){
			memcpy(buffer,bufferRecibido,size);
			free(bufferRecibido);
			return 1;
		}
	}
	return 0;
}
int recvString(char** string){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(pokedexServer, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		char* temp=malloc(sizeof(char)*size);
		if (recv(pokedexServer, temp, size,  0) == size){
			*string = string_substring(temp,0,size);
			free(temp);
			return 1;
		}else{
			free(temp);
		}
	}else{
		free(sizeStr);
	}

	return 0;
}
void sendBasicInfo(char* function, const char* path){
	send(pokedexServer, function, 5,0);
	sendValue((char*)path, string_length((char*)path));
}
int recvInt(){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(pokedexServer, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		return size;
	}
	free(sizeStr);
	return -ENOENT;
}

//LISTA - FUNCIONA
int chamba_getattr(char* path, struct stat* stbuf){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: GETATTR");
	log_info(archivoLog, "2 - Path: %s", path);

	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));

	sendBasicInfo("GETAT", path);
	int resultadoOsada = recvInt();
	log_info(archivoLog, "3 - resultadoOsada: %d", resultadoOsada);

	if (resultadoOsada==1){
		int tipoDeArchivo = recvInt();
		log_info(archivoLog, "4 - tipoDeArchivo: %d", tipoDeArchivo);

		if (tipoDeArchivo==2){
			stbuf->st_size = recvInt();
			recv(pokedexServer, &(stbuf->st_atim), sizeof(time_t), 0);
			stbuf->st_ctim = stbuf->st_atim;
			stbuf->st_mtim = stbuf->st_atim;
			log_info(archivoLog, "5 - stbuf->st_size: %d", stbuf->st_size);
			stbuf->st_mode=S_IFDIR | 0755;
			stbuf->st_nlink = 2;
		}else if (tipoDeArchivo==1){
			stbuf->st_size = recvInt();
			recv(pokedexServer, &(stbuf->st_atim), sizeof(time_t), 0);
			stbuf->st_ctim = stbuf->st_atim;
			stbuf->st_mtim = stbuf->st_atim;
			log_info(archivoLog, "6 - stbuf->st_size: %d", stbuf->st_size);
			stbuf->st_mode=S_IFREG | 0444;
			stbuf->st_nlink = 1;
		}else{
			res=-ENOENT;
		}
	}else{
		res = -ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: READDIR");
	log_info(archivoLog, "2 - Path: %s", path);

	int res =0;
	sendBasicInfo("READD", path);
	int resultadoOsada = recvInt();
	log_info(archivoLog, "3 - resultadoOsada: %d", resultadoOsada);

	if(resultadoOsada == 1){
		int cant = recvInt();
		log_info(archivoLog, "4 - cant: %d", cant);
		int i;
		for(i=0;i<cant;i++){
			char* directorio = malloc(sizeof(char)*17);
			if(recvString(&directorio)==0){
				log_info(archivoLog, "5 - No obtuve valor");
			}
			log_info(archivoLog, "6 - Directorio: %s", directorio);
			filler(buf, directorio, NULL, 0);
			free(directorio);
		}
	}
	else{
		res = -ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}
/*
void recvBasicInfo(int* resultadoOsada, char* nombreFuncion, char* path){
	recv(pokedexServer,resultadoOsada, sizeof(int), 0);
	log_info(archivoLog, "La respuesta recibida (int) desde osada para %s con el path %s es: %d", nombreFuncion, path, *resultadoOsada);
}

void sendNuevoPath(const char* newPath){
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)newPath));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, newPath, string_length((char*)newPath), 0);
	log_info(archivoLog,"PokedexCliente: La nueva path va a ser %s",newPath);
}

void sendSize(size_t size){
	send(pokedexServer, &size, sizeof(size_t), 0);
	log_info(archivoLog,"FUSE: Envie el size_t: %d",size);
}

void sendOffset(off_t offset){
	send(pokedexServer, &offset, sizeof(off_t), 0);
	log_info(archivoLog,"FUSE: Envie el off_t: %d",offset);
}

void sendMode(mode_t mode){
	send(pokedexServer, &mode, sizeof(mode_t), 0);
	//log_info(archivoLog,"FUSE: Envie el mode_t: %d",mode);
}

void enviarNombreDeLaFuncion(char* nom){
	if (send(pokedexServer,nom,5*sizeof(char),0) >0){
	log_info(archivoLog,"FUSE: Envie %s al servidor",nom);
	}
	else{
		log_info(archivoLog, "FUSE: No se pudo enviar el nombre de la funcion");
	}
}

void enviarTamanioDelPath(const char* path){
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer,sizePath,11,0);
	log_info(archivoLog,"FUSE: Envie %d al servidor",strlen(path));
}
void enviarPath(const char* path){
	send(pokedexServer,path,strlen(path),0);
	log_info(archivoLog,"FUSE: Envie %s al servidor",path);
}
void enviarBuffer(struct stat* stbuf){
	send(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
	log_info(archivoLog,"FUSE: Envie el primer parametro de stbuf");
}
void recibirBufferCompleto (struct stat* stbuf){
	recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
	log_info(archivoLog,"FUSE: Recibo el primer parametro de stbuf");
}
*/

//LISTA - FUNCIONA (era necesaria para el chamba_read)
int chamba_flush(const char* path, struct fuse_file_info* fi){
	return 0;
}

//LISTA - FUNCIONA (era necesaria para el seteo de la fecha de modificacion del createFile)
int chamba_utimens(const char *path, const struct timespec tv[2]){
	return 0;
}

int chamba_chown (const char *filename, uid_t owner, gid_t group){
	return 0;
}

//LISTA - FUNCIONA
int chamba_open (const char * path, struct fuse_file_info * fi){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: OPEN");
	log_info(archivoLog, "2 - Path: %s", path);

	int res = -ENOENT;
	sendBasicInfo("OPENF", path);

	int resultadoOsada=recvInt();

	if(resultadoOsada == 1){
		res = 0;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA (reveer la reproduccion del archivo mp4)
int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: READF");
	log_info(archivoLog, "2 - Path: %s", path);

	int res = 0;
	sendBasicInfo("READF", path);

	log_info(archivoLog, "El size_t que le llega a FUSE es: %d", size);
	log_info(archivoLog, "El off_t que le llega a FUSE es: %d", offset);
	sendValue(&size, sizeof(size_t));
	sendValue(&offset, sizeof(off_t));

	int resultadoOsada=recvInt();
	log_info(archivoLog,"Recibo como resultadoOsada: %d",resultadoOsada);


	if(resultadoOsada >0){
//		char* bufAlternativo=malloc(resultadoOsada);
//		recvValue(bufAlternativo);
		recibirBuffer(buffer, resultadoOsada);

		res = resultadoOsada;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: CREATE");
	log_info(archivoLog, "2 - Path: %s", path);

	sendBasicInfo("CREAT", path);
//	sendValue(&mode, sizeof(mode_t));

	int res= -ENOENT;
	int resultadoOsada = recvInt();
	if(resultadoOsada == 1){
		res = 0;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//CUASI LISTA (hay que ver por que a veces rompe cuando offset>tamanioArchivo)
int chamba_truncate (const char * path, off_t offset){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: TRUNCATE");
	log_info(archivoLog, "2 - Path: %s", path);

	int res=0;
	sendBasicInfo("TRUNC", path);
	sendValue(&offset, sizeof(off_t));
	log_info(archivoLog, "Le mando al servidor el offset: %d", offset);

	int resultadoOsada = recvInt();

	if(resultadoOsada != 1){
		res = -1;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

// LISTA - FUNCIONA
int chamba_mkdir (const char * path, mode_t modo){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: MKDIR");
	log_info(archivoLog, "2 - Path: %s", path);

	int res=0;
	sendBasicInfo("MKDIR", path);

	int resultadoOsada=recvInt();
	log_info(archivoLog, "Para MKDIR se recibe como resultadoOsada: %d", resultadoOsada);

	if (resultadoOsada != 1){
		 res=- ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_rename (const char * path, const char * newPath){
	int res=0;
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: RENAME");
	log_info(archivoLog, "2 - Path: %s", path);
	log_info(archivoLog, "3 - El nuevo path: %s", newPath);

	sendBasicInfo("RENAM", path);

	sendValue((char*)newPath,string_length((char*)newPath));

	int resultadoOsada=recvInt();

	if(resultadoOsada != 1){
		res = -ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_unlink (const char * path){
	int res=0;
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: UNLINK");
	log_info(archivoLog, "2 - Path: %s", path);


	sendBasicInfo("ULINK", path);
	int resultadoOsada = recvInt();

	if(resultadoOsada != 1){
		res = -ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_rmdir (const char * path){
	int res=0;
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: RMDIR");
	log_info(archivoLog, "2 - Path: %s", path);

	sendBasicInfo("RMDIR", path);
	int resultadoOsada = recvInt();

	if(resultadoOsada != 1){
		res = -ENOENT;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}

int chamba_write (const char*  path, const char*  buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: WRITE");
	log_info(archivoLog, "2 - Path: %s", path);
	int res = -ENOENT;
	sendBasicInfo("WRITE", path);

	log_info(archivoLog, "El size_t que le llega a FUSE es: %d", size);
	log_info(archivoLog, "El off_t que le llega a FUSE es: %d", offset);

	int cantBytesDelPrimerBuffer = 1;

	sendValue(&cantBytesDelPrimerBuffer, sizeof(size_t));
	sendValue(&offset, sizeof(off_t));
//	log_info(archivoLog,"----------IMPRIMO EL BUFFER ENVIADO----------");
//	int i;
//	for (i=0; i<size; i++){
//		log_info(archivoLog,"%d", buffer[i]);
//	}
//	sendValue((char*)buffer,size);

	log_info(archivoLog,"----------IMPRIMO EL 1er BYTE DEL BUFFER QUE VOY A ENVIAR----------");
	int i;
	for (i=0; i<1; i++){
		log_info(archivoLog,"%d", buffer[i]);
	}
	sendPrimerByteBuffer((char*)buffer, size);


	int primerResultadoOsada = recvInt();
	log_info(archivoLog, "El primerResultadoOsada es: %d", primerResultadoOsada);

	if(primerResultadoOsada > 0 && cantBytesDelPrimerBuffer == primerResultadoOsada){
		log_info(archivoLog, "El 1er byte se escribio bien.");

		sendBasicInfo("WRITE", path);

		int elRestoDeBytes = size-1;
		int offSetDeUno = offset+1;
		sendValue(&elRestoDeBytes, sizeof(size_t));
		sendValue(&offSetDeUno, sizeof(off_t));

		char* elRestoDelBuffer = malloc(size - 1);
		memcpy(elRestoDelBuffer, buffer+1, size -1);
		sendBufferParaElWrite(elRestoDelBuffer, size - 1);

		int segundoResultadoOsada = recvInt();
		log_info(archivoLog, "El segundoResultadoOsada es: %d", segundoResultadoOsada);
		int cantBytesDelSegundoBuffer = size - 1;
		if(segundoResultadoOsada > 0 && cantBytesDelSegundoBuffer == segundoResultadoOsada){
			res = primerResultadoOsada + segundoResultadoOsada;
		}else{
			res = primerResultadoOsada;
		}
	}


	pthread_mutex_unlock(&mutexSocket);
	return res;
}

//LISTA - FUNCIONA
int chamba_statfs (const char * path, struct statvfs * stats){
	pthread_mutex_lock(&mutexSocket);
	log_info(archivoLog, "1 - Funcion: STATFS");
	log_info(archivoLog, "2 - Path: %s", path);
	sendBasicInfo("STATF", path);
	int resultadoOsada = recvInt();

	int res=-ENOENT;

	if(resultadoOsada == 1){
		memset(stats, 0, sizeof(struct statvfs));
		int bloquesLibres = recvInt();
		int bloqueSize = recvInt();
		int bloquesTotales = recvInt();
		int filesLibres = recvInt();
		int filenameLength = recvInt();

		//stats->__f_spare;
		//stats->__f_unused;
		stats->f_bavail = bloquesLibres;
		stats->f_bfree = bloquesLibres;
		stats->f_blocks = bloquesTotales;
		stats->f_bsize = bloqueSize;
		stats->f_favail = filesLibres;
		stats->f_ffree = filesLibres;
		stats->f_files = 2048;
		//stats->f_flag;
		//stats->f_frsize;
		//stats->f_fsid;
		stats->f_namemax = filenameLength;
		res=0;
	}

	pthread_mutex_unlock(&mutexSocket);
	return res;
}
