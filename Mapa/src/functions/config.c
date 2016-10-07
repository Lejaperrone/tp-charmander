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

t_pokemon_custom* createPokemon(char* pathPokemons, char* name){

	//Genero path del pokemon
		char* pathPokemon = string_new();
		string_append(&pathPokemon, pathPokemons);
		string_append(&pathPokemon, "/");
		string_append(&pathPokemon, name);

	//Alloco el pokemon
		t_pokemon_custom* pokemon = malloc(sizeof(t_pokemon_custom));

	//Levanto la configuracion del pokemon
		t_config* configPokemon = config_create(pathPokemon);
		pokemon->nivel =config_get_int_value(configPokemon, "Nivel");
		pokemon->path = pathPokemon;
		pokemon->disponible =1;
		pokemon->duenio=' ';
		free(configPokemon);

	//Return
		return pokemon;
}

t_pokenest* createPokeNest(char* pathPokenest, char* name){
	//Creo el path del metadata
		char* pathPokenestMeta = string_new();
		string_append(&pathPokenestMeta, pathPokenest);
		string_append(&pathPokenestMeta, "/metadata");

	//Alloco la pokenest
		t_pokenest* pokenest = malloc(sizeof(t_pokenest));

	//Levanto la configuracion de la pokenest
		t_config* configPokeNest = config_create(pathPokenestMeta);
		pokenest->tipo = config_get_string_value(configPokeNest, "Tipo");
		pokenest->identificador = (config_get_string_value(configPokeNest, "Identificador"))[0];

		char* pos = config_get_string_value(configPokeNest, "Posicion");
		char** ub = string_split(pos, ";");
		pokenest->ubicacion.x = atoi(ub[0]);
		pokenest->ubicacion.y = atoi(ub[1]);
		pokenest->pokemons = list_create();

		free(configPokeNest);

	//Recorro los pokemons
		DIR * pokemons;
		struct dirent *ep2;
		pokemons = opendir (pathPokenest);

		if (pokemons != NULL){
			while ((ep2 = readdir (pokemons))){
				if(string_ends_with(ep2->d_name, "dat")){
					t_pokemon_custom* pokemon = createPokemon(pathPokenest, ep2->d_name); //Creo el  pokemon
					pokemon->identificadorPokenest = pokenest->identificador;
					list_add(pokenest->pokemons,pokemon); //Agrego a la lista de pokemons de la pokenest
				}
			}
		}
		(void) closedir (pokemons);

	//Return
		return pokenest;
}

void leerConfiguracionMetadataMapa(t_mapa* mapa, char* name, char* pokedexPath){
	char* path = string_new();
	string_append(&path, pokedexPath);
	string_append(&path, "/Mapas/");
	string_append(&path, name);
	string_append(&path, "/metadata");

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
}

void leerConfiguracion(t_mapa* mapa, char* name, char* pokedexPath){
	leerConfiguracionMetadataMapa(mapa, name, pokedexPath);


	//Creo el path de los pokenest
	char* pathPokenests = string_new();
	string_append(&pathPokenests, pokedexPath);
	string_append(&pathPokenests, "/Mapas/");
	string_append(&pathPokenests, name);
	string_append(&pathPokenests, "/PokeNests");

	DIR * directory;

	struct dirent *ep;
	directory = opendir (pathPokenests);

	if (directory != NULL){
		while ((ep = readdir (directory))!=NULL){
			if(strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")){
				//Creo el directorio del Pokenest
					char* pathPokeNest = string_new();
					string_append(&pathPokeNest, pathPokenests);
					string_append(&pathPokeNest, "/");
					string_append(&pathPokeNest, ep->d_name);

				//Agrego el pokenest
					t_pokenest* pokenest = createPokeNest(pathPokeNest, ep->d_name);
					if(pokenest!=NULL){
						list_add(mapa->pokeNests,pokenest);
					}
			}
		}
	}
	(void) closedir (directory);
}
