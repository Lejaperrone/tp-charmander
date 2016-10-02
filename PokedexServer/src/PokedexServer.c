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
#include "osada/osada.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define PUERTO "7666"

int main(){
	//Osada
		osada_init("../../osada.bin");

	//Inicializo socket para escuchar
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int listeningSocket;
		create_serverSocket(&listeningSocket, PUERTO);

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
								printf("selectserver: new connection from %s on ""socket %d\n", inet_ntoa(addr.sin_addr),newfd);
							}
						} else {
							//Si es un socket existente
							if ((nbytes = recv(i, (void*)package, PACKAGESIZE, 0)) <= 0) {
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
									printf("%s",package);
								}
							}
						}
						}
				}
		}

	//Libero memoria y termino ui
		close(listeningSocket);

	//Termino el mapa
	return 0;

}


