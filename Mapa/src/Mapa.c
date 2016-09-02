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


#define PUERTO "6667"		//Va a haber que leerlo del metadata del mapa
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

typedef struct{
int tiempoChequeoDeadlock;
int batalla;
char* algoritmo;
int quantum;
int retardo;
int puerto;
//Falta ip, no se que tipo de dato seria
}t_mapa;

int main(){

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
