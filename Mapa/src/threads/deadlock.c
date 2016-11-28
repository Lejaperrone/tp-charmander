#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <nivel.h>
#include <pkmn/battle.h>
#include <sys/socket.h>
#include "../socketLib.h"
#include "../commons/structures.h"
#include "../functions/recursos.h"
#include "../functions/collections_list_extension.h"
#include "../threads/planificador.h"

//Variables globales
t_list* entrenadoresDeadlock;

int** mMaximos;
int** mAsignacion;
int** mNecesidad;
int* vDisponibles;
int* resultadoDeteccion;
int cantEntrenadores,cantPokenests;
extern pthread_mutex_t mutexEntrBQ;

int countPokemonsCapturadosFromPokenest(t_entrenador* entrenador, char identificador){
	bool _validarTipoPokemon(t_pokemon_custom* pokemon){
		return pokemon->id==identificador;
	}

	return list_count_satisfying(entrenador->pokemons, (void*)_validarTipoPokemon);
}
int pokemonsDisponibles(t_pokenest* pokenest){
	bool _validarDisponibilidadPokemon(t_pokemon_custom* pokemon){
		return pokemon->disponible==1;
	}

	return list_count_satisfying(pokenest->pokemons, (void*)_validarDisponibilidadPokemon);
}
void llenarEstructuras(){
	int i,j, iteracion;

	cantEntrenadores = list_size(entrenadoresBloqueados); //Filas Matriz
	cantPokenests = list_size(mapa->pokeNests); //Columnas Matriz

	//Matriz de maximos
	mMaximos = (int**)malloc(cantEntrenadores*sizeof(int*));
	for(i=0; i<cantEntrenadores; i++){
		mMaximos[i] = (int*)malloc(cantPokenests*sizeof(int));

		t_entrenador* entrenador = (t_entrenador*)list_get(entrenadoresBloqueados, i);
		for(j=0;j<cantPokenests;j++){
			char idPokenest = ((t_pokenest*)list_get(mapa->pokeNests,j))->identificador;
			mMaximos[i][j] = countPokemonsCapturadosFromPokenest(entrenador, idPokenest);
			if(entrenador->pokenestBloqueante->identificador==idPokenest){
				mMaximos[i][j]++;
			}
		}
	}

	//Matriz de asignacion
	mAsignacion = (int**)malloc(cantEntrenadores*sizeof(int*));
	for(i=0; i<cantEntrenadores; i++){
		mAsignacion[i] = (int*)malloc(cantPokenests*sizeof(int));

		t_entrenador* entrenador = list_get(entrenadoresBloqueados, i);
		for(j=0;j<cantPokenests;j++){
			char idPokenest = ((t_pokenest*)list_get(mapa->pokeNests,j))->identificador;
			mAsignacion[i][j] = countPokemonsCapturadosFromPokenest(entrenador, idPokenest);

		}
	}

	//Matriz de necesitadad - No se llena de infor, se hace dentro del algoritmo de deteccion
	mNecesidad=(int**)malloc(cantEntrenadores*sizeof(int*));
	for(iteracion=0; iteracion<cantEntrenadores; iteracion++){
		mNecesidad[iteracion] = (int*)malloc(cantPokenests*sizeof(int));
	}

	//Vector de disponibles
	vDisponibles=(int*)malloc(sizeof(int));
	for(j=0;j<cantPokenests;j++){
		vDisponibles[j]= pokemonsDisponibles(list_get(mapa->pokeNests,j));
	}

	//Vector de resultados
	resultadoDeteccion=(int*)malloc(cantEntrenadores*sizeof(int));
}

void informarDeadlock (t_entrenador* entrenador){
	char m='K';
	if(send(entrenador->socket,&m,1,0)==1){
		log_info(archivoLog,"Informo deadlock a %c ",entrenador->simbolo);
	}else{
		log_info(archivoLog,"No fue posible informar muerte a %c ",entrenador->simbolo);
	}
}
void algoritmoDeDeteccion(){
	int flagDeDeteccionDeDeadlock=1,k;

	int i,j;
	for(i=0;i<cantEntrenadores;i++){
		resultadoDeteccion[i]=0;
	}
	//find mNecesidad matrix
	for(i=0;i<cantEntrenadores;i++){
		for(j=0;j<cantPokenests;j++){
			mNecesidad[i][j]=mMaximos[i][j]-mAsignacion[i][j];
		}
	}
	while(flagDeDeteccionDeDeadlock){
		flagDeDeteccionDeDeadlock=0;
		for(i=0;i<cantEntrenadores;i++){
			int c=0;
			for(j=0;j<cantPokenests;j++){
				if((resultadoDeteccion[i]==0)&&(mNecesidad[i][j]<=vDisponibles[j])){
					c++;
					if(c==cantPokenests){
						for(k=0;k<cantPokenests;k++){
							vDisponibles[k]+=mAsignacion[i][j];
							resultadoDeteccion[i]=1;
							flagDeDeteccionDeDeadlock=1;
						}
						if(resultadoDeteccion[i]==1){
							i=cantEntrenadores;
						}
					}
				}
			}
		}
	}

	flagDeDeteccionDeDeadlock=0;
	for(i=0;i<cantEntrenadores;i++){
		if(resultadoDeteccion[i]==0){
			t_entrenador* entrenadorDeadlock=(t_entrenador*)list_get(entrenadoresBloqueados,i);
			list_add(entrenadoresDeadlock,entrenadorDeadlock);
			flagDeDeteccionDeDeadlock=1;
			log_info(archivoLog,"El entrenador %c esta en deadlock",entrenadorDeadlock->simbolo);
			informarDeadlock(entrenadorDeadlock);
		}
	}

}

