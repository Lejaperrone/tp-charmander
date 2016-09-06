#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <tad_items.h>
#include <commons/log.h>

#include "socketLib.h"
#include "commons/structures.c"
#include "functions/log.h"
#include "functions/config.h"

#define IP "127.0.0.1" //Lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PUERTO "6667"  // En realidad lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

t_log* archivoLog;

int main(){

	//pido memoria para guardar el entrenador y leo la configuracion
		t_entrenador* entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
		leerConfiguracion(entrenador);

	//Creo el archivo de log
		archivoLog = crearArchivoLog();

	//Logueo que arranco bien y laconfiguracion del entrenador
		log_info(archivoLog,"Cliente levantado.\n");
		loguearConfiguracion(archivoLog, entrenador);

	int serverMapa;

	printf("Conectandose al servidor...\n");
	create_socketClient(&serverMapa, IP, PUERTO);
	printf("Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");



//------------Envio de mensajes al servidor------------
	int enviar = 1;
	char message[PACKAGESIZE];

	while(enviar){
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message,"exit\n")) enviar = 0;
		if (enviar) send(serverMapa, message, strlen(message) + 1, 0);
	}

	free(archivoLog);
	free(entrenador);
	close(serverMapa);
	return 0;
}
