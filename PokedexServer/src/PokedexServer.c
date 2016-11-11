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
#include "osada/osada.h"
#include <pthread.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define PUERTO "7666"
pthread_attr_t attr;
pthread_t thread;

void solicitud(){

}

char* obtenerNombreDelDirectorio(char* path){

	int tamanio = strlen(*string_split(path,"/"));

	return string_split(path,"/")[tamanio-1];
}

void devolverResultadoAlCliente(int resultadoDeOsada){

	//	char* resultado = string_new();
	//	string_append(&resultado, string_itoa(resultadoDeOsada));
	//	int nbytesEnvio = send(pokedexCliente, resultado, 1, 0);

	//	if(nbytesEnvio <0){
	//		//No se pudo enviar el mensaje
	//	}
}
void identificarFuncionRecibida(char* package){

	int resultadoOsada;
	char* nombreFuncion = string_new();
	char* path = string_new();

	string_append(&nombreFuncion, string_split(package,",")[0]);
	string_append(&path, string_split(package,",")[1]);

	if(string_equals_ignore_case(nombreFuncion, "GETAT")){
		//invocar la funcion correspondiente de osada des-serializando la estructura
	}
	if(string_equals_ignore_case(nombreFuncion, "READD")){
		t_list* directorios = list_create();

		resultadoOsada = osada_readdir(path, directorios);
	}
	if(string_equals_ignore_case(nombreFuncion, "OPENF")){
		resultadoOsada = osada_open(path);
	}
	if(string_equals_ignore_case(nombreFuncion, "READF")){
		char* buffer = string_new();
		string_append(&buffer, string_split(package,",")[3]);

		size_t size = atoi(string_split(package,",")[4]);
		off_t offset = atoi(string_split(package,",")[5]);

		resultadoOsada = osada_read(path, buffer, size, offset);

	}
	if(string_equals_ignore_case(nombreFuncion, "CREAT")){

		mode_t mode = atoi(string_split(package,",")[3]);

		resultadoOsada = (int)osada_createFile(path, mode);
	}
	if(string_equals_ignore_case(nombreFuncion, "TRUNC")){

		off_t offset = atoi(string_split(package,",")[3]);

		resultadoOsada = osada_truncate(path, offset);
	}
	if(string_equals_ignore_case(nombreFuncion, "MKDIR")){

		mode_t modo = atoi(string_split(package,",")[3]);

		resultadoOsada = osada_createDir(path, obtenerNombreDelDirectorio(path), modo);
	}
	if(string_equals_ignore_case(nombreFuncion, "RENAM")){
		char* newPath = string_split(package,",")[3];

		resultadoOsada = osada_rename(path, newPath);
	}
	if(string_equals_ignore_case(nombreFuncion, "ULINK")){
		resultadoOsada = osada_removeFile(path);
	}
	if(string_equals_ignore_case(nombreFuncion, "RMDIR")){
		resultadoOsada = osada_removeDir(path);
	}
	if(string_equals_ignore_case(nombreFuncion, "WRITE")){
		char* buffer = string_new();
		string_append(&buffer, string_split(package,",")[3]);

		size_t size = atoi(string_split(package,",")[4]);
		off_t offset = atoi(string_split(package,",")[5]);

		resultadoOsada = osada_write(path, buffer, size, offset);

	}
	if(string_equals_ignore_case(nombreFuncion, "STATF")){
		//invocar la funcion correspondiente de osada habiendo des-serializado la estructura
	}
	if(string_equals_ignore_case(nombreFuncion, "RLEAS")){
		//Este supuestamente no se hace segun dijo matias
	}
	if(string_equals_ignore_case(nombreFuncion, "FALOC")){
		//Este supuestamente no se hace segun dijo matias

		//Asignar datos del package spliteado
		//		int amount;
		//		off_t sizeh;
		//		off_t sizef;

		//invocar la funcion correspondiente de osada
	}

	devolverResultadoAlCliente(resultadoOsada);
}

int main(){
	printf("Inicio osada\n");
	//Osada
	osada_init("/home/utnso/git/tp-2016-2c-Chamba/osada.bin");

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
							identificarFuncionRecibida(package);

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
