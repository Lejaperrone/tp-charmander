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
		//Reservo espacio para el mapa con sus objetivos
			t_mapa* mapa = malloc(sizeof(t_mapa));
		//Copio el nombre del mapa
			mapa->nombre = *p;
		//Obtengo los objetivos del mapa
			char * key = string_new();
			string_append(&key, "obj[");
			string_append(&key, mapa->nombre);
			string_append(&key, "]");

			char**o = config_get_array_value(config, key);
			mapa->objetivos = list_create();
			while(*o!=NULL){
				list_add(mapa->objetivos,*o);
				o++;
			}

		list_add(entrenador->hojaDeViaje,mapa);
		p++;
	}


	free(config);
}

