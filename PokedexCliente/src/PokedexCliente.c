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

		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
		memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
		if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
			perror("Invalid arguments!");
			return EXIT_FAILURE;
		}

		//getting environment variable for connecting to server
		IP_SERVER = getenv("IP_SERVER");
		PORT = getenv("PUERTO");

		printf("Ip Server : %s\n", IP_SERVER);
		printf("Puerto : %s\n", PORT);

		create_socketClient(&pokedexServer, IP_SERVER, PORT);
		printf("Conectado al servidor\n");
		log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

		log_info(archivoLog, "Levanto fuse\n");
		printf("Fuse montado\n");
		printf("%d",fuse_main(args.argc, args.argv, &chamba_oper, NULL));
		return 1;

}

