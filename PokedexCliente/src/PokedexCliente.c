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

		while(0){
			create_socketClient(&pokedexServer, IP, PUERTO);
			printf("Conectado al servidor\n");
			log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

			fuse_main(args.argc, args.argv, &chamba_oper, NULL);
			log_info(archivoLog, "Levanto fuse\n");
		}

		close(pokedexServer);
		return 0;
}

