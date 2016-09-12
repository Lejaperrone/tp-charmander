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
#include <commons/collections/list.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

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
	mapa->pokeNests = list_create();

	free(config);
/*
	//Creo el path de los pokenest
	char* pathPokenests = string_new();
	string_append(&pathPokenests, pokedexPath);
	string_append(&pathPokenests, "/Mapas/");
	string_append(&pathPokenests, name);
	string_append(&pathPokenests, "/PokeNests");

	DIR * directory;
	DIR * pokemons;
	struct dirent *ep;
	struct dirent *ep2;
	directory = opendir (pathPokenests);

	if (directory != NULL){
		while ((ep = readdir (directory))){
			//Genero path del metadata de la pokenest
				char* pathPokenest = string_new();
				string_append(&pathPokenest, pathPokenests);
				string_append(&pathPokenest, "/");
				string_append(&pathPokenest, ep->d_name);
				string_append(&pathPokenest, "/metadata");

			//Obtengo los pokemos del pokenest
				char* pathPokemons = string_new();
				string_append(&pathPokemons, pathPokenests);
				string_append(&pathPokemons, "/");
				string_append(&pathPokemons, ep->d_name);

				puts(pathPokemons);
			//Verifico si el path es un directorio
				int esDir = 0;
				struct stat s;
				if( stat(pathPokemons,&s) == 0 ){
				    if( s.st_mode & S_IFDIR ){
				    	esDir = 1;
				    }
				}

			//Verifico si efectivamente es una pokenest
				pokemons = opendir (pathPokemons);
				if (pokemons != NULL && esDir){
					//Alloco la pokenest
						t_pokenest* pokenest = malloc(sizeof(t_pokenest));

					//Levanto la configuracion de la pokenest
						t_config* configPokeNest = config_create(pathPokenest);
						pokenest->tipo = config_get_string_value(configPokeNest, "Tipo");
						pokenest->identificador = (config_get_string_value(configPokeNest, "Identificador"))[0];
						pokenest->pokemons = list_create();

						char* pos = config_get_string_value(configPokeNest, "Posicion");
						char** ub = string_split(pos, ";");
						pokenest->ubicacion.x = atoi(ub[0]);
						pokenest->ubicacion.x = atoi(ub[1]);

						free(configPokeNest);


				}
				(void) closedir (pokemons);


		}
		(void) closedir (directory);
	}*/
}

