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
#include "functions/log.h"
#include "commons/structures.h"

#include "socketLib.h"

static int fuse_getattr(const char *path, struct stat *stbuf) {
		int res = 0;
		char * mensaje = string_new();
		string_append(&mensaje, "GETATTR");
		string_append(&mensaje, path);

		if(send(pokedexCliente, &mensaje, sizeof(mensaje), 0)){
		char * resp;
		recv(pokedexCliente, &resp, 1024, 0);
	}
		memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
		if (strcmp(path, "/") == 0) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
	} 	else if (strcmp(path, "Default file path") == 0) {
			stbuf->st_mode = S_IFREG | 0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = strlen("Default file name");
	} 	else {
			res = -ENOENT;
	}
		return res;
}

static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
		(void) offset;
		(void) fi;

		char * mensaje = string_new();
		string_append(&mensaje, "READDIR");
		string_append(&mensaje, path);

		if(send(pokedexCliente, &mensaje, sizeof(mensaje), 0)){
		char * resp;
		recv(pokedexCliente, &resp, 1024, 0);
		}

		if (strcmp(path, "/") != 0)
		return -ENOENT;

		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "Default File Name", NULL, 0);

		return 0;
}

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */
static struct fuse_operations bb_oper = {
		.getattr = fuse_getattr,
		.readdir = fuse_readdir,
		//.open = fuse_open,
		//.read = fuse_read,
};

/*
static int fuse_readdir(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_open(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_read(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
*/

int main(int argc, char *argv[]){

	// Creo archivo log
		archivoLog = crearArchivoLog();

	//Recivo parametros por linea de comandos
		if(argc != 2){
			log_info(archivoLog,"El pokedexCliente no tiene los parametros correctamente seteados.");
			printf("Agregue un punto de montaje.\n");
			return 1;
		}
		puntoPontaje = argv[1]; //tmp
		log_info(archivoLog,"Punto de montaje: %s", puntoPontaje);

	//IP = getenv("POKEIP");
	//PUERTO = getenv("POKEPORT");

		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
		memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
		if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
			perror("Invalid arguments!");
			return EXIT_FAILURE;
		}

		create_socketClient(&pokedexCliente, IP, PUERTO);
		printf("Conectado al servidor\n");
		log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

		fuse_main(args.argc, args.argv, &bb_oper, NULL);
		log_info(archivoLog, "Levanto fuse\n");

		close(pokedexCliente);
		return 0;
}

