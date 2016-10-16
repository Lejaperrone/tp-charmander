#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "socketLib.h"
#include "string.h"
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
#include "functions/signals.h"
#include "threads/planificador.h"
#include "threads/deadlock.h"


pthread_mutex_t mutexEntrBQ;

pthread_mutex_t mutexEntrRD;

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(int argc, char *argv[]){
	//Creo archivo de log
		archivoLog = crearArchivoLog();
	//Informo mi PID
		log_info(archivoLog,"PID: %d", getpid());

	//Recivo parametros por linea de comandos
		if(argc != 3){
			log_info(archivoLog,"El mapa no tiene los parametros correctamente seteados.");
			return 1;
		}
		name = argv[1]; //PuebloPaleta
		pokedexPath = argv[2]; //../../PokedexConfig
		log_info(archivoLog,"Name: %s", name);
		log_info(archivoLog,"PokedexPath: %s", pokedexPath);


	//Inicializo listas para el manejo de entrenadores
		log_info(archivoLog,"Inicializo listas");
		entrenadoresPreparados = list_create();
		entrenadoresListos = list_create();
		entrenadoresBloqueados = list_create();
		elementosUI = list_create();
		garbageCollectorEntrenadores = list_create();
		listaDeIdentificadoresDePokenests = list_create();

	//Inicializo UI
		log_info(archivoLog,"Inicializo UI");
		nivel_gui_inicializar();
		nivel_gui_get_area_nivel(&rows, &cols);

	//Alloco memoria de  mapa e inicializo su informacion
		log_info(archivoLog,"Inicializo mapa y leo su configuracion");
		mapa = (t_mapa*) malloc(sizeof(t_mapa));
		log_info(archivoLog,"Inicializo mapa y leo su configuracion");
		leerConfiguracion(mapa, name, pokedexPath);
		log_info(archivoLog,"Inicializo mapa y leo su configuracion");

	//Logueo informacion del mapa
		log_info(archivoLog,"Logueo configuracion");
		loguearConfiguracion(archivoLog, mapa);

	//Muestro recursos en el mapa
		log_info(archivoLog,"Cargo elementos a la UI");
		int j;
		for(j=0; j<list_size(mapa->pokeNests); j++){
			t_pokenest* pokenest = (t_pokenest*)list_get(mapa->pokeNests, j);
			if(pokenest->ubicacion.x<=cols && pokenest->ubicacion.y<=rows){
				CrearCaja(elementosUI, pokenest->identificador, pokenest->ubicacion.x, pokenest->ubicacion.y, list_size(pokenest->pokemons));

			}else{
				log_info(archivoLog,"No se pudo dibujar el recurso %c", pokenest->identificador);
			}
		}

		nivel_gui_dibujar(elementosUI, mapa->nombre);

	//Creo el hilo planificador
		log_info(archivoLog,"Inicializo los hilos de planificacion y deadlock");
		pthread_mutex_init(&mutexEntrBQ,NULL);
		pthread_create(&hiloPlanificador,NULL,planificador, NULL);
		pthread_create(&hiloDeadlock,NULL,deadlock, NULL );



	//Inicializo socket para escuchar
		log_info(archivoLog,"Inicializo el socket de escucha");
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		int listeningSocket;
		create_serverSocket(&listeningSocket, mapa->puerto);

	//Registro signal handler
		log_info(archivoLog,"Registro handler de seniales");
		signal(SIGUSR2, sigusr2_handler); //signal-number 12

	//Inicializo el select
		log_info(archivoLog,"Inicializo el SELECT");
		fd_set master;		// conjunto maestro de descriptores de fichero
		fd_set read_fds;	// conjunto temporal de descriptores de fichero para select()
		int fdmax;			// nÃºmero mÃ¡ximo de descriptores de fichero
		int newfd;			// descriptor de socket de nueva conexiÃ³n aceptada
		int i;
		int nbytes;
		char package;

		FD_ZERO(&master);					// borra los conjuntos maestro y temporal
		FD_ZERO(&read_fds);
		FD_SET(listeningSocket, &master);	// aÃ±adir listener al conjunto maestro
		fdmax = listeningSocket; 			// seguir la pista del descriptor de fichero mayor, por ahora es este

		//Me mantengo en el bucle para asi poder procesar cambios en los sockets
		while(1) {
			//Copio los sockets y me fijo si alguno tiene cambios, si no hay itinero de vuelta
			read_fds = master; // cÃ³pialo
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
							log_info(archivoLog,"Ocurrio error al aceptar una conexion");
						} else {
							FD_SET(newfd, &master); // AÃ±ado el nuevo socket al  select
							//Actualizo la cantidad
							if (newfd > fdmax) {
								fdmax = newfd;
							}

							log_trace(archivoLog, "Nueva conexion de %s en  el socket %d", inet_ntoa(addr.sin_addr),newfd);

						}
					} else {
						//Si es un socket existente
						if ((nbytes = recv(i, &package, 1, 0)) <= 0) {
							//Si la conexion se cerro
							if (nbytes == 0) {
								log_trace(archivoLog, "El socket %d se desconecto", i);
							} else {
								log_trace(archivoLog, "Error al recibir informacion del socket");
							}
							close(i);
							FD_CLR(i, &master); // eliminar del conjunto maestro
						} else {
							// tenemos datos de algÃºn cliente
							if (nbytes != 0){
								log_trace(archivoLog, "Ingresa nuevo entrenador a la lista de entrenadores preparados");
								t_entrenador* entrenador = malloc(sizeof(t_entrenador));
								entrenador->simbolo = package;
								entrenador->socket = i;
								entrenador->pokemons = list_create();
								entrenador->ubicacion.x = 1;
								entrenador->ubicacion.y = 1;
								entrenador->ubicacionObjetivo.x = -1;
								entrenador->ubicacionObjetivo.y = -1;
								pthread_mutex_lock(&mutexEntrRD);
								list_add(entrenadoresPreparados, entrenador);
								pthread_mutex_unlock(&mutexEntrRD);
								FD_CLR(i, &master);// eliminar del conjunto maestro

								log_trace(archivoLog, "Agrego entrenador a preparados: %c", entrenador->simbolo);
							}

						}
					}
				}
			}
		}
		//Destruyo el semaforo de los entrenadores bloqueados
		pthread_mutex_destroy(&mutexEntrRD);
		pthread_mutex_destroy(&mutexEntrBQ);
	//Libero memoria y termino ui
		free(archivoLog);
		nivel_gui_terminar();
		free(mapa);
		close(listeningSocket);

	//Termino el mapa
		return 0;

}
