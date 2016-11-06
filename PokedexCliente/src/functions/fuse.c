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

void armarMensajeBasico(char* nombreFuncion, char* path, char** mensaje){
	string_append(mensaje, nombreFuncion);
	string_append(mensaje, path);
}

void conectarConServidorYRecibirRespuesta(int pokedexServer, char* mensaje, char** respuesta){
	if(send(pokedexServer, &mensaje, sizeof(mensaje), 0)){
		recv(pokedexServer, respuesta, 1024, 0);
	}
}

int chamba_getattr(const char* path, struct stat* stbuf, struct fuse_file_info *fi){
	int res = 0;

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "GETATTR", sizeof("GETATTR"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, "Default file path") == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen("Default file name");
	} else {
		res = -ENOENT;
	}
	return 0;
}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "READDIR", sizeof("READDIR"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			char* stringOffset = string_new();
			string_append(&stringOffset, string_itoa(offset));
			int nbytesEnvioOffset = send(pokedexServer, stringOffset, sizeof(stringOffset), 0);
			if(nbytesEnvioOffset>0){
				recv(pokedexServer, &respuestaServer, 1, 0);
			}
		}
	}


	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, "Default File Name", NULL, 0);

	return 0;
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "OPEN", sizeof("OPEN"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	return 0;
}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "READ", sizeof("READ"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			int nbytesEnvioBuffer= send(pokedexServer, buffer, sizeof(buffer), 0);
			if(nbytesEnvioBuffer>0){
				char* stringSize = string_new();
				string_append(&stringSize, string_itoa(size));
				int nbytesEnvioSize= send(pokedexServer, stringSize, sizeof(stringSize), 0);
				if(nbytesEnvioSize>0){
					char* stringOffset = string_new();
					string_append(&stringOffset, string_itoa(offset));
					int nbytesEnvioOffset = send(pokedexServer, stringOffset, sizeof(stringOffset), 0);
					if(nbytesEnvioOffset>0){
						recv(pokedexServer, &respuestaServer, 1, 0);
					}
				}
			}
		}
	}

	return 0;
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "CREATE", sizeof("CREATE"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			char* stringMode = string_new();
			string_append(&stringMode, string_itoa(mode));
			int nbytesEnvioMode = send(pokedexServer, stringMode, sizeof(stringMode), 0);
			if(nbytesEnvioMode>0){
				recv(pokedexServer, &respuestaServer, 1, 0);
			}
		}
	}

	return 0;
}

int chamba_truncate (const char * path, off_t offset){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "TRUNCATE", sizeof("TRUNCATE"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			char* stringOffset = string_new();
			string_append(&stringOffset, string_itoa(offset));
			int nbytesEnvioOffset = send(pokedexServer, stringOffset, sizeof(stringOffset), 0);
			if(nbytesEnvioOffset>0){
				recv(pokedexServer, &respuestaServer, 1, 0);
			}
		}
	}

	return 0;
}

int chamba_mkdir (const char * path, mode_t modo){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "MKDIR", sizeof("MKDIR"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			char* stringMode = string_new();
			string_append(&stringMode, string_itoa(modo));
			int nbytesEnvioMode = send(pokedexServer, stringMode, sizeof(stringMode), 0);
			if(nbytesEnvioMode>0){
				recv(pokedexServer, &respuestaServer, 1, 0);
			}
		}
	}

	return 0;
}

int chamba_rename (const char * path, const char * newPath){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "RENAME", sizeof("RENAME"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			int nbytesEnvioNuevoPath = send(pokedexServer, newPath, sizeof(newPath), 0);
			if(nbytesEnvioNuevoPath>0){
				recv(pokedexServer, &respuestaServer, 1, 0);
			}
		}
	}

	return 0;
}

int chamba_unlink (const char * path){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "UNLINK", sizeof("UNLINK"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	return 0;
}

int chamba_rmdir (const char * path){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "RMDIR", sizeof("RMDIR"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	return 0;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "WRITE", sizeof("WRITE"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			int nbytesEnvioBuffer= send(pokedexServer, buffer, sizeof(buffer), 0);
			if(nbytesEnvioBuffer>0){
				char* stringSize = string_new();
				string_append(&stringSize, string_itoa(size));
				int nbytesEnvioSize= send(pokedexServer, stringSize, sizeof(stringSize), 0);
				if(nbytesEnvioSize>0){
					char* stringOffset = string_new();
					string_append(&stringOffset, string_itoa(offset));
					int nbytesEnvioOffset = send(pokedexServer, stringOffset, sizeof(stringOffset), 0);
					if(nbytesEnvioOffset>0){
						recv(pokedexServer, &respuestaServer, 1, 0);
					}
				}
			}
		}
	}

	return 0;
}

int chamba_statfs (const char * path, struct statvfs * stats){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "STATFS", sizeof("STATFS"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	return 0;
}

int chamba_release (const char * path, struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "RELEASE", sizeof("RELEASE"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			recv(pokedexServer, &respuestaServer, 1, 0);
		}
	}

	return 0;
}

int chamba_fallocate (const char * path, int amount, off_t sizeh, off_t sizef,  struct fuse_file_info * fi){

	char respuestaServer;

	int nbytesEnvioFuncion = send(pokedexServer, "WRITE", sizeof("WRITE"), 0);
	if(nbytesEnvioFuncion>0){
		int nbytesEnvioPath = send(pokedexServer, path, sizeof(path), 0);
		if(nbytesEnvioPath>0){
			char* stringAmount = string_new();
			string_append(&stringAmount, string_itoa(amount));
			int nbytesEnvioAmount= send(pokedexServer, stringAmount, sizeof(stringAmount), 0);
			if(nbytesEnvioAmount>0){
				char* stringSizeH = string_new();
				string_append(&stringSizeH, string_itoa(sizeh));
				int nbytesEnvioSizeH= send(pokedexServer, stringSizeH, sizeof(stringSizeH), 0);
				if(nbytesEnvioSizeH>0){
					char* stringSizeF = string_new();
					string_append(&stringSizeF, string_itoa(sizef));
					int nbytesEnvioSizeF = send(pokedexServer, stringSizeF, sizeof(stringSizeF), 0);
					if(nbytesEnvioSizeF>0){
						recv(pokedexServer, &respuestaServer, 1, 0);
					}
				}
			}
		}
	}

	return 0;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
