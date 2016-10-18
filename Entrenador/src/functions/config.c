#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "../commons/structures.c"


void leerConfiguracion(t_entrenador* entrenador, char* name, char* pokedexPath) {
	char* path = string_new();
	string_append(&path, pokedexPath);
	string_append(&path, "/Entrenadores/");
	string_append(&path, name);
	string_append(&path, "/metadata");

	//Leo configuracion del entrenador
		t_config* config = config_create(path);
		entrenador->nombre = config_get_string_value(config, "nombre");
		entrenador->simbolo = *config_get_string_value(config, "simbolo");
		entrenador->vidas = config_get_int_value(config, "vidas");
		entrenador->reintentos = config_get_int_value(config, "reintentos");
		entrenador->vecesQueMurio=0;

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
				t_objetivo* objetivo = malloc(sizeof(t_objetivo));
				objetivo->nombre = *o;
				objetivo->ubicacion.x = -1;
				objetivo->ubicacion.y = -1;
				objetivo->logrado = 0;

				list_add(mapa->objetivos, objetivo);
				o++;
			}

		//Obtengo la configuracion del mapa
			char* pathMapa = string_new();
			string_append(&pathMapa, pokedexPath);
			string_append(&pathMapa, "/Mapas/");
			string_append(&pathMapa, mapa->nombre);
			string_append(&pathMapa, "/metadata");

			t_config* configMapa = config_create(pathMapa);
			mapa->ip = config_get_string_value(configMapa, "IP");
			mapa->puerto = config_get_string_value(configMapa, "Puerto");
			mapa->miPosicion.x= 1;
			mapa->miPosicion.y= 1;

			free(configMapa);

		//Agrego el mapa a mi hoja de Viaje
			list_add(entrenador->hojaDeViaje,mapa);

		p++;
	}


	free(config);
}

