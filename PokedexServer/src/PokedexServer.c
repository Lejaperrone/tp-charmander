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

t_log* logPokedexServer;

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
pthread_attr_t attr;
pthread_t thread;
pthread_mutex_t mutexTablaArchivos;
pthread_mutex_t mutexTablaAsignaciones;
pthread_mutex_t mutexBitmap;
pthread_mutex_t mutexDatos;
char *PORT;

struct hilo{
		char* package;
		int socket;
		int noEsMiPrimeraVez;
};
struct hilo* h;
struct getAttr{
	unsigned long int primerP;
	}typedef t_getAttr;
t_getAttr* getAttr;

struct readFile{
	__SIZE_TYPE__ size;
	__off_t  offset;
}typedef t_readFile;
t_readFile* readFile;

struct createFile{
	__mode_t modo;
}typedef t_createFile;
t_createFile* createFile;

struct truncateFile{
	off_t offset;
}typedef t_truncateFile;
t_truncateFile* truncateFile;

struct makeDir{
	mode_t mode;
}typedef t_makeDir;
t_makeDir* makeDir;

struct write{
	size_t size;
	off_t offset;
}typedef t_write;
t_write* swrite;

struct statvfs{
	    unsigned long int f_bsize;
	    unsigned long int f_frsize;
	#ifndef __USE_FILE_OFFSET64
	    __fsblkcnt_t f_blocks;
	    __fsblkcnt_t f_bfree;
	    __fsblkcnt_t f_bavail;
	    __fsfilcnt_t f_files;
	    __fsfilcnt_t f_ffree;
	    __fsfilcnt_t f_favail;
	#else
	    __fsblkcnt64_t f_blocks;
	    __fsblkcnt64_t f_bfree;
	    __fsblkcnt64_t f_bavail;
	    __fsfilcnt64_t f_files;
	    __fsfilcnt64_t f_ffree;
	    __fsfilcnt64_t f_favail;
	#endif
	    unsigned long int f_fsid;
	#ifdef _STATVFSBUF_F_UNUSED
	    int __f_unused;
	#endif
	    unsigned long int f_flag;
	    unsigned long int f_namemax;
	    int __f_spare[6];
}typedef t_statfs;
t_statfs* statfs;

struct falloc{
	int amoun;
	off_t sizeh;
	off_t sizef;
}typedef t_falloc;
t_falloc* sfalloc;
void solicitud(){

}

char* obtenerNombreDelDirectorio(char* path){

	int tamanio = strlen(*string_split(path,"/"));

	return string_split(path,"/")[tamanio-1];
}

void devolverResultadoAlCliente(int resultadoDeOsada,int socketCliente){
	if(send(socketCliente,&resultadoDeOsada,sizeof(int),0)>=1){
		printf("Se envia correctamente el resultado al cliente\n");
	}
	//	char* resultado = string_new();
	//	string_append(&resultado, string_itoa(resultadoDeOsada));
	//	int nbytesEnvio = send(pokedexCliente, resultado, 1, 0);

	//	if(nbytesEnvio <0){
	//		//No se pudo enviar el mensaje
	//	}
}
void recibirNombreDeLaFuncion(int socketCliente, char** nombreFuncion){
	recv(socketCliente,*nombreFuncion,5*sizeof(char),0);
	log_info(logPokedexServer,"PokedexServer: Recibo %s", *nombreFuncion);
}

