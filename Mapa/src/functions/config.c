/*
 * config.c
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../commons/structures.h"

void leerConfiguracion(t_mapa* mapa, char* name, char* pokedexPath){
	char* path = string_new();
	string_append(&path, pokedexPath);
	string_append(&path, "/Mapas/");
	string_append(&path, name);
	string_append(&path, "/metadata");

//¿Como hacemos que cargue el metadata del mapa que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create(path);
	mapa->nombre = name;
	mapa->tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	mapa->batalla = config_get_int_value(config, "Batalla");
	mapa->algoritmo = config_get_string_value(config, "algoritmo");
	mapa->quantum = config_get_int_value(config, "quantum");
	mapa->retardo = config_get_int_value(config, "retardo");
	mapa->ip = config_get_string_value(config, "IP");
	mapa->puerto = config_get_string_value(config, "Puerto");

	free(config);
}
