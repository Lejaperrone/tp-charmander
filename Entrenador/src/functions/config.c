#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../commons/structures.c"


void leerConfiguracion(t_entrenador* entrenador) {

//¿Como hacemos que cargue el metadata del entrenador que corresponde? Porque en este caso cargaria solo el de Red
	t_config* config = config_create("../../PokedexConfig/Entrenadores/Red/metadata");
	entrenador->nombre = config_get_string_value(config, "nombre");
	entrenador->simbolo = config_get_string_value(config, "simbolo");
	entrenador->vidas = config_get_int_value(config, "vidas");
	entrenador->reintentos = config_get_int_value(config, "reintentos");

	char**p = config_get_array_value(config, "hojaDeViaje");
	entrenador->hojaDeViaje = list_create();

	while(*p!=NULL){
		t_ruta_mapa* ruta = malloc(sizeof(t_ruta_mapa));
		ruta->nombre = *p;




		list_add(entrenador->hojaDeViaje,ruta);
		p++;
	}


	free(config);
}

