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
#include <commons/config.h>

#define PUERTO "6667"		//Va a haber que leerlo del metadata del mapa
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

void leerConfiguracion();

typedef struct{
int tiempoChequeoDeadlock;
int batalla;
char* algoritmo;
int quantum;
int retardo;
char* puerto;
char* ip;
}t_mapa;

int main(){

	leerConfiguracion();

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int listeningSocket;
	create_serverSocket(&listeningSocket, PUERTO);

	//Atributos para select
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

	//------------Comienzo del select------------
	for(;;) {
		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == listeningSocket) {
					// gestionar nuevas conexiones
					addrlen = sizeof(addr);
					if ((newfd = accept(listeningSocket, (struct sockaddr*)&addr, &addrlen)) == -1){
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {
							// actualizar el máximo
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on ""socket %d\n", inet_ntoa(addr.sin_addr),newfd);
					}
				} else {
					// gestionar datos de un cliente
					if ((nbytes = recv(i, (void*)package, PACKAGESIZE, 0)) <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						// tenemos datos de algún cliente
						if (nbytes != 0){
							printf("%s", package);
						}
					}
				}
			}
		}
	}

	close(listeningSocket);

	return 0;
}


void leerConfiguracion(){

//¿Como hacemos que cargue el metadata del mapa que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create("../../PokedexConfig/Mapas/Ciudad Paleta/metadata");
	t_mapa* mapa = (t_mapa*) malloc(sizeof(t_mapa));
	mapa->tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	mapa->batalla = config_get_int_value(config, "Batalla");
	mapa->algoritmo = config_get_string_value(config, "algoritmo");
	mapa->quantum = config_get_int_value(config, "quantum");
	mapa->retardo = config_get_int_value(config, "retardo");
	mapa->ip = config_get_string_value(config, "IP");
	mapa->puerto = config_get_string_value(config, "Puerto");



//Muestra de que se asignaron bien los datos del archivo metadata. (Para el checkpoint habria que borrar esto)
	printf("---------------Mi configuracion---------------\n");
	printf("TiempoChequeoDeadLock: %d\n", mapa->tiempoChequeoDeadlock);
	printf("Batalla: %d\n", mapa->batalla);
	printf("Algoritmo: %s\n", mapa->algoritmo);
	printf("Quantum: %d\n", mapa->quantum);
	printf("Retardo: %d\n", mapa->retardo);
	printf("Puerto: %s\n", mapa->puerto);
	printf("Ip: %s\n", mapa->ip);
	printf("----------------------------------------------\n");
}