void recibirParametrosDeReadDir(int socketCliente,char* path){
	recv(socketCliente,path,sizeof(path),0);
}
void recibirTamanioDelPath(int socketCliente, size_t* tamanio){
	char* size=(char*)malloc(11*sizeof(char));
	recv(socketCliente,size,11,0);
	*tamanio=atoi(size);
	log_info(logPokedexServer,"PokedexServer: Recibo el tamanio del path: %d",*tamanio);
}
void recibirPath(int socketCliente,char** path, int tamanioPath){
	recv(socketCliente,*path,tamanioPath,0);
	log_info(logPokedexServer,"PokedexServer: Recibo el path %s",*path);
}
void recibirBuffer(int socketCliente){
	recv(socketCliente,&(getAttr->primerP),sizeof(getAttr->primerP),0);
}
void enviarBufferLleno(int socketCliente){
	send(socketCliente,&(getAttr->primerP),sizeof(getAttr->primerP),0);
	log_info(logPokedexServer,"PokedexServer: Envio buffer lleno");
}
void* identificarFuncionRecibida(void* arg){
	char* nombreFuncion=string_new();
	char* path=string_new();
	int resultadoOsada;
	size_t tamanioPath;

	//El primer mensaje que recibi fue la cantidad de datos que voy a recibir despues
	//Recibo el nombre de la funcion
		recibirNombreDeLaFuncion(h->socket,&nombreFuncion);
		printf("Identifico de que funcion se trata, es: %s",nombreFuncion);
				/*string_append(&nombreFuncion, string_split(package,",")[0]);
				string_append(&path, string_split(package,",")[1]);*/

		if(string_equals_ignore_case(nombreFuncion, "GETAT")){
			recibirTamanioDelPath(h->socket,&tamanioPath);
			recibirPath(h->socket,&path,tamanioPath);
			recibirBuffer(h->socket);
			//invocar la funcion correspondiente de osada des-serializando la estructura
			resultadoOsada=osada_getattr(path,(file_attr*)getAttr);
			enviarBufferLleno(h->socket);
			}
			/*if(string_equals_ignore_case(nombreFuncion, "READD")){
				recibirParametrosDeReadDir(h->socket,path);
				t_list* directorios=list_create();
				resultadoOsada = osada_readdir(path, directorios);
			}
			if(string_equals_ignore_case(nombreFuncion, "OPENF")){
				resultadoOsada = osada_open(path);
			}
			if(string_equals_ignore_case(nombreFuncion, "READF")){
				char* buffer = string_new();
				recv(h->socket,buffer,sizeof(buffer),0);
				recv(h->socket,&(readFile->size),sizeof(readFile->size),0);
				recv(h->socket,&(readFile->offset),sizeof(readFile->offset),0);
				resultadoOsada = osada_read(path, buffer, readFile->size, readFile->offset);
			}
			if(string_equals_ignore_case(nombreFuncion, "CREAT")){
				recv(h->socket,&(createFile->modo),sizeof(createFile->modo),0);
				resultadoOsada = (int)osada_createFile(path, createFile->modo);
			}
			if(string_equals_ignore_case(nombreFuncion, "TRUNC")){
				recv(h->socket,&(truncateFile->offset),sizeof(truncateFile->offset),0);
				resultadoOsada = osada_truncate(path, truncateFile->offset);
			}
			if(string_equals_ignore_case(nombreFuncion, "MKDIR")){
				recv(h->socket,&(makeDir->mode),sizeof(makeDir->mode),0);
				resultadoOsada = osada_createDir(path, obtenerNombreDelDirectorio(path), makeDir->mode);
			}
			if(string_equals_ignore_case(nombreFuncion, "RENAM")){
				char* nombreNuevo=string_new();
				recv(h->socket,nombreNuevo,sizeof(nombreNuevo),0);
				resultadoOsada = osada_rename(path, nombreNuevo);
			}
			if(string_equals_ignore_case(nombreFuncion, "ULINK")){
				resultadoOsada = osada_removeFile(path);
			}
			if(string_equals_ignore_case(nombreFuncion, "RMDIR")){
				resultadoOsada = osada_removeDir(path);
			}
			if(string_equals_ignore_case(nombreFuncion, "WRITE")){
				char* buffer = string_new();
				recv(h->socket,buffer,sizeof(buffer),0);
				recv(h->socket,&(swrite->size),sizeof(swrite->size),0);
				recv(h->socket,&(swrite->offset),sizeof(swrite->size),0);
				resultadoOsada = osada_write(path, buffer, swrite->size, swrite->offset);
			}
			if(string_equals_ignore_case(nombreFuncion, "STATF")){
				recv(h->socket,&(statfs->__f_spare),sizeof(statfs->__f_spare),0);
				recv(h->socket,&(statfs->f_bavail),sizeof(statfs->f_bavail),0);
				recv(h->socket,&(statfs->f_bfree),sizeof(statfs->f_bfree),0);
				recv(h->socket,&(statfs->f_blocks),sizeof(statfs->f_blocks),0);
				recv(h->socket,&(statfs->f_bsize),sizeof(statfs->f_bsize),0);
				recv(h->socket,&(statfs->f_favail),sizeof(statfs->f_favail),0);
				recv(h->socket,&(statfs->f_ffree),sizeof(statfs->f_ffree),0);
				recv(h->socket,&(statfs->f_files),sizeof(statfs->f_files),0);
				recv(h->socket,&(statfs->f_flag),sizeof(statfs->f_flag),0);
				recv(h->socket,&(statfs->f_frsize),sizeof(statfs->f_frsize),0);
				recv(h->socket,&(statfs->f_fsid),sizeof(statfs->f_fsid),0);
				recv(h->socket,&(statfs->f_namemax),sizeof(statfs->f_namemax),0);
				resultadoOsada=osada_statfs(path,statfs);
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
				recv(h->socket,&(sfalloc->amoun),sizeof(sfalloc->amoun),0);
				recv(h->socket,&(sfalloc->sizeh),sizeof(sfalloc->sizeh),0);
				recv(h->socket,&(sfalloc->sizef),sizeof(sfalloc->sizef),0);
				resultadoOsada=osada_fallocate(path,sfalloc->amoun,sfalloc->sizeh,sfalloc->sizef);
			}*/


	devolverResultadoAlCliente(resultadoOsada,h->socket);
	return arg;
}
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
	osada_init("../../osada.bin");
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
					if ((nbytes = recv(i, package, 5*sizeof(char), 0)) <= 0) {
						//Si la conexion se cerro
						if (nbytes == 0) {
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						pthread_attr_destroy(&attr);
						free(h);
						close(i);
						FD_CLR(i, &master); // eliminar del conjunto maestro

					} else {
						// tenemos datos de algún cliente

						if (nbytes != 0){

							h=(struct hilo*)malloc(sizeof(struct hilo));
							h->package=package;
							h->socket=i;

							log_info(logPokedexServer,"Mando a %d a un hilo a parte", h->socket);
							pthread_create(&thread, &attr,identificarFuncionRecibida,NULL);

							//identificarFuncionRecibida(package);

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
