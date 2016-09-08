#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "../commons/structures.c"


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
	log_info(archivoLogs, "CONFIGUtrenadores,&RACION DEL METADATA");
	log_info(archivoLogs, "Nombre: %s", entrenador->nombre);
	log_info(archivoLogs, "Simbolo: %s", entrenador->simbolo);
	log_info(archivoLogs, "Vidas: %d", entrenador->vidas);
	log_info(archivoLogs, "Reintentos: %d", entrenador->reintentos);

	//Recorro los mapas
		int i;
		for(i=0; i<list_size(entrenador->hojaDeViaje); i++){
			t_mapa* mapa = (t_mapa*)list_get(entrenador->hojaDeViaje, i);
			log_info(archivoLogs, "Mapa: %s", mapa->nombre);
			//Recorro los objetivos
				int j;
				for(j=0; j<list_size(mapa->objetivos); j++){
					log_info(archivoLogs, "Objetivo: %s", list_get(mapa->objetivos, j));
				}

		}
}

