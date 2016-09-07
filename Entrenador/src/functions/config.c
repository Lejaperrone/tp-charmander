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

	char**p = config_get_array_value(config, "hojaDeViaje");
	t_ruta_mapa ** hojaDeViaje =malloc(0);
	int cant = 0;

	while(*p!=NULL){
		cant++;
		t_ruta_mapa* ruta = malloc(sizeof(t_ruta_mapa));
		ruta->nombre = *p;

		realloc(hojaDeViaje, cant*sizeof(t_ruta_mapa));
		hojaDeViaje[cant-1] = ruta;
		//TODO una vez que leemos el nombre del mapa deberiamos cargar los objetivos
		p++;
	}

	entrenador->hojaDeViaje = hojaDeViaje;
	entrenador->cantMapas = cant;


	free(config);
}

