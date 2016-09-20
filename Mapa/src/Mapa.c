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
#include <tad_items.h>
#include <curses.h>
#include <nivel.h>
#include <signal.h>
#include <pthread.h>
#include "commons/structures.h"
#include "functions/config.h"
#include "functions/log.h"
#include "functions/collections_list_extension.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar


//Estructuras y variables globales
t_mapa* mapa;
char* name;
char* pokedexPath;

t_log* archivoLog;
t_list* t_elementosEnMapa;
t_list* t_entrenadoresBloqueados;
t_list* t_entrenadoresListos;
void inicializarListasDeEntrenadoresParaPlanificar(){
	t_entrenadoresBloqueados=list_create();
	t_entrenadoresListos=list_create();
	log_info(archivoLog,"Se crearon las listas de entrenadores listos y bloqueados\n");
}
void hiloPlanificador(int* i, char* paquete){
	while(strcmp(paquete,"FINOB")!=0){
		send(*i, "QUANTUM", 7, 0);
		int nbytes;
		nbytes = recv(*i, (void*)paquete, 7, 0);
		if (nbytes!=0){
		list_add(t_elementosEnMapa,paquete);
		nivel_gui_dibujar(t_elementosEnMapa,mapa->nombre);
	}
	}
}
void sigusr2_handler(int signum){
	log_info(archivoLog,"Recibo senial SIGUSR2, releo metadata.");
	leerConfiguracionMetadataMapa(mapa, name, pokedexPath);
}

int main(int argc, char *argv[]){
	//Recivo parametros por linea de comandos
		if(argc != 3){
			printf("El mapa no tiene los parametros correctamente seteados.\n");
			return 1;
		}
		name = argv[1]; //PuebloPaleta
		pokedexPath = argv[2]; //../../PokedexConfig

	//Inicializo UI
		archivoLog = crearArchivoLog();
		nivel_gui_inicializar();
		inicializarListasDeEntrenadoresParaPlanificar();
	//Alloco memoria de  mapa e inicializo su informacion
		mapa = (t_mapa*) malloc(sizeof(t_mapa));
		leerConfiguracion(mapa, name, pokedexPath);

	//Creo archivo de log y logueo informacion del mapa

		log_info(archivoLog,"Servidor levantado.\n");
		loguearConfiguracion(archivoLog, mapa);

	//Creo lista de elementos para dibujar en el map
		t_elementosEnMapa=list_create();

	//Inicializo socket para escuchar
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int listeningSocket;
		create_serverSocket(&listeningSocket, mapa->puerto);
	//Informo mi PID
		log_info(archivoLog,"PID: %d", getpid());

	//Registro signal handler
		signal(SIGUSR2, sigusr2_handler); //signal-number 12

	//Muestro recursos en el mapa
		int j;
		for(j=0; j<list_size(mapa->pokeNests); j++){
			t_pokenest* pokenest = (t_pokenest*)list_get(mapa->pokeNests, j);
			CrearCaja(t_elementosEnMapa, pokenest->identificador, pokenest->ubicacion.x, pokenest->ubicacion.x, list_size(pokenest->pokemons));
		}

		nivel_gui_dibujar(t_elementosEnMapa, mapa->nombre);

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
								log_trace(archivoLog, "selectserver: new connection from %s on ""socket %d", inet_ntoa(addr.sin_addr),newfd);
							}
						} else {
							//Si es un socket existente
							if ((nbytes = recv(i, (void*)package, PACKAGESIZE, 0)) <= 0) {
								//Si la conexion se cerro
								if (nbytes == 0) {
									log_trace(archivoLog, "selectserver: socket %d hung up", i);
								} else {
									perror("recv");
								}
								close(i);
								FD_CLR(i, &master); // eliminar del conjunto maestro

								//list_remove_custom(t_entrenadores, *package);
								BorrarItem(t_elementosEnMapa, *package);
								nivel_gui_dibujar(t_elementosEnMapa, mapa->nombre);

							} else {
								// tenemos datos de algún cliente
								if (nbytes != 0){
									CrearPersonaje(t_elementosEnMapa,package[0],1,1);
									//list_add(t_entrenadores,&(package[0]));
									nivel_gui_dibujar(t_elementosEnMapa,mapa->nombre);
									hiloPlanificador(&i,package);
								}
							}
						}
						}
				}
		}

	//Libero memoria y termino ui
		free(archivoLog);
		nivel_gui_terminar();
		free(mapa);
		close(listeningSocket);

	//Termino el mapa
	return 0;
}


