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

pthread_t hiloPlanificador;
pthread_t hiloListener;
pthread_t hiloDeteccionDeadlock;
char* arg1="hiloPlanificador";
char* arg2="hiloListener";
char* arg3="hiloDeteccionDeadlock";
int h1;
int h2;
int h3;

int inicioPlanificador=1;

typedef struct entrenadorConectado{
	int* sock;
	char* paquete;
}t_entrenadorConectado;
t_log* archivoLog;
t_entrenadorConectado* entrenador;
t_list* t_elementosEnMapa;
t_list* t_entrenadoresBloqueados;
t_list* t_entrenadoresListos;
//t_list* entrenadoresEnMapa;

void inicializarListasDeEntrenadoresParaPlanificar(){
	t_entrenadoresBloqueados=list_create();
	t_entrenadoresListos=list_create();
	log_info(archivoLog,"Se crearon las listas de entrenadores listos y bloqueados\n");
}


void encolarEntrenadorAlIniciar(t_entrenadorConectado* entrenador){
	list_add(t_entrenadoresListos,entrenador);
}

t_pokenest *find_pokenest_by_id(char id) {
	int _is_the_one(t_pokenest *p) {
            			return (p->identificador==id);

            	}
            		return list_find(mapa->pokeNests, (void*) _is_the_one);
}
t_pokemon* find_pokemon_by_id(t_list* pokemons, t_pokenest pokenest) {
            		int _is_the_one(t_pokemon *poke) {

            			return (pokenest.identificador==poke->id);

            			return (pokenest.identificador==poke->identificador);


            	}
            		 t_pokemon* pokemonCapturado=malloc(sizeof(t_pokemon));
            		 pokemonCapturado = list_find(mapa->pokeNests, (void*) _is_the_one);
            		pokemonCapturado->disponible=0;
            		return list_find(mapa->pokeNests, (void*) _is_the_one);
}

int estaEnLaPokenest(t_pokemon* pokemon){
	return pokemon->disponible==0;
}
bool pudoSerCapturado(t_pokemon pokemon, t_pokenest* pokenest){
	return list_is_empty(list_filter(pokenest->pokemons,(void*)estaEnLaPokenest));
}



