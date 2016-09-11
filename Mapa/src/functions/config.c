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

			//Verifico si efectivamente es una pokenest
				pokemons = opendir (pathPokemons);
				if (pokemons != NULL){
					//Alloco la pokenest
						t_pokenest* pokenest = malloc(sizeof(t_pokenest));

					//Levanto la configuracion de la pokenest
						t_config* configPokeNest = config_create(pathPokenest);
						pokenest->tipo = config_get_string_value(configPokeNest, "Tipo");
						pokenest->identificador = (config_get_string_value(configPokeNest, "Identificador"))[0];

						char* pos = config_get_string_value(configPokeNest, "Posicion");
						char** ub = string_split(pos, ";");
						pokenest->ubicacion.x = atoi(ub[0]);
						pokenest->ubicacion.x = atoi(ub[1]);

						free(configPokeNest);

					//Recorro los pokemons
						while ((ep2 = readdir (pokemons))){
							if(string_ends_with(ep2->d_name, "dat")){
								//Alloco el pokemon
									t_pokemon* pokemon = malloc(sizeof(t_pokemon));
								//Genero path del pokemon
									char* pathPokemon = string_new();
									string_append(&pathPokemon, pathPokemons);
									string_append(&pathPokemon, "/");
									string_append(&pathPokemon, ep2->d_name);
								//Levanto la configuracion del pokemon
									t_config* configPokemon = config_create(pathPokemon);
									pokemon->nivel =config_get_int_value(configPokemon, "Nivel");
									pokemon->path = pathPokemon;
									pokemon->disponible =1;
									pokemon->duenio=' ';
									free(configPokemon);
								//Agrego a  la lista de pokemons de la pokenest
									list_add(pokenest->pokemons,pokemon);
							}
						}
					//Agrego al mapa la pokenest
						list_add(mapa->pokeNests,pokenest);
				}
				(void) closedir (pokemons);


		}
		(void) closedir (directory);
	}*/
}

