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
#include "../commons/structures.h"

void sendBasicInfo(char* function, const char* path){
	send(pokedexServer, function, 5,0);
	log_info(archivoLog,"PokedexCliente: Envio %s",function);
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, path, string_length((char*)path), 0);
	log_info(archivoLog,"PokedexCliente: La path para la funcion %s es %s",function,path);
}


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

int chamba_getattr(char* path, struct stat* stbuf){
	int res = 0;
	int resultadoOsada;
	sendBasicInfo("GETAT", path);
	recvBasicInfo(&resultadoOsada, "GETAT", path);

	memset(stbuf, 0, sizeof(struct stat));

	if (resultadoOsada==1){
		int tipoDeArchivo;
		recv(pokedexServer, &tipoDeArchivo,sizeof(int),0); //Recibo el tipo del archivo reconocido por osada
		log_info(archivoLog,"PokedexCliente: El tipo de archivo reconocido por osada_getattr para GETAT con el path %s es %d", path, tipoDeArchivo);

		//Si es un directorio
		if (tipoDeArchivo==2){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			stbuf->st_mode=S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			log_info(archivoLog,"PokedexCliente: El peso del archivo del path %s es %d", path, stbuf->st_size);
		}
		//Si es un archivo regular
		else if (tipoDeArchivo==1){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			log_info(archivoLog,"PokedexCliente: El peso del archivo del path %s es %d", path, stbuf->st_size);
			stbuf->st_mode=S_IFREG | 0444;
			stbuf->st_nlink = 1;
		}
		//Si es un archivo DELETED (el tipo es 0)
		else{
			res=-ENOENT;
		}
	}

	return res;
}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	int tamanio;
	int resultadoOsada;

	sendBasicInfo("READD", path);
	recvBasicInfo(&resultadoOsada, "READD", (char*)path);

	if(resultadoOsada != -1){
		recv(pokedexServer,&tamanio,sizeof(int),0);
		log_info(archivoLog,"Voy a recibir %d archivos del directorio %s",tamanio,path);
		recv(pokedexServer,buf,tamanio,0);
		log_info(archivoLog,"Recibi el buf: %s", buf);
	}
	else{
		return -ENOENT;
	}

	return resultadoOsada;
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	int resultadoOsada;
	sendBasicInfo("OPENF", path);
	recvBasicInfo(&resultadoOsada, "OPENF", (char*)path);


	return resultadoOsada; //Desde el servidor ya me devuelve un 1 o un ENOENT
}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	int resultadoOsada;
	int tamanio;
	sendBasicInfo("READF", path);

	sendSize(size);
	sendOffset(offset);

	recvBasicInfo(&resultadoOsada, "READF", (char*)path);

	if(resultadoOsada == 1){
		recv(pokedexServer,&tamanio,sizeof(int),0);
		log_info(archivoLog,"Voy a leer %d bytes del path %s",tamanio,path);
		recv(pokedexServer,buffer,tamanio,0);
		log_info(archivoLog,"Lei el buffer: %s", buffer);
	}

	return resultadoOsada;
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){
	int resultadoOsada;

	sendBasicInfo("CREAT", path);
	sendMode(mode);

	recvBasicInfo(&resultadoOsada, "CREAT", (char*)path);


	return resultadoOsada;
}

int chamba_truncate (const char * path, off_t offset){
	int resultadoOsada;

	sendBasicInfo("TRUNC", path);
	sendOffset(offset);

	recvBasicInfo(&resultadoOsada, "TRUNC", (char*)path);

	return resultadoOsada;
}

int chamba_mkdir (const char * path, mode_t modo){
	int resultadoOsada;

	sendBasicInfo("MKDIR", path);
	sendMode(modo);

	recvBasicInfo(&resultadoOsada, "MKDIR", (char*)path);

	return resultadoOsada;
}

int chamba_rename (const char * path, const char * newPath){

	int resultadoOsada;

	sendBasicInfo("RENAM", path);
	sendNuevoPath(newPath);

	recvBasicInfo(&resultadoOsada, "RENAM", (char*)path);

	return resultadoOsada;
}

int chamba_unlink (const char * path){
	int resultadoOsada;

	sendBasicInfo("ULINK", path);
	recvBasicInfo(&resultadoOsada, "ULINK", (char*)path);

	return resultadoOsada;

}

int chamba_rmdir (const char * path){

	int resultadoOsada;

	sendBasicInfo("RMDIR", path);

	recvBasicInfo(&resultadoOsada, "RMDIR", (char*)path);

	return resultadoOsada;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	int resultadoOsada;
	int tamanio;

	sendBasicInfo("WRITE", path);
	sendSize(size);
	sendOffset(offset);

	recvBasicInfo(&resultadoOsada, "WRITE", (char*)path);

	if(resultadoOsada > 0){
		recv(pokedexServer,&tamanio,sizeof(int),0);
		log_info(archivoLog,"Voy a escribir %d bytes al path %s",tamanio,path);
		recv(pokedexServer,(char*)buffer,tamanio,0);
		log_info(archivoLog,"Escribi en el buffer: %s", buffer);
	}

	return resultadoOsada;
}

int chamba_statfs (const char * path, struct statvfs * stats){
	int resultadoOsada;

	sendBasicInfo("STATF", path);

	recvBasicInfo(&resultadoOsada, "STATF", (char*)path);

	if(resultadoOsada == 1){
		recv(pokedexServer,&(stats->__f_spare),sizeof(int),0);
		recv(pokedexServer,&(stats->f_bavail),sizeof(__fsblkcnt_t),0);
		recv(pokedexServer,&(stats->f_bfree),sizeof(__fsblkcnt_t),0);
		recv(pokedexServer,&(stats->f_blocks),sizeof(__fsblkcnt_t),0);
		recv(pokedexServer,&(stats->f_bsize),sizeof(unsigned long int),0);
		recv(pokedexServer,&(stats->f_favail),sizeof(__fsfilcnt_t),0);
		recv(pokedexServer,&(stats->f_ffree),sizeof(__fsfilcnt_t),0);
		recv(pokedexServer,&(stats->f_files),sizeof(__fsfilcnt_t),0);
		recv(pokedexServer,&(stats->f_flag),sizeof(unsigned long int),0);
		recv(pokedexServer,&(stats->f_frsize),sizeof(unsigned long int),0);
		recv(pokedexServer,&(stats->f_fsid),sizeof(unsigned long int),0);
		recv(pokedexServer,&(stats->f_namemax),sizeof(unsigned long int),0);
	}

	return resultadoOsada;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
