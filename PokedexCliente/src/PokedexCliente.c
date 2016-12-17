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
#include "functions/log.h"
#include "commons/structures.h"
#include "functions/fuse.h"
#include "socketLib.h"

int main(int argc, char *argv[]){

	// Creo archivo log
	archivoLog = crearArchivoLog();

	//Agarro puerto e IP por variables de entorno e imprimo por consola
	IP_SERVER = getenv("IP_SERVER");
	PORT = getenv("PUERTO");
	printf("Ip Server : %s\n", IP_SERVER);
	printf("Puerto : %s\n", PORT);
	printf("PID: %d\n", getpid());

	//Creo el socket t su mutex
	pthread_mutex_init(&mutexSocket,NULL);
	create_socketClient(&pokedexServer, IP_SERVER, PORT);

	//Identificacion
	char* sizePID=malloc(sizeof(char)*11); //LEAK!! aunque se libere
	sprintf(sizePID,"%i",getpid());
	send(pokedexServer, sizePID, 11, 0);
	free(sizePID);

	printf("Conectado al servidor\n");
	log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

	//Levanto fuse
	log_info(archivoLog, "Levanto fuse\n");
	return fuse_main(argc, argv, &chamba_oper, NULL);

	//free (sizePID);
	return EXIT_SUCCESS;

}

