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
	t_list* pokemonsDisponibles=malloc(sizeof(t_list));
	unPoke=unE->ultimoPokeSolicitado;
	int numPokenest;
	for(numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
	t_pokenest* unaPokenest=malloc(sizeof(t_pokenest));
			 unaPokenest=list_get(mapa->pokeNests,numPokenest);
			 pokemonsDisponibles=list_filter(unaPokenest->pokemons,(void*)estaDisponible (unPoke));
			 free(unaPokenest);
	}
	 free(unPoke);
//	 free(pokemonsDisponibles);
	 return (list_size(pokemonsDisponibles)<0);
}

bool poseePokemonSolicitado (t_entrenador* unE){

	return (list_count_satisfying(unE->pokemons,(void*)estaDisponible)>0);
}
void juntarTodosLosEntrenadores(t_list* posiblesEntrenadoresEnDeadlock) {
	sizeTrainersOnMap=list_size(entrenadoresListos)+list_size(entrenadoresBloqueados);
			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresListos);
			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresBloqueados);

}
t_list* analizarDeadlock (){
	int i;
	for (i=0;i<sizeTrainersOnMap;i++){
				t_entrenador* posibleEntrenadorEnDeadlock=malloc(sizeof(t_entrenador));
				posibleEntrenadorEnDeadlock=(t_entrenador*)list_get(posiblesEntrenadoresEnDeadlock,i);
				if (noHayPokemonsDisponiblesPara(posibleEntrenadorEnDeadlock) && poseePokemonSolicitado(posibleEntrenadorEnDeadlock) ){
					list_add(entrenadoresEnDeadlock,posibleEntrenadorEnDeadlock);
				}
			}
	return entrenadoresEnDeadlock;
}
bool hayEntrenadoresEnDeadlock(t_list* entrenadores){
	return list_size(entrenadoresEnDeadlock)>0;
}
bool batallaActivada(){
	return mapa->batalla==1;
}

void* deadlock(void* arg){
	log_trace(archivoLog, "Deadldock - Arranca");
	while (1){
		t_list* entrenadoresParaAnalizar=malloc(sizeof(t_entrenador));
		t_list* entrenadoresAnalizados=malloc(sizeof(t_entrenador));
		sleep(mapa->retardo);
		juntarTodosLosEntrenadores(entrenadoresParaAnalizar);
		entrenadoresAnalizados=analizarDeadlock(entrenadoresParaAnalizar);
		if (hayEntrenadoresEnDeadlock(entrenadoresParaAnalizar)){
			if (batallaActivada()){
				//BATALLA PKMN
		}else{
		log_info(archivoLog,"Hay DEADLOCK pero no hay batalla pokemon configurada");
		}
			free(posiblesEntrenadoresEnDeadlock);
			free(entrenadoresParaAnalizar);
				free(entrenadoresAnalizados);
	}
	}

	return arg;
}

//Estructura para la espera
typedef struct
{
	uint8_t * const espera;
	int head;
	int tail;
	const int maxLen;
}esperaCircular_t;


//Funcion de la espera
int esperaCircularPush(esperaCircular_t *c, uint8_t data)
{
	int next = c->head + 1;
	if (next >= c->maxLen)
		next = 0;
	// Si esta llena
	if (next == c->tail)
		return -1;  // error
	c->espera[c->head] = data;
	c->head = next;
	return 0;
}

int esperaCircularPop(esperaCircular_t *c, uint8_t *data)
{
	// si la cabeza no esta adelante de la cola
	if (c->head == c->tail)
		return -1;  // error
	*data = c->espera[c->tail];
	c->espera[c->tail] = 0;  // limpia espera(no es necesario)
	int next = c->tail + 1;
	if(next >= c->maxLen)
		next = 0;
	c->tail = next;
	return 0;
}

