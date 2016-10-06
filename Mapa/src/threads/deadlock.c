/*
 * deadlock.c
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#include "../commons/structures.h"
#include <pkmn/battle.h>
#include <unistd.h>


t_list* entrenadoresEnDeadlock;
t_list* posiblesEntrenadoresEnDeadlock;
int sizeTrainersOnMap;
int **mAsignacion;
int **mNecesidad;

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
bool esEsePokemon(t_pokemon_custom* p1, t_pokemon_custom* p2){
	return p1->id==p2->id;
}
bool loTieneAsignado(t_entrenador* e, t_list* p){
	t_list* lpok=list_create();
	list_add_all(lpok,e->pokemons);
	return list_any_satisfy(lpok, (void*)esEsePokemon );
}
int asignarCantidadDePokemons (int entr, int poke){
	t_entrenador* entrAsig=(t_entrenador*)malloc(sizeof(t_entrenador));
	entrAsig=list_get(entrenadoresBloqueados,entr);
	t_pokenest* pokenestAsig=(t_pokenest*)malloc(sizeof(t_pokenest));
	pokenestAsig=(t_pokenest*)list_get(mapa->pokeNests,poke);

	return list_count_satisfying(pokenestAsig->pokemons, (void*)loTieneAsignado);
}
int noHayEntrenadoresBloqueados(){
	return list_is_empty(entrenadoresBloqueados);
}
void crearMatrizAsignacion(){
	int numEntrenador;
mAsignacion=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
		for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
			mAsignacion[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
		}
}

void crearMatrizNecesidad(){
	int numEntrenador;
	mNecesidad=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
			for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
				mNecesidad[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
			}
}
void completarMatrizAsignacion(){
	int numEntrenador;
	int numPokenest;
	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
				for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
				mAsignacion[numEntrenador][numPokenest]=asignarCantidadDePokemons(numEntrenador,numPokenest);
				//Esto que sigue es solo para verificar que todo este bien cargado
				t_entrenador* e = (t_entrenador*)malloc(sizeof(t_entrenador));
				e=list_get(entrenadoresBloqueados,numEntrenador);
				t_pokenest* p=(t_pokenest*)malloc(sizeof(t_pokenest));
				p=list_get(mapa->pokeNests,numPokenest);
				log_info(archivoLog,"%c tiene asignados %d %c",e->simbolo,mAsignacion[numEntrenador][numPokenest],p->identificador );
				free(e);
				free(p);
				}
			}


}
int asignarNecesidad(int numEntrenador, int numPokenest){
	t_entrenador* e = (t_entrenador*)malloc(sizeof(t_entrenador));
	e=list_get(entrenadoresBloqueados,numEntrenador);
	t_pokenest* p=(t_pokenest*)malloc(sizeof(t_pokenest));
	p=list_get(mapa->pokeNests,numPokenest);
	if (e->pokenestBloqueante->identificador==p->identificador){
		free(e);
		free(p);
		return 1;
	}else{
		free(e);
		free(p);
		return 0;
	}
}
void completarMatrizNecesidad(){
	int numEntrenador;
	int numPokenest;
	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
		for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
			mNecesidad[numEntrenador][numPokenest]=asignarNecesidad(numEntrenador,numPokenest);
		}
	}
}
void* deadlock(void* arg){
	log_trace(archivoLog, "Deadldock - Arranca");
	while (1){
		log_info(archivoLog,"Hilo deadlock espera proxima vez de chequeo");
		//sleep(0.1*(mapa->retardo));
		sleep(2);
		log_info(archivoLog,"Hilo deadlock comienza su deteccion");
		int i,j;
		if (noHayEntrenadoresBloqueados()){
					log_info(archivoLog,"No hay entrenadores bloqueados aun");
				}else{

		//REFACTORIZAR TODO ESTE ELSE EN FUNCIONES MAS CHICAS
		t_list* entrenadoresParaAnalizar=malloc(sizeof(t_entrenador));
		t_list* entrenadoresAnalizados=malloc(sizeof(t_entrenador));
		log_info(archivoLog,"Se crea la matriz de asignacion de pokemons ");
		crearMatrizAsignacion();
		log_info(archivoLog,"Comienzo a llenar la matriz ");
		completarMatrizAsignacion();
		//Logueo la matriz
		for (i=0;i<list_size(entrenadoresBloqueados);i++){
					for (j=0;j<list_size(mapa->pokeNests);j++){
					log_info(archivoLog,"%d ",mAsignacion[i][j]);
					}
				}
		log_info(archivoLog,"Se crea la matriz de necesidad");
		crearMatrizNecesidad();
		log_info(archivoLog,"Comienzo a llenar la matriz de necesidad");
		completarMatrizNecesidad();

		//juntarTodosLosEntrenadores(entrenadoresParaAnalizar);
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


/*
//falta asociar a la fila y columna con la estructura en mapas (t_list* entrenadoresBloqueados y el int cantidad de pokemonesAsignados)
typedef struct matrizAsignada {
	int fila_entrenadoresBloqueados;
	int columna_PokeAsignados;
	int **matriz;
	matriz = new int *[fila_entrenadoresBloqueados];
	for(int i=0; i<fila_entrenadoresBloqueados; i++);
	matriz[i] = new int[columna_PokeAsignados];
}t_matriz_asignada;
*/
