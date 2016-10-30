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
#include <pthread.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define PUERTO "7666"
pthread_attr_t attr;
pthread_t thread;
typedef struct solicitud{
	int socket_pokedexCliente;
	char nombreArch_Or_File[17];
}t_solicitud;
t_solicitud* solicitud;
t_list* solicitudes;

void procesar_solicitud(){
	while (1){
		t_solicitud* s=(t_solicitud*)list_get(solicitudes,0);
		char turno='T';
		//Le digo a esa solicitud que es su turno
		send(s->socket_pokedexCliente,&turno,1,0);
		//Recibo la accion que quiere ejecutar
		char accion;
		recv(s->socket_pokedexCliente,&accion,1,0);
		//Chequeo que quiere hacer
		switch (accion){
		case 'L':
			recv(s->socket_pokedexCliente,s->nombreArch_Or_File,PACKAGESIZE,0);
			int indice=osada_TA_buscarRegistroPorNombre(s->nombreArch_Or_File,osada_drive.directorio->parent_directory);

		}
	}

}

int main(){
	printf("Inicio osada");
	//Osada
		osada_init("../../osada.bin");
		//pthread_create(&thread,&attr,&procesar_solicitud,NULL);
		solicitudes=list_create();
	//Creo lista de solicitudes
		printf("Se levanto OSADA");
	//Inicializo socket para escuchar
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int listeningSocket;
		create_serverSocket(&listeningSocket, PUERTO);
		printf("Se crea el socket servidor");
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
			printf("Entro al while de los sockets");
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
									/*pthread_attr_init(&attr);
									pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
									solicitud=(t_solicitud*)malloc(sizeof(t_solicitud));
									list_add(solicitudes,solicitud);
									pthread_attr_destroy(&attr);*/
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


