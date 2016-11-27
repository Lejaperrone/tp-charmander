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
	log_info(archivoLog,"FUSE - Envio %s",function);
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, path, string_length((char*)path), 0);
	log_info(archivoLog,"FUSE - La path es %s",path);
}


void recvBasicInfo(int* resultadoOsada){
	recv(pokedexServer,resultadoOsada, sizeof(int), 0);
	log_info(archivoLog, "La respuesta recibida (int) desde osada es: ", *resultadoOsada);
}

void sendOffset(off_t offset){
	send(pokedexServer, &offset, sizeof(off_t), 0);
	log_info(archivoLog, "FUSE - Envio el offset %d", offset);
}

void sendMode(mode_t mode){
	send(pokedexServer, &mode, sizeof(mode_t), 0);
	log_info(archivoLog, "FUSE - Envio el offset %d", mode);
}

void sendTamanioYContenidoNewPath(char* newPath){
	char* sizePath = malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)newPath));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, newPath, string_length((char*)newPath), 0);
	log_info(archivoLog,"FUSE - La newPath es %s",newPath);
}

void recvCadaElementoDeLaEstructura(struct statvfs * stats){
	recv(pokedexServer,&(stats->__f_spare),sizeof(stats->__f_spare),0);
	recv(pokedexServer,&(stats->f_bavail),sizeof(stats->f_bavail),0);
	recv(pokedexServer,&(stats->f_bfree),sizeof(stats->f_bfree),0);
	recv(pokedexServer,&(stats->f_blocks),sizeof(stats->f_blocks),0);
	recv(pokedexServer,&(stats->f_bsize),sizeof(stats->f_bsize),0);
	recv(pokedexServer,&(stats->f_favail),sizeof(stats->f_favail),0);
	recv(pokedexServer,&(stats->f_ffree),sizeof(stats->f_ffree),0);
	recv(pokedexServer,&(stats->f_files),sizeof(stats->f_files),0);
	recv(pokedexServer,&(stats->f_flag),sizeof(stats->f_flag),0);
	recv(pokedexServer,&(stats->f_frsize),sizeof(stats->f_frsize),0);
	recv(pokedexServer,&(stats->f_fsid),sizeof(stats->f_fsid),0);
	recv(pokedexServer,&(stats->f_namemax),sizeof(stats->f_namemax),0);
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
	recvBasicInfo(&resultadoOsada);

	memset(stbuf, 0, sizeof(struct stat));

	if (resultadoOsada==1){
		recv(pokedexServer, &(stbuf->st_nlink),sizeof(stbuf->st_nlink),0); //Recibo el tipo del archivo reconocido por osada
		log_info(archivoLog,"PokedexCliente: El tipo de archivo reconocido por osada_getattr es %d",stbuf->st_nlink);

		//Si es un directorio
		if ((stbuf->st_nlink)==2){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			log_info(archivoLog,"PokedexCliente: El peso del archivo (directorio) es %d",stbuf->st_size);
			stbuf->st_mode=S_IFDIR | 0755;
			stbuf->st_nlink = 2;
		}
		//Si es un archivo regular
		else if (stbuf->st_nlink==1){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			log_info(archivoLog,"PokedexCliente: El peso del archivo (regular) es %d",stbuf->st_size);
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

	sendBasicInfo("READD", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;

	/*(void) offset;
	(void) fi;*/
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	sendBasicInfo("OPENF", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;

}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	sendBasicInfo("READF", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("READF", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){

	sendBasicInfo("CREAT", path);
	sendMode(mode);

	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_truncate (const char * path, off_t offset){

	sendBasicInfo("TRUNC", path);
	sendOffset(offset);

	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_mkdir (const char * path, mode_t modo){

	sendBasicInfo("MKDIR", path);
	sendMode(modo);

	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_rename (const char * path, const char * newPath){

	sendBasicInfo("RENAM", path);
	sendTamanioYContenidoNewPath((char*)newPath);

	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_unlink (const char * path){

	sendBasicInfo("ULINK", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_rmdir (const char * path){

	sendBasicInfo("RMDIR", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	sendBasicInfo("WRITE", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("WRITE", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, (char*)buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_statfs (const char * path, struct statvfs * stats){

	sendBasicInfo("STATF", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	recvCadaElementoDeLaEstructura(stats);


	return resultadoOsada;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
