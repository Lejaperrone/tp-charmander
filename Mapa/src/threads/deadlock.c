#include "../commons/structures.h"
#include <pkmn/battle.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <nivel.h>
#include "../functions/recursos.h"
#include "../functions/collections_list_extension.h"
#include "../threads/planificador.h"
#include <sys/socket.h>
#include "../socketLib.h"

//Variables globales
t_list* entrenadoresBloqueados;
t_list* entrenadoresEnDeadlock;


int** mMaximos;
int** mAsignacion;
int** mNecesidad;
int* vPokeDisponibles;
int* finish;
int cantDeEntrenadores,cantDeRecursosDePokemons;
extern pthread_mutex_t mutexEntrBQ;

//Encabezados de funciones
void llenarMatricesYVectores();
void algoritmoDeDeteccion();
int tiene_estos_pokemons(t_list* pokemons, char* id_pokenest){
	int tam = list_size(pokemons);
	int i,totAsignado;
	totAsignado = 0;
	for (i=0;i<tam;i++){
		t_pokemon_custom* unPoke=(t_pokemon_custom*)list_get(pokemons,i);
		if (unPoke->identificadorPokenest==id_pokenest[0]){
			totAsignado++;
		}
	}
	return totAsignado;
}
char* nombreDelPokeMasFuerte(t_entrenador* unE){
	int cantPokemonsQuePosee=list_size(unE->pokemons);
	int numPoke;
	t_pokemon_custom* pokeMasFuerte=(t_pokemon_custom*)list_get(unE->pokemons,0);
	for (numPoke=0;numPoke<cantPokemonsQuePosee;numPoke++){
		t_pokemon_custom* pokeDePrueba=(t_pokemon_custom*)list_get(unE->pokemons,numPoke);
		if (pokeDePrueba->nivel>pokeMasFuerte->nivel){
			pokeMasFuerte=pokeDePrueba;
		}
	}

	log_info(archivoLog,"El pokemon mas fuerte de %c es %s",unE->simbolo,pokeMasFuerte->nombre);
	return pokeMasFuerte->nombre;
}

int nivelDelPokeMasFuerte(t_entrenador* unE){
	int cantPokemonsQuePosee=list_size(unE->pokemons);
	int numPoke;
	t_pokemon_custom* pokeMasFuerte=(t_pokemon_custom*)list_get(unE->pokemons,0);
	for (numPoke=0;numPoke<cantPokemonsQuePosee;numPoke++){
		t_pokemon_custom* pokeDePrueba=(t_pokemon_custom*)list_get(unE->pokemons,numPoke);
		if (pokeDePrueba->nivel>pokeMasFuerte->nivel){
			pokeMasFuerte=pokeDePrueba;
		}
	}
	return pokeMasFuerte->nivel;
}
bool hayEntrenadoresEnDeadlock(){
	return (list_size(entrenadoresEnDeadlock)>1);
}

bool entrenadorTieneA(t_entrenador* unE,t_pokemon_custom* unP){
	int cantPokemonsQuetiene=list_size(unE->pokemons);
	int numPoke;
	bool loTiene=false;
	for (numPoke=0;numPoke<cantPokemonsQuetiene;numPoke++){
		t_pokemon_custom* pokePrueba=(t_pokemon_custom*)list_get(unE->pokemons,numPoke);
		if (pokePrueba==unP){
			loTiene=true;
		}
	}
	return loTiene;
}
t_entrenador* buscarDuenioDe(t_pokemon_custom* unP){
	int cantEntrenadoresInterbloqueados=list_size(entrenadoresEnDeadlock);
	int numEntrenador;
	t_entrenador* entrenadorDuenio;
	for(numEntrenador=0;numEntrenador<cantEntrenadoresInterbloqueados;numEntrenador++){
		t_entrenador* unE=(t_entrenador*)list_get(entrenadoresEnDeadlock,numEntrenador);
		if (entrenadorTieneA(unE,unP)){
			entrenadorDuenio=unE;
		}
	}
	return entrenadorDuenio;
}

bool batallaActivada(){
	return mapa->batalla==1;
}
void devolverPokemons(t_entrenador* unE){
	int cantPokemonsQueTenia=list_size(unE->pokemons);
	int i;
	for (i=0;i<cantPokemonsQueTenia;i++){
		t_pokemon_custom* unPoke=(t_pokemon_custom*)list_get(unE->pokemons,i);
		sumarRecurso(elementosUI,unPoke->identificadorPokenest);
		t_pokenest* pokenestActualizar=find_pokenest_by_id(unPoke->identificadorPokenest);
		unPoke->disponible=1;
		list_add(pokenestActualizar->pokemons,unPoke);
	}
}

