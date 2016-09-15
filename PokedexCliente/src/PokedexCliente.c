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

#define IP "127.0.0.1" //Lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PUERTO "7666"  // En realidad lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(){

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

		close(serverMapa);
		return 0;
}