void dibujarEntrenadorEnElOrigen(int* socket, char* package,int posx,int posy, int* posicionInicial){
	CrearPersonaje(t_elementosEnMapa,package[0],1,1);
	log_info(archivoLog,package);
	//list_add(t_entrenadores,&(package[0]));
	nivel_gui_dibujar(t_elementosEnMapa,mapa->nombre);
	*posicionInicial=1;
}
int noHayEntrenadoresListosNiBloqueados (){
	return (list_is_empty(t_entrenadoresListos)&&list_is_empty(t_entrenadoresBloqueados));
}
int entrenadorQuiereConocerUbicacionDePokenest(char* paquete){
return (paquete[0]== 'C' && paquete[1]== 'A' && paquete[2]== 'P' && paquete[3]== 'T' && paquete[4]== 'U');



//esto se transformara en un hilo

int entrenadorQuiereMoverse(char* paquete){
return (paquete[0]=='M'&&paquete[1]=='O'&&paquete[2]=='V'&&paquete[3]=='E'&&paquete[4]=='R');
}
int entrenadorQuiereAtraparUnPokemon (char* paquete){
	return (paquete[0]=='F'&&paquete[1]=='I'&&paquete[2]=='N'&&paquete[3]=='O'&&paquete[4]=='B');
}
void waitEntrenador(t_entrenadorConectado* entrenadorAbloquear){
	list_add(t_entrenadoresBloqueados,entrenadorAbloquear);
	list_remove(t_entrenadoresListos,0);
}

void otorgarQuantum(t_entrenadorConectado* entrenador, int Q, int* t, int* duracion, char** paquete){
	int i;
	*duracion=0;
	for (i=0;i<=Q;i++){
		log_info(archivoLog,"quantum numero %d\n",i);


		int m;
		m=send(*entrenador->sock,"QUANTUM",7,0);
		if (m==-1){
			log_info(archivoLog, "no se envio el mensaje a %c\n",*paquete[0]);
		}
		log_info(archivoLog,"envie senial QUANTUM\n");
		recv(*entrenador->sock,*paquete,6,0);
		log_info(archivoLog,"recibo respuesta del entrenador\n");
		if (entrenadorQuiereConocerUbicacionDePokenest(*paquete)){
			t_pokenest pokenestObjetivo;
			pokenestObjetivo =find_pokenest_by_id(*paquete[5])[0];
			char* posy=malloc(sizeof(char));
			char* posx=malloc(sizeof(char));
			sprintf(posx,"%i",pokenestObjetivo.ubicacion.x);
			log_info(archivoLog,"Posicion en x %s\n",posx);
			sprintf(posy,"%i",pokenestObjetivo.ubicacion.y);
			log_info(archivoLog,"Posicion en y %s\n",posy);
			send(*entrenador->sock,posx, 2,0);
			send(*entrenador->sock, posy,2,0);
			free(posy);
			free(posx);
			log_info(archivoLog,"encontre pokenest %d, %d\n",pokenestObjetivo.ubicacion.x, pokenestObjetivo.ubicacion.y);
		}else{
			if (entrenadorQuiereMoverse(*paquete)){
				send(*entrenador->sock, "QUANTUM", 7, 0);
				char* mensaje=malloc(sizeof(char));
				recv(*entrenador->sock,mensaje,6,0);
				//dibujar que se mueva
				free(mensaje);
			}else{
				if (entrenadorQuiereAtraparUnPokemon(*paquete)){
					t_pokenest pokenestObjetivo;
					pokenestObjetivo = find_pokenest_by_id(*paquete[6])[0];
					//ACA VA UN MUTEX
					t_pokemon* pokeActual=malloc(sizeof(t_pokemon));
					pokeActual=find_pokemon_by_id(pokenestObjetivo.pokemons,pokenestObjetivo);
					list_remove_by_condition(pokenestObjetivo.pokemons, (void*)pudoSerCapturado);
					waitEntrenador(entrenador);
					free(pokeActual);
					exit(1);
					//ACA HAY QUE BORRAR EL POKEMON DE LA POKENEST
				}else{
					//INFORMA AL ENTRENADOR LA RUTA DE LA MEDALLA
				}
			}
		*duracion=*duracion+1;
		if(*paquete[0]=='F'&&*paquete[1]=='I'&&*paquete[2]=='N'&&*paquete[3]=='O'&&*paquete[4]=='B'){
			*t=*t+*duracion;
		}
	}

}waitEntrenador(entrenador);
}

}

void bloquearEntrenador(t_entrenadorConectado* entrenadorAbloquear){
	list_add(t_entrenadoresBloqueados,entrenadorAbloquear);
	list_remove(t_entrenadoresListos,0);
}
/*
void otorgarQuantum(t_entrenadorConectado* entrenador, int Q, int* t, int* duracion, char** paquete){
	int i;
	*duracion=0;
	for (i=0;i<=Q;i++){
		send(*entrenador->sock,"QUANTUM",7,0);
		recv(*entrenador->sock,*paquete,6,0);
		if (entrenadorQuiereConocerUbicacionDePokenest(*paquete)){
			t_pokenest pokenestObjetivo;
			pokenestObjetivo =find_pokenest_by_id(*paquete[5])[0];
			char* posy=malloc(sizeof(char));
			char* posx=malloc(sizeof(char));
			sprintf(posx,"%i",pokenestObjetivo.ubicacion.x);
			log_info(archivoLog,"Posicion en x %s\n",posx);
			sprintf(posy,"%i",pokenestObjetivo.ubicacion.y);
			log_info(archivoLog,"Posicion en y %s\n",posy);
			send(*entrenador->sock,posx, 2,0);
			send(*entrenador->sock, posy,2,0);
			free(posy);
			free(posx);
			log_info(archivoLog,"encontre pokenest %d, %d\n",pokenestObjetivo.ubicacion.x, pokenestObjetivo.ubicacion.y);
		}else{
			if (entrenadorQuiereMoverse(*paquete)){
				send(*entrenador->sock, "QUANTUM", 7, 0);
				char* mensaje=malloc(sizeof(char));
				recv(*entrenador->sock,mensaje,6,0);
				//dibujar que se mueva
				free(mensaje);
			}else{
				if (entrenadorQuiereAtraparUnPokemon(*paquete)){
					t_pokenest pokenestObjetivo;
					//pokenestObjetivo = find_pokenest_by_id(entrenador->pokenestActual)[0];
					//ACA VA UN MUTEX
					t_pokemon* pokeActual=malloc(sizeof(t_pokemon));
					pokeActual=find_pokemon_by_id(pokenestObjetivo.pokemons,pokenestObjetivo);
					list_remove_by_condition(pokenestObjetivo.pokemons, (void*)pudoSerCapturado);
					bloquearEntrenador(entrenador);
					free(pokeActual);
					exit(1);
					//ACA HAY QUE BORRAR EL POKEMON DE LA POKENEST
				}else{
					//INFORMA AL ENTRENADOR LA RUTA DE LA MEDALLA
				}
			}
		*duracion=*duracion+1;
		if(*paquete[0]=='F'&&*paquete[1]=='I'&&*paquete[2]=='N'&&*paquete[3]=='O'&&*paquete[4]=='B'){
			*t=*t+*duracion;
		}
	}

}bloquearEntrenador(entrenador);
}*/

