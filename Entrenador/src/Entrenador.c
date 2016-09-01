#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "socketLib.h"
#include <commons/config.h>

#define IP_MAPA "127.0.0.1" //Lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PUERTO_MAPA "6667"  // En realidad lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

void leerConfiguracion();

typedef struct{
	char* nombre;
	char* simbolo;
	int vidas;
	int reintentos;
	//Falta hoja de viaje y los objetivos
}t_entrenador;

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

//¿Como hacemos que cargue el metadata del entrenador que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create("../../PokedexConfig/Entrenadores/Red/metadata");
	t_entrenador* entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
	entrenador->nombre = config_get_string_value(config, "nombre");
	entrenador->simbolo = config_get_string_value(config, "simbolo");
	entrenador->vidas = config_get_int_value(config, "vidas");
	entrenador->reintentos = config_get_int_value(config, "reintentos");


//Muestra de que se asignaron bien los datos del archivo metadata. (Para el checkpoint habria que borrar esto)
	printf("---------------Mi configuracion---------------\n");
	printf("Nombre: %s\n", entrenador->nombre);
	printf("Simbolo: %s\n", entrenador->simbolo);
	printf("Vidas: %d\n", entrenador->vidas);
	printf("Reintentos: %d\n", entrenador->reintentos);
	printf("----------------------------------------------\n");
}


