#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include "../commons/structures.c"


void leerConfiguracion(t_entrenador* entrenador) {

//¿Como hacemos que cargue el metadata del entrenador que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create(
			"../../PokedexConfig/Entrenadores/Red/metadata");
	entrenador->nombre = config_get_string_value(config, "nombre");
	entrenador->simbolo = config_get_string_value(config, "simbolo");
	entrenador->vidas = config_get_int_value(config, "vidas");
	entrenador->reintentos = config_get_int_value(config, "reintentos");

	free(config);
}

