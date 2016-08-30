#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "socketLib.h"

#define IP_MAPA "127.0.0.1" //Define Ip
#define PUERTO_MAPA "6667"  // Define numero Puerto
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

void leerConfiguracion();

int main(){

	leerConfiguracion();

	int serverMapa;

	printf("Conectandose al servidor...\n");
	create_socketClient(&serverMapa, IP_MAPA, PUERTO_MAPA);
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

void leerConfiguracion(){

	int c;
	FILE *file;
	file = fopen("../../PokedexConfig/Entrenadores/Red/metadata", "r");
	if (file){
	    while ((c = getc(file)) != EOF)
	        printf("%c",c);
	    fclose(file);
	}
}

