/*
 * config.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include "../commons/structures.h"

#ifndef FUNCTIONS_CONFIG_H_
#define FUNCTIONS_CONFIG_H_

	void leerConfiguracionMetadataMapa(t_mapa* mapa, char* name, char* pokedexPath);
	void leerConfiguracion(t_mapa* mapa, char* name, char* pokedexPath);
	t_pokemon_custom* createPokemon(char* pathPokemons, char* name);
	t_pokenest* createPokeNest(char* pathPokenest, char* name);

#endif /* FUNCTIONS_CONFIG_H_ */