int find_index_trainer_on_blocked(t_entrenador* unE){
	int cantEntrenadoresBloqueados=list_size(entrenadoresBloqueados);
	int i;
	int index=0;
	for (i=0;i<cantEntrenadoresBloqueados;i++){
		t_entrenador* e=(t_entrenador*)list_get(entrenadoresBloqueados,i);
		if (e->simbolo==unE->simbolo){
			index=i;
		}
	}
	return index;
}

int vNivelBatalla[2];
void resolverNecesidades(t_entrenador* unE){
	list_add(garbageCollectorEntrenadores, unE);

}
void informarMuerteAlEntrenador (t_entrenador* unE){
	char m='K';
	send(unE->socket,&m,1,0);
	log_info(archivoLog,"Informo muerte a %c ",unE->simbolo);
}

void batallaPokemon(){
	if (batallaActivada()){
		if (hayEntrenadoresEnDeadlock()){
			t_pkmn_factory* pokemon_factory = create_pkmn_factory();
			int cantEntrenadoresEnDeadlock=list_size(entrenadoresBloqueados);
			int numBatalla;
			t_entrenador* unE=(t_entrenador*)list_get(entrenadoresBloqueados,0);

			for (numBatalla=1;numBatalla<cantEntrenadoresEnDeadlock;numBatalla++){
				log_info(archivoLog,"Deadlock - Habra %d batallas pokemon",cantEntrenadoresEnDeadlock-1);
				log_info(archivoLog,"Entrenador a pelear: %c",unE->simbolo);
				t_entrenador* otroE=(t_entrenador*)list_get(entrenadoresBloqueados,numBatalla);
				log_info(archivoLog,"Deadlock - Entrendor a pelear con %c es: %c",unE->simbolo,otroE->simbolo);
				//Convierto los pokemons a t_pokemon
				t_pokemon * poke1 = create_pokemon(pokemon_factory, nombreDelPokeMasFuerte(unE), nivelDelPokeMasFuerte(unE));
				vNivelBatalla[0]=poke1->level;
				t_pokemon * poke2 = create_pokemon(pokemon_factory, nombreDelPokeMasFuerte(otroE), nivelDelPokeMasFuerte(otroE));
				vNivelBatalla[1]=poke2->level;
				//Batalla propiamente dicha
				t_pokemon* unPoke=pkmn_battle(poke1,poke2);
				log_info(archivoLog,"Se ha llevado a cabo la batalla pokemon");
				if (unPoke->level==vNivelBatalla[0]){
					log_info(archivoLog,"El entrenador que perdio es %c",unE->simbolo);
				}else{
					unE=otroE;
					log_info(archivoLog,"El entrenador que perdio es %c", otroE->simbolo);
				}
			}
			log_info(archivoLog,"El entrenador %c sera elegido como victima", unE->simbolo);
			informarMuerteAlEntrenador(unE);
			list_remove(entrenadoresBloqueados,find_index_trainer_on_blocked(unE));
			BorrarItem(elementosUI,unE->simbolo);
			nivel_gui_dibujar(elementosUI,mapa->nombre);
			devolverPokemons(unE);
			resolverNecesidades(unE);
		}
	}else{
		log_info(archivoLog,"No esta activada la batalla Pokemon.");
	}
}

int pokemonsDisponiblesPara (t_pokenest* p){
	int cantPoke;
	int cant = 0;
	for (cantPoke=0;cantPoke<list_size(p->pokemons);cantPoke++){
		t_pokemon_custom* pk=(t_pokemon_custom*)list_get(p->pokemons,cantPoke);
		if (pk->disponible == 1){
			cant++;
		}

	}

	return cant;
}

void liberarMemoriaMatrices(){
	pthread_mutex_unlock(&mutexEntrBQ);
	int iteracion;

	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		int* punteroDeMaximos = mMaximos[iteracion];
		free(punteroDeMaximos);
	}

	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		int* punteroDeAsignacion = mAsignacion[iteracion];
		free(punteroDeAsignacion);
	}
	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		int* punteroDeNecesidad = mNecesidad[iteracion];
		free(punteroDeNecesidad);
	}

	free(vPokeDisponibles);
}
void* deadlock(void* arg){
	while (1){
		sleep(2);

		if (list_size(entrenadoresBloqueados)>1){
			pthread_mutex_lock(&mutexEntrBQ);
			log_info(archivoLog,"Deadlock - Creo lista de entrenadores bloqueados");
			entrenadoresEnDeadlock = list_create();
			log_info(archivoLog,"Deadlock - Creo matrices y vectores");
			llenarMatricesYVectores();
			log_info(archivoLog,"Deadlock - Inicio algoritmo de deteccion");
			algoritmoDeDeteccion();
			batallaPokemon();
			liberarMemoriaMatrices();
		}else{


		}
	}
	return arg;
}

