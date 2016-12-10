#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "socketLib.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include "osada/osada.h"
#include <pthread.h>
#include <stdint.h>
#include <pthread.h>
#include "commons/structures.h"
#include "commons/definitions.h"
#include "threads/onDemand.h"

t_log* crearArchivoLogPokedexServer() {

	remove("logPokedexServer");

	t_log* logs = log_create("logPokedexServer", "PokedexServidoLog", 0, LOG_LEVEL_TRACE);

	if (logs == NULL) {
		puts("No se pudo generar el archivo de logueo.\n");
		return NULL;
	};


	log_info(logs, "ARCHIVO DE LOGUEO INICIALIZADO");

	return logs;
}

int main(){
	logPokedexServer=crearArchivoLogPokedexServer();
	log_info(logPokedexServer,"Inicio OSADA");
	//Osada
	osada_init("../../challenge.bin");
	log_info(logPokedexServer,"Inicializo semaforos para el bitmap\n");
	pthread_mutex_init(&mutexBitmap,NULL);
	log_info(logPokedexServer,"Inicializo semaforos para la tabla de archivos\n");
	pthread_mutex_init(&mutexTablaArchivos,NULL);
	log_info(logPokedexServer,"Inicializo semaforos para la tabla de asignaciones\n");
	pthread_mutex_init(&mutexTablaAsignaciones,NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	log_info(logPokedexServer,"Inicio hilos detached");
	//Inicializo socket para escuchar
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);


	PORT = getenv("PUERTO");
	int listeningSocket;
	create_serverSocket(&listeningSocket, PORT);
	log_info(logPokedexServer,"Puerto por variable de entorno OK");
	log_info(logPokedexServer, "--------------------------------------------------------------");

	//Inicializo el select
	fd_set master;		// conjunto maestro de descriptores de fichero
	fd_set read_fds;	// conjunto temporal de descriptores de fichero para select()
	int fdmax;			// número máximo de descriptores de fichero
	int newfd;			// descriptor de socket de nueva conexión aceptada
	int i;
	int nbytes;
	char package[PACKAGESIZE];

	FD_ZERO(&master);					// borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(listeningSocket, &master);	// añadir listener al conjunto maestro
	fdmax = listeningSocket; 			// seguir la pista del descriptor de fichero mayor, por ahora es este

	//Me mantengo en el bucle para asi poder procesar cambios en los sockets
	while(1) {
		//Copio los sockets y me fijo si alguno tiene cambios, si no hay itinero de vuelta
		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		//Recorro los sockets con cambios
		for(i = 0; i <= fdmax; i++) {
			//Detecto si hay datos en un socket especifico
			if (FD_ISSET(i, &read_fds)) {
				//Si es el socket de escucha proceso el nuevo socket
				if (i == listeningSocket) {
					addrlen = sizeof(addr);
					if ((newfd = accept(listeningSocket, (struct sockaddr*)&addr, &addrlen)) == -1){
						perror("accept");
					} else {
						FD_SET(newfd, &master); // Añado el nuevo socket al  select
						//Actualizo la cantidad
						if (newfd > fdmax) {
							fdmax = newfd;
						}


						printf("selectserver: new connection from %s on ""socket %d\n", inet_ntoa(addr.sin_addr),i);
					}
				} else {
					//Si es un socket existente
					if ((nbytes = recv(i, package, 11*sizeof(char), 0)) <= 0) {
						//Si la conexion se cerro
						if (nbytes == 0) {
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master); // eliminar del conjunto maestro

					} else {
						// tenemos datos de algún cliente

						if (nbytes != 0){
							t_hilo *h=malloc(sizeof(t_hilo));
							h->socket = i;
							h->id=atoi(package);

							FD_CLR(i, &master);// eliminar del conjunto maestro

							printf("Se intento conectar: %d\n", h->id);

							pthread_create(&thread, NULL,(void*)procesarPeticiones,h);
						}
					}
				}
			}
		}
	}

	log_info(logPokedexServer, "POKEDEXSERVER - Se cerro PokedexServer");
	//Libero memoria y termino ui
	close(listeningSocket);

	//Termino el mapa
	return 0;

}
