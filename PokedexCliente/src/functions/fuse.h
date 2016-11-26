/*
 * fuse.h
 *
 *  Created on: 1/11/2016
 *      Author: utnso
 */

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#ifndef FUNCTIONS_FUSE_H_
#define FUNCTIONS_FUSE_H_

void armarMensajeBasico(char* nombreFuncion, char* path, char** mensaje);
void conectarConServidorYRecibirRespuesta(int pokedexServer, char* mensaje, char** respuesta);
int chamba_getattr (const char* path, struct stat* stbuf);
int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int chamba_open (const char * path, struct fuse_file_info * fi);
int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi);
int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi);
int chamba_truncate (const char * path, off_t offset);
int chamba_mkdir (const char * path, mode_t modo);
int chamba_rename (const char * path, const char * newPath);
int chamba_unlink (const char * path);
int chamba_rmdir (const char * path);
int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi);
int chamba_statfs (const char * path, struct statvfs * stats);

static struct fuse_operations chamba_oper = {
	.getattr        = chamba_getattr, //obtiene lo atributos de un path
	.readdir        = chamba_readdir, //lee un directorio
	.open           = chamba_open, //abre  un archivo
	.read           = chamba_read, //lee
	.create			= chamba_create, //crea y abre un archivo.
	.truncate       = chamba_truncate, //cambia  el tamanio de un archivo (lo reserva)
	.mkdir          = chamba_mkdir, //crea  un directorio
	.rename         = chamba_rename, //cambia el nombre  del archivo
	.unlink         = chamba_unlink, //elimina un archivo
	.rmdir          = chamba_rmdir, //elimina un directorio
	.write          = chamba_write, //escribe
	.statfs         = chamba_statfs //estadisticas del filesystem, espacio disponible, etc.
};

#endif /* FUNCTIONS_FUSE_H_ */
