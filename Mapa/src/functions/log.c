/*
 * log.c
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include "../commons/structures.h"

t_log* crearArchivoLog() {

	remove("logsMapa");

	t_log* logs = log_create("logsMapa", "MapaLog", 0, LOG_LEVEL_TRACE);

	if (logs == NULL) {
		puts("No se pudo generar el archivo de logueo.\n");
		return NULL;
	};


	log_info(logs, "ARCHIVO DE LOGUEO INICIALIZADO");

	return logs;
}

void loguearConfiguracion(t_log* archivoLogs, t_mapa* mapa){
	log_info(archivoLogs, "CONFIGURACION DEL METADATA");
	log_info(archivoLogs, "TiempoChequeoDeadLock: %d", mapa->tiempoChequeoDeadlock);
	log_info(archivoLogs, "Batalla: %d", mapa->batalla);
	log_info(archivoLogs, "Algoritmo: %s", mapa->algoritmo);
	log_info(archivoLogs, "Quantum: %d", mapa->quantum);
	log_info(archivoLogs, "Retardo: %d", mapa->retardo);
	log_info(archivoLogs, "Puerto: %s", mapa->puerto);
	log_info(archivoLogs, "IP: %s", mapa->ip);

	int i;
	for(i=0; i<list_size(mapa->pokeNests); i++){
		t_pokenest* pokenest = (t_pokenest*)list_get(mapa->pokeNests, i);
		log_info(archivoLogs, "Pokenest: %c", pokenest->identificador);

		int j;
		for(j=0; j<list_size(pokenest->pokemons); j++){
			t_pokemon* pokemon = (t_pokemon*)list_get(pokenest->pokemons, j);
			log_info(archivoLogs, "Pokemon path: %s", pokemon->path);
			log_info(archivoLogs, "Pokemon nivel: %d", pokemon->nivel);
		}

	}

}
