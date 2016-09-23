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

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar


//Estructuras y variables globales
t_mapa* mapa;
char* name;
char* pokedexPath;

typedef struct entrenadorEnMapa{
	char simbolo;
	int socket;
	int posx;
	int posy;
	char* pokemonesCapturados;
	int mapaFinalizado;
	struct entrenadorEnMapa* sig;
}t_entrenadorEnMapa;

t_log* archivoLog;
t_list* t_elementosEnMapa;
t_list* t_entrenadoresBloqueados;
t_list* t_entrenadoresListos;
t_list* entrenadoresEnMapa;

void inicializarListasDeEntrenadoresParaPlanificar(){
	t_entrenadoresBloqueados=list_create();
	t_entrenadoresListos=list_create();
	log_info(archivoLog,"Se crearon las listas de entrenadores listos y bloqueados\n");
}

void encolarEntrenadorAlIniciar(int* i,char* package){
	t_entrenadorEnMapa nuevoEntrenador;
	entrenadoresEnMapa=malloc(sizeof(t_entrenadorEnMapa));
	nuevoEntrenador.socket=*i;
	nuevoEntrenador.posx=0;
	nuevoEntrenador.posy=0;
	nuevoEntrenador.pokemonesCapturados=NULL;
	nuevoEntrenador.mapaFinalizado=0;
	nuevoEntrenador.simbolo=package[0];
	nuevoEntrenador.sig=NULL;
	list_add(entrenadoresEnMapa,&nuevoEntrenador);
	free(entrenadoresEnMapa);
}
t_pokenest *find_pokenest_by_id(char id) {
            		int _is_the_one(t_pokenest *p) {
            			return (p->identificador==id);

            	}
            		return list_find(mapa->pokeNests, (void*) _is_the_one);
}

void dibujarEntrenadorEnElOrigen(int* socket, char* package,int posx,int posy, int* posicionInicial){
	CrearPersonaje(t_elementosEnMapa,package[0],1,1);
	//list_add(t_entrenadores,&(package[0]));
	nivel_gui_dibujar(t_elementosEnMapa,mapa->nombre);
	*posicionInicial=1;
}


//esto se transformara en un hilo
void enviarAlPlanificador(int* i, char* paquete){

	while(1){
	log_info(archivoLog,"entra al planificador: %c\n",paquete[0]);
	//Intento recibir un mensaje del entrenador
	int peticion=recv(*i,paquete,6,0);
	t_pokenest pokenestObjetivo;

	if(paquete[0]== 'C' && paquete[1]== 'A' && paquete[2]== 'P' && paquete[3]== 'T' && paquete[4]== 'U'){
		pokenestObjetivo = find_pokenest_by_id(paquete[5])[0];
		log_info(archivoLog,"encontre pokenest %d, %d\n",pokenestObjetivo.ubicacion.x, pokenestObjetivo.ubicacion.y);
	}

	/*if (peticion<=6){
			int caracter;
			for (caracter=0;caracter<strlen(paquete);caracter++){
				mensaje[caracter]=paquete[caracter];
			}
			log_info(archivoLog,"%c quiere %c a %c\n",mensaje[0],mensaje[1],mensaje[6]);
	}*/
		//log_info(archivoLog,"%s\n",mensaje);

	while(paquete[0]!='F'){
		//t_entrenador entrenadorEjecutando;
		//t_coordenadas coordenadasDelTurno;
		log_info(archivoLog,"%c no finalizo su objetivo\n",paquete[0]);
		switch(paquete[0]){
			case 'C':
				//obtengo la pokenest a la que quiere llegar el entrenador
				//pokenestObjetivo=list_find(mapa->pokeNests, filtrarPokenest);
				//Envio al entrenador la coordenada de la pokenest
				;
				char* posicion=malloc(sizeof(char));
					char* posy=malloc(sizeof(char));
					char* posx=malloc(sizeof(char));
				log_info(archivoLog,"Quiere capturar un pokemon\n");
				sprintf(posx,"%i",pokenestObjetivo.ubicacion.x);
				log_info(archivoLog,"Posicion en x %s\n",posx);
				sprintf(posy,"%i",pokenestObjetivo.ubicacion.y);
				log_info(archivoLog,"Posicion en y %s\n",posy);
				string_append(&posicion,posx);
				string_append(&posicion,posy);
				send(*i,posx, 2,0);
				send(*i, posy,2,0);
				break;
			case 'M':
				//entrenadorEjecutando=list_get(t_entrenadoresListos,0);
			send(*i, "QUANTUM", 7, 0);
			break;
		}
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
	//Creo el hilo planificador


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
									int posicionInicial=0;
									encolarEntrenadorAlIniciar(&i,package);
									dibujarEntrenadorEnElOrigen(&i,package,1,1,&posicionInicial);
									send(i,"QUANTUM",7,0);
									enviarAlPlanificador(&i,package);

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