t_pokemon_custom* obtenerPokemonMasFuerte(t_entrenador* entrenador){
	int i;
	t_pokemon_custom* pokemonMasFuerte=(t_pokemon_custom*)list_get(entrenador->pokemons,0);
	for (i=1;i<list_size(entrenador->pokemons);i++){
		t_pokemon_custom* pokemon=(t_pokemon_custom*)list_get(entrenador->pokemons,i);
		if (pokemon->nivel>pokemonMasFuerte->nivel){
			pokemonMasFuerte=pokemon;
		}
	}

	return pokemonMasFuerte;
}
void batalla(){
	if (mapa->batalla==1){
		int cantEntrenadoresDeadlock=list_size(entrenadoresDeadlock);
		if (cantEntrenadoresDeadlock>1){

			t_pkmn_factory* pokemon_factory = create_pkmn_factory();
			int numBatalla;
			int vNivelBatalla[2];
			t_entrenador* entrenador1=(t_entrenador*)list_get(entrenadoresDeadlock,0);

			log_info(archivoLog,"Deadlock - Tendremos %d batallas pokemon",entrenadoresDeadlock);

			for (numBatalla=1;numBatalla<cantEntrenadoresDeadlock;numBatalla++){
				log_info(archivoLog,"Entrenador a pelear: %c",entrenador1->simbolo);
				t_entrenador* entrenador2=(t_entrenador*)list_get(entrenadoresDeadlock,numBatalla);
				log_info(archivoLog,"Deadlock - Entrendor a pelear con %c es: %c",entrenador1->simbolo,entrenador2->simbolo);

				t_pokemon_custom* pokemon1 = obtenerPokemonMasFuerte(entrenador1);
				t_pokemon_custom* pokemon2 = obtenerPokemonMasFuerte(entrenador2);

				log_info(archivoLog, "Nombre poke1: %s", pokemon1->nombre);
				log_info(archivoLog, "Nombre poke2: %s", pokemon2->nombre);

				t_pokemon * poke1 = create_pokemon(pokemon_factory, pokemon1->nombre, pokemon1->nivel);
				t_pokemon * poke2 = create_pokemon(pokemon_factory, pokemon2->nombre, pokemon2->nivel);

				vNivelBatalla[0]=poke1->level;
				vNivelBatalla[1]=poke2->level;

				t_pokemon* unPoke=pkmn_battle(poke1,poke2);

				if (unPoke->level==vNivelBatalla[0]){
					log_info(archivoLog,"El entrenador que perdio es %c",entrenador1->simbolo);
				}else{
					entrenador1=entrenador2;
					log_info(archivoLog,"El entrenador que perdio es %c", entrenador2->simbolo);
				}
			}

			log_info(archivoLog,"El entrenador %c sera elegido como victima", entrenador1->simbolo);

			bool _encontrarEntrenador(t_entrenador* entrenador){
				return entrenador->simbolo==entrenador1->simbolo;
			}
			list_remove_by_condition(entrenadoresBloqueados, (void*)_encontrarEntrenador);
			list_add(garbageCollectorEntrenadores, entrenador1);
		}
	}else{
		log_info(archivoLog,"No esta activada la batalla Pokemon.");
	}
}

void liberarMemoria(){
	int i;

	for(i=0; i<cantEntrenadores; i++){
		free(mMaximos[i]);
		free(mAsignacion[i]);
		free(mNecesidad[i]);
	}

	free(mMaximos);
	free(mAsignacion);
	free(mNecesidad);
	free(vDisponibles);
	free(resultadoDeteccion);
}

void* deadlock(void* arg){
	while (1){
		sleep(2);

		if (list_size(entrenadoresBloqueados)>1){
			pthread_mutex_lock(&mutexEntrBQ);

			entrenadoresDeadlock = list_create();
			llenarEstructuras();
			algoritmoDeDeteccion();
			batalla();
			liberarMemoria();
			list_destroy(entrenadoresDeadlock);
			pthread_mutex_unlock(&mutexEntrBQ);
		}
	}
	return NULL;
}
