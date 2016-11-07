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
#include "functions/fuse.h"

#include "socketLib.h"


/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */
static struct fuse_operations bb_oper = {
		.getattr = chamba_getattr,
		.readdir = chamba_readdir,
		.open = chamba_open,
		.create = chamba_create,
		.read = chamba_read,
	    //.init = chamba_init,
	    .rmdir = chamba_rmdir,
	    .unlink = chamba_unlink,
		.mkdir = chamba_mkdir,
	    .rename = chamba_rename,
		.write = chamba_write,
		//.access = chamba_access,
		//.readlink	= chamba_readlink,
		//.mknod = chamba_mknod,
		//.link = chamba_link,
		//.chmod = chamba_chmod,
		//.chown = chamba_chown,
		.statfs = chamba_statfs,
};

/*
static int fuse_readdir(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}*/
static int fuse_open(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_read(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}

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


		create_socketClient(&pokedexServer, IP, PUERTO);
		printf("Conectado al servidor\n");
		log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

		log_info(archivoLog, "Levanto fuse\n");
		return fuse_main(args.argc, args.argv, &chamba_oper, NULL);

}

