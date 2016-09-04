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
#include <commons/log.h>

#define IP "127.0.0.1" //Lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PUERTO "6667"  // En realidad lo deberia buscar del metadata del primer mapa de su hoja de viaje
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

void leerConfiguracion();
t_log* crearArchivoLog();
void loguearConfiguracion();

typedef struct{
	char* nombre;
	char* simbolo;
	int vidas;
	int reintentos;
	//Falta hoja de viaje y los objetivos
}t_entrenador;

t_log* archivoLog;

int main(){

	t_entrenador* entrenador = (t_entrenador*) malloc(sizeof(t_entrenador));
	leerConfiguracion(entrenador);

	archivoLog = crearArchivoLog();
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


void leerConfiguracion(t_entrenador* entrenador){

//¿Como hacemos que cargue el metadata del entrenador que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create("../../PokedexConfig/Entrenadores/Red/metadata");
	entrenador->nombre = config_get_string_value(config, "nombre");
	entrenador->simbolo = config_get_string_value(config, "simbolo");
	entrenador->vidas = config_get_int_value(config, "vidas");
	entrenador->reintentos = config_get_int_value(config, "reintentos");

	free(config);
}

t_log* crearArchivoLog() {

	remove("logsEntrenador");

	t_log* logs = log_create("logsEntrenador", "EntrenadorLog", 0, LOG_LEVEL_TRACE);

	if (logs == NULL) {
		puts("No se pudo generar el archivo de logueo.\n");
		return NULL;
	}


	log_info(logs, "ARCHIVO DE LOGUEO INICIALIZADO");

	return logs;
}

void loguearConfiguracion(t_log* archivoLogs, t_entrenador* entrenador){
	log_info(archivoLogs, "CONFIGURACION DEL METADATA");
	log_info(archivoLogs, "Nombre: %s", entrenador->nombre);
	log_info(archivoLogs, "Simbolo: %s", entrenador->simbolo);
	log_info(archivoLogs, "Vidas: %d", entrenador->vidas);
	log_info(archivoLogs, "Reintentos %d", entrenador->reintentos);
	//Faltan los que son de array
}