int _contarCantDePokemonsDelEntrenadorParaEstaColumna(t_entrenador* entrenador, char* identificadorPokenest){
	int cantDeEstePokemon = 0;
	if (!list_is_empty(entrenador->pokemons)){
		cantDeEstePokemon = tiene_estos_pokemons(entrenador->pokemons,identificadorPokenest);
		if(strcmp(&(entrenador->pokenestBloqueante->identificador),identificadorPokenest) == 0){
			cantDeEstePokemon++;
		}
	}
	return cantDeEstePokemon;
}
void llenarMatricesYVectores(){


	//Variables que uso en la funcion
	int i,j, iteracion;

	//INICIALIZACION DE MATRICES Y ALOCACION DE MEMORIA
	//Filas para las matrices
	cantDeEntrenadores = list_size(entrenadoresBloqueados);

	//Columnas para las matrices
	cantDeRecursosDePokemons = list_size(mapa->pokeNests);

	//Alocacion de memoria para matriz de maximos
	mMaximos = (int**)malloc(cantDeEntrenadores*sizeof(int*));

	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		mMaximos[iteracion] = (int*)malloc(cantDeRecursosDePokemons*sizeof(int));
	}


	mAsignacion = (int**)malloc(cantDeEntrenadores*sizeof(int*));

	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		mAsignacion[iteracion] = (int*)malloc(cantDeRecursosDePokemons*sizeof(int));
	}
	//Alocacion de memoria para matriz de necesidad
	mNecesidad=(int**)malloc(cantDeEntrenadores*sizeof(int*));
	for(iteracion=0; iteracion<cantDeEntrenadores; iteracion++){
		mNecesidad[iteracion] = (int*)malloc(cantDeRecursosDePokemons*sizeof(int));
	}


	//-------------------------LLENADO DE LAS MATRICES-----------------------------------

	//Para matrices de peticiones maximas
	for(i=0;i<cantDeEntrenadores;i++){
		t_entrenador* entrenador = (t_entrenador*)list_get(entrenadoresBloqueados, i);
		for(j=0;j<cantDeRecursosDePokemons;j++){
			char* idPokenest = (char*)list_get(listaDeIdentificadoresDePokenests,j);
			mMaximos[i][j] = _contarCantDePokemonsDelEntrenadorParaEstaColumna(entrenador, idPokenest);
		}
	}

	//Para matriz de asignacion
	for(i=0;i<cantDeEntrenadores;i++){

		t_entrenador* entrenador = list_get(entrenadoresBloqueados, i);
		for(j=0;j<cantDeRecursosDePokemons;j++){
			char* idPokenest = (char*)list_get(listaDeIdentificadoresDePokenests,j);
			mAsignacion[i][j]=tiene_estos_pokemons(entrenador->pokemons,idPokenest);

		}
	}

	//Para vector de recursos disponibles
	vPokeDisponibles=(int*)malloc(sizeof(int));
	for(j=0;j<cantDeRecursosDePokemons;j++){

		vPokeDisponibles[j]= pokemonsDisponiblesPara(list_get(mapa->pokeNests,j));
	}
	//Para vector de terminados
	finish=(int*)malloc(cantDeEntrenadores*sizeof(int));


}

void algoritmoDeDeteccion(){
	int flagDeDeteccionDeDeadlock=1,k;

	//int safe[100];
	int i,j;
	for(i=0;i<cantDeEntrenadores;i++){
		finish[i]=0;
	}
	//find mNecesidad matrix
	for(i=0;i<cantDeEntrenadores;i++){
		for(j=0;j<cantDeRecursosDePokemons;j++){
			mNecesidad[i][j]=mMaximos[i][j]-mAsignacion[i][j];
		}
	}
	while(flagDeDeteccionDeDeadlock){
		flagDeDeteccionDeDeadlock=0;
		for(i=0;i<cantDeEntrenadores;i++){
			int c=0;
			for(j=0;j<cantDeRecursosDePokemons;j++){
				if((finish[i]==0)&&(mNecesidad[i][j]<=vPokeDisponibles[j])){
					c++;
					if(c==cantDeRecursosDePokemons){
						for(k=0;k<cantDeRecursosDePokemons;k++){
							vPokeDisponibles[k]+=mAsignacion[i][j];
							finish[i]=1;
							flagDeDeteccionDeDeadlock=1;
						}
						if(finish[i]==1){
							i=cantDeEntrenadores;
						}
					}
				}
			}
		}
	}
	j=0;
	flagDeDeteccionDeDeadlock=0;
	for(i=0;i<cantDeEntrenadores;i++){
		if(finish[i]==0){
			t_entrenador* unE_en_deadlock=(t_entrenador*)list_get(entrenadoresBloqueados,i);
			list_add(entrenadoresEnDeadlock,unE_en_deadlock);
			j++;
			flagDeDeteccionDeDeadlock=1;
			log_info(archivoLog,"El entrenador %c esta en deadlock",unE_en_deadlock->simbolo);
		}
	}

}
