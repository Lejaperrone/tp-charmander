/*
 * deadlock.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#include "../commons/structures.h"
#include <pkmn/battle.h>


t_list* entrenadoresEnDeadlock;
t_list* posiblesEntrenadoresEnDeadlock;
int sizeTrainersOnMap;

//bool(*condition)(void*)

bool estaDisponible (t_pokemon_custom* unPoke){
	return (unPoke->disponible==1);
}

bool noHayPokemonsDisponiblesPara(t_entrenador* unE){
	t_pokemon_custom* unPoke=malloc(sizeof(t_pokemon_custom));
	unPoke=unE->ultimoPokeSolicitado;
	int sizePokenests=list_size(mapa->pokeNests);
	int i;
	t_list* pokemonsDisponibles=malloc(sizeof(t_list));
	for(i=0;i<sizePokenests;i++){
	t_pokenest* unaPokenest=malloc(sizeof(t_pokenest));
			 unaPokenest=list_get(mapa->pokeNests,i);

			 pokemonsDisponibles=list_filter(unaPokenest->pokemons,(void*)estaDisponible (unPoke));
	}
	 return (list_size(pokemonsDisponibles)<0);
}

bool poseePokemonSolicitado (t_entrenador* unE){

	return (list_count_satisfying(unE->pokemons,(void*)estaDisponible)>0);
}

void* deadlock(void* arg){
	log_trace(archivoLog, "Deadldock - Arranca");
	while (1){
		sleep(mapa->retardo);
		int i;
		sizeTrainersOnMap=list_size(entrenadoresListos);
				//+list_size(t_entrenadoresBloqueados);
		t_list* posiblesEntrenadoresEnDeadlock=malloc(sizeof(t_list));
		list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresListos);
		list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresBloqueados);
		for (i=0;i<sizeTrainersOnMap;i++){
			t_entrenador* posibleEntrenadorEnDeadlock=malloc(sizeof(t_entrenador));
			posibleEntrenadorEnDeadlock=(t_entrenador*)list_get(posiblesEntrenadoresEnDeadlock,i);
			if (noHayPokemonsDisponiblesPara(posibleEntrenadorEnDeadlock) && poseePokemonSolicitado(posibleEntrenadorEnDeadlock) ){
				list_add(entrenadoresEnDeadlock,posibleEntrenadorEnDeadlock);
			}
		}
		if (list_size(entrenadoresEnDeadlock)>0&&mapa->batalla==1){

		}
	}

	return arg;
}