void actualizarTiempo(int* t, int* duracionTurno){
	*t=*t+*duracionTurno+mapa->retardo;
}

//esto se transformara en un hilo
/*
typedef struct datosPlanificar{
	int* socket;
	char* paquete;
}t_datosPlanificador;

t_datosPlanificador* datos;
*/
/*void* planificar(t_entrenadorConectado* datos){
	int Q;
	Q=mapa->quantum;
	int t=0;
	int duracionRealTurno;
	t_entrenadorConectado* entrenadorActual=malloc(sizeof(t_entrenadorConectado));
	log_info(archivoLog,"comienza el planificador\n");
	while(1){

char* simbolo=malloc(sizeof(char));
bool primeraPasada=true;
		if(primeraPasada){
	log_info(archivoLog,"entra al planificador: %s\n",entrenador->paquete);
		primeraPasada=false;}else{
			log_info(archivoLog,"entra al planificador: %s\n",simbolo);
		}
		log_info(archivoLog,"chequeo que no haya entrenadores listos ni bloqueados\n");
		if (noHayEntrenadoresListosNiBloqueados()){
			//Inicio el planificador
			log_info(archivoLog,"agrego a todos los entrenadores que haya a listos\n");
			list_add_all(t_entrenadoresListos,entrenadoresEnMapa);
			log_info(archivoLog,"entrenadores listos OK\n");
			int tamanio;
			tamanio=list_size(t_entrenadoresListos);
			log_info(archivoLog,"hay %d entrenadores listos\n",tamanio);
			entrenadorActual=(t_entrenadorConectado*)list_get(t_entrenadoresListos,0);
			log_info(archivoLog,"elijo al primer entrenador a procesar\n");
			otorgarQuantum(entrenadorActual,Q,&t,&duracionRealTurno,&datos->paquete);
			log_info(archivoLog,"El quantum de %c duro %d\n",datos->paquete[0], duracionRealTurno);

		if (noHayEntrenadoresListosNiBloqueados()){
			//Inicio el planificador
			list_add_all(t_entrenadoresListos,entrenadoresEnMapa);
			entrenadorActual=(t_entrenadorConectado*)list_get(t_entrenadoresListos,0);
			otorgarQuantum(entrenadorActual,Q,&t,&duracionRealTurno,&datos->paquete);

			free(entrenadorActual);
			actualizarTiempo(&t,&duracionRealTurno);//t=t+duracionRealTurno
		}else{
			if (list_is_empty(t_entrenadoresListos)){
				list_add_all(t_entrenadoresListos,t_entrenadoresBloqueados);
			}else{
				entrenadorActual=list_get(t_entrenadoresListos,0);
				otorgarQuantum(entrenadorActual,Q,&t,&duracionRealTurno, &datos->paquete);
				free(entrenadorActual);

				waitEntrenador(entrenadorActual);

				bloquearEntrenador(entrenadorActual);

				actualizarTiempo(&t,&duracionRealTurno);
				sleep(mapa->retardo);
			}

		}
	}
}*/
void planificar(t_entrenadorConectado* entrenador){

	while(1){
	log_info(archivoLog,"entra al planificador: %s\n",entrenador->paquete);

	//Intento recibir un mensaje del entrenador
	recv(*entrenador->sock,entrenador->paquete,8,0);
	//strcpy(simbolo,&entrenador->paquete[6]);
	log_info(archivoLog,"recibo: %s\n",entrenador->paquete);
	t_pokenest pokenestObjetivo;

//Busco pokenest
	if(entrenador->paquete[0]== 'C'){
		pokenestObjetivo = find_pokenest_by_id(entrenador->paquete[5])[0];
		log_info(archivoLog,"encontre pokenest %d, %d\n",pokenestObjetivo.ubicacion.x, pokenestObjetivo.ubicacion.y);
		log_info(archivoLog,"%c no finalizo su objetivo\n",entrenador->paquete[0]);
				t_pokenest pokenestObjetivo;
					char* posy=malloc(sizeof(char));
					char* posx=malloc(sizeof(char));
				log_info(archivoLog,"Quiere capturar un pokemon\n");
				sprintf(posx,"%i",pokenestObjetivo.ubicacion.x);
				log_info(archivoLog,"Posicion en x %s\n",posx);
				sprintf(posy,"%i",pokenestObjetivo.ubicacion.y);
				log_info(archivoLog,"Posicion en y %s\n",posy);
				send(*entrenador->sock,posx, 2,0);
				send(*entrenador->sock, posy,2,0);
	}
	if	(entrenador->paquete[0]=='M'){

				recv(*entrenador->sock,entrenador->paquete,2,0);
				int posxInt=atoi(entrenador->paquete);
				log_info(archivoLog,"Recibo posicion x %d\n",posxInt);
				recv(*entrenador->sock,entrenador->paquete,2,0);
				int posyInt=atoi(entrenador->paquete);
				log_info(archivoLog,"Recibo posicion y %d\n",posyInt);
				//log_info(archivoLog,"Lo que recibi es: %s",entrenador->paquete);
				MoverPersonaje(t_elementosEnMapa,entrenador->paquete[6],posxInt,posyInt);

			send(*entrenador->sock, "QUANTUM", 7, 0);
			break;
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
	log_info(archivoLog,"apunto de crear el hilo\n");
	log_info(archivoLog,"ya cree el hilo\n");
	//Creo el hilo listener
	//h2=pthread_create(&hiloListener,NULL,escucharConexiones, (void*)arg1 ); //la funcion escucharConexiones seria el select
	//Creo el hilo de deteccion de deadlock
	//h3=pthread_create(&hiloDeteccionDeadlock,NULL,detectarDeadlock, (void*)arg1 );//la funcion detectarDeadlock no existe aun

	//Creo archivo de log y logueo informacion del mapa

	log_info(archivoLog,"Servidor levantado.\n");
	loguearConfiguracion(archivoLog, mapa);

	//Creo lista de elementos para dibujar en el map
	t_elementosEnMapa=list_create();
	log_info(archivoLog,"lista de elementos creada\n");

	//Inicializo socket para escuchar
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int listeningSocket;
	create_serverSocket(&listeningSocket, mapa->puerto);

	log_info(archivoLog,"socket de escucha ok\n");
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

						//list_remove_customntren(t_entrenadores, *package);
						BorrarItem(t_elementosEnMapa, *package);
						nivel_gui_dibujar(t_elementosEnMapa, mapa->nombre);

					} else {
						// tenemos datos de algún cliente
						if (nbytes != 0){
							int posicionInicial=0;
							entrenador=malloc(sizeof(t_entrenadorConectado));
							entrenador->paquete=package;
							entrenador->sock=&i;
							encolarEntrenadorAlIniciar(entrenador);
							dibujarEntrenadorEnElOrigen(&i,package,1,1,&posicionInicial);

							send(i,"QUANTUM",7,0);


							planificar(entrenador);

							/*if (inicioPlanificador==1){
								inicioPlanificador=0;
								h1=pthread_create(&hiloPlanificador,NULL,(void*)planificar,(void*)entrenador );
								log_info(archivoLog,"Se inicia el planificador\n");
								log_info(archivoLog,"el hilo trabaja con %s y %d\n",entrenador->paquete,*entrenador->sock);
							}else{
								send(i,"QUANTUM",7,0);
								planificar(entrenador);
							}*/
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
	}
		//Termino el mapa
		return 0;

}

