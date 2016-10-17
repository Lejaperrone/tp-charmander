#include "../commons/structures.h"
#include <pkmn/battle.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <nivel.h>
#include "../functions/recursos.h"
#include "../functions/collections_list_extension.h"
#include "../threads/planificador.h"

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

	/*int totalBloqueados=list_size(entrenadoresBloqueados);
	log_info(archivoLog,"Hay %d entrenadores bloqueados",totalBloqueados);
	int i;
	for (i=0;i<totalBloqueados;i++){
		t_entrenador* unE=(t_entrenador*)list_get(entrenadoresBloqueados,i);*/
		list_add(garbageCollectorEntrenadores, unE);

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
		list_remove(entrenadoresBloqueados,find_index_trainer_on_blocked(unE));
		BorrarItem(elementosUI,unE->simbolo);
		nivel_gui_dibujar(elementosUI,mapa->nombre);
		devolverPokemons(unE);
		resolverNecesidades(unE);
	}
	}else{
		log_info(archivoLog,"No esta la batalla Pokemon activada");
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
						//printf("\nP%d",i);
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

	//Aca en lugar de printear deberia:

		//1) agregarlos a la lista de entrenadoresEnDeadlock
		//2) loguear los entrenadores involucrados en el deadlock, y loguear las matrices utilizadas (lo pide el tp)

}







//---------------------------------------DEADLOCK ANTERIOR----------------------------------------------
//------------------------------------------------------------------------------------------------------
//t_list* entrenadoresEnDeadlock;
//t_list* posiblesEntrenadoresEnDeadlock;
//int sizeTrainersOnMap;
//int **mAsignacion;
//int **mNecesidad;
//int *vecPokeDisp;
//typedef struct entrPosiblesDeadlock{
//	t_entrenador* e;
//	int status;
//}t_entrPosibleDeadlock;
//
//
//extern pthread_mutex_t mutexEntrBQ;
//
////bool(*condition)(void*)
//
//bool estaDisponible (t_pokemon_custom* unPoke){
//	return (unPoke->disponible==1);
//}
//
//bool noHayPokemonsDisponiblesPara(t_entrenador* unE, int** vecDisp){
//	int numPokenest;
//	bool noHayDisp=false;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		if (unE->pokenestBloqueante==(t_pokenest*)list_get(mapa->pokeNests,numPokenest)){
//			if (*vecDisp[numPokenest]<=0){
//				noHayDisp=true;
//			}
//		}
//	}
//	return noHayDisp;
//}
//
//bool poseePokemonSolicitado (t_entrenador* unE){
//
//	return (list_count_satisfying(unE->pokemons,(void*)estaDisponible)>0);
//}
//void juntarTodosLosEntrenadores(t_list* posiblesEntrenadoresEnDeadlock) {
//	sizeTrainersOnMap=list_size(entrenadoresListos)+list_size(entrenadoresBloqueados);
//			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresListos);
//			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresBloqueados);
//
//}
//int find_pokenest_on_map(char idPokenest){
//	int size_pokenests_on_map=list_size(mapa->pokeNests);
//	int i,pos=-1;
//	for (i=0;i<size_pokenests_on_map;i++){
//		t_pokenest* unaP=list_get(mapa->pokeNests,i);
//		if (unaP->identificador==idPokenest){
//			pos=i;
//		}
//	}
//	return pos;
//}
//void verificarNecesidadDe(int posTrainer, char id_poke_to_cmp, t_entrenador* e_to_cmp){
//	int i;
//	int pknst=find_pokenest_on_map(id_poke_to_cmp);
//	for (i=0;i<list_size(entrenadoresBloqueados);i++){
//		if (mNecesidad[i][pknst]==1){
//			t_entrenador* e=(t_entrenador*)list_get(entrenadoresBloqueados,i);
//			e_to_cmp=e;
//		}
//	}
//}
//bool chequearAsignacionAjena (int id, t_list*posiblesEntrsEnDL, t_entrenador* trainer_to_compare,t_entrenador*orig_tr){
//	int total_trainers_to_check=list_size(entrenadoresBloqueados);
//	//log_info(archivoLog,"Deadlock - La cantidad de entrenadores bloqueados para chequear asignacion ajena es: %d", total_trainers_to_check);
//	int i=0;
//	bool otroTieneElPokeQueNecesito=false;
//	//log_info(archivoLog,"Deadlock - Chequeo que alguien tenga el pokemon solicitado");
//	for (i=0;i<total_trainers_to_check;i++){
//		log_info(archivoLog,"Deadlock - entro al for de asignaciones ajenas");
//		log_info(archivoLog,"Deadlock - Asignacion en %d %d: %d",i,id,mAsignacion[i][id]);
//		if (mAsignacion[i][id]>0){
//			log_info(archivoLog,"Deadlock - veo la asignacion de");
//			t_entrenador* trainer=(t_entrenador*)list_get(entrenadoresBloqueados,i);
//			log_info(archivoLog,"Deadlock - %c", trainer->simbolo);
//			if (trainer->simbolo!=orig_tr->simbolo){
//			log_info(archivoLog,"Obtengo entrenador %c", trainer->simbolo);
//			t_entrPosibleDeadlock* trainer_on_deadlock=(t_entrPosibleDeadlock*)malloc(sizeof(t_entrPosibleDeadlock));
//			trainer_on_deadlock->e=trainer;
//			trainer_on_deadlock->status=1;
//			trainer_to_compare->simbolo=trainer->simbolo;
//			//trainer_to_compare=trainer;
//			log_info(archivoLog,"trainer_to_compare es: %c",trainer_to_compare->simbolo);
//			list_add(posiblesEntrsEnDL,trainer_on_deadlock);
//			log_info(archivoLog,"Deadlock - agrego a %c como posible entrenador en deadlock",trainer_on_deadlock->e->simbolo);
//
//			//log_info(archivoLog,"Deadlock - el entrenador %c tiene a %c",(trainer_on_deadlock->e)->simbolo,((t_pokenest*)list_get(mapa->pokeNests,id))->identificador);
//
//			}else{}
//		otroTieneElPokeQueNecesito=true;
//		}
//	}
//	return otroTieneElPokeQueNecesito;
//}
//
//
//
//t_list* analizarDeadlock (){
//	log_info(archivoLog,"Deadlock - Hay %d entrenadoresBloqueados", list_size(entrenadoresBloqueados));
//	t_entrenador* entrenadorQueDeterminaDeadlock=(t_entrenador*)list_get(entrenadoresBloqueados,0);
//	log_info(archivoLog,"Leo al primer entrenador: %c", entrenadorQueDeterminaDeadlock->simbolo);
//	t_entrenador* trainer_to_compare=(t_entrenador*)malloc(sizeof(t_entrenador));
//	t_list* entrsPosiblesDeadlock=list_create();
//	//Elijo fin para cortar el barrido de entrenadores bloqueados
//	int noHayaEsperaCircular =1;
//	//Agrego al primer entrenador a los posibles "deadlock"
//	log_info(archivoLog,"Deadlock - Elijo a %c como entrenador determinante del deadlock", entrenadorQueDeterminaDeadlock->simbolo);
//	//Comienzo el barrido de los bloqueados
//	int size_trainers_maybe_on_deadlock,iEntrDeadlock;
//	size_trainers_maybe_on_deadlock=list_size(entrenadoresBloqueados);
//	log_info(archivoLog,"Deadlock - hay %d entrenadores para analizar", size_trainers_maybe_on_deadlock);
//	/*if ((size_trainers_maybe_on_deadlock<2)&&(noHayPokemonsDisponiblesPara(entrenadorQueDeterminaDeadlock, vecDisp))){
//		log_info(archivoLog,"Deadlock - El entrenador %c esta en inanicion",entrenadorQueDeterminaDeadlock->simbolo);
//		return entrsPosiblesDeadlock;
//	}else{*/
//		while (noHayaEsperaCircular==1){
//			for (iEntrDeadlock=0;iEntrDeadlock<size_trainers_maybe_on_deadlock;iEntrDeadlock++){
//				//Elijo al primer entrenador y me fijo si alguien mas tiene lo que necesita
//				t_entrenador* unE=list_get(entrenadoresBloqueados,iEntrDeadlock);
//				//log_info(archivoLog,"Deadlock - Se analiza el bloqueo de %c", unE->simbolo);
//				int pokeOnMatrix=find_pokenest_on_map(unE->pokenestBloqueante->identificador);
//				//log_info(archivoLog,"La pokenest en el mapa es la numero %d",pokeOnMatrix);
//				log_info(archivoLog,"Deadlock - La pokenest bloqueante de %c es: %c",unE->simbolo,unE->pokenestBloqueante->identificador);
//				chequearAsignacionAjena(pokeOnMatrix,entrsPosiblesDeadlock, trainer_to_compare,unE);
//				log_info(archivoLog,"el simbolo del actual %c",trainer_to_compare->simbolo);
//				log_info(archivoLog,"el simbolo del determinante %c",entrenadorQueDeterminaDeadlock->simbolo);
//				if (trainer_to_compare->simbolo==entrenadorQueDeterminaDeadlock->simbolo){
//					log_info(archivoLog,"Deadlock - Hay espera circular");
//					noHayaEsperaCircular =0;
//					}
//			}
//		}
//
//	return entrsPosiblesDeadlock;
//}
//bool hayEntrenadoresEnDeadlock(t_list* entrenadores){
//	if( list_is_empty(entrenadores)){
//		return false;
//	}else{
//		return true;
//	}
//}
//bool batallaActivada(){
//	return mapa->batalla==1;
//}
//
//int tiene_estos_pokemons(t_list* pokemons, char id_pokenest){
//	int tam=list_size(pokemons);
//	log_info(archivoLog,"Deadlock - Cuento la cantidad de pokmns que tiene un entrenador %d", tam);
//	int i,totAsignado;
//	totAsignado=0;
//	for (i=0;i<tam;i++){
//		log_info(archivoLog,"Deadlock - entro al foor de tiene_Estos_pokemons");
//		t_pokemon_custom* unPoke=(t_pokemon_custom*)list_get(pokemons,i);
//		log_info(archivoLog,"Deadlock - el pokemon actual es %c", unPoke->identificadorPokenest);
//		/*char vecAux[2];
//		vecAux[0]=unPoke->identificadorPokenest;
//		vecAux[1]=id_pokenest;*/
//		log_info(archivoLog,"Deadlock - unPoke vale %c",unPoke->identificadorPokenest);
//		log_info(archivoLog,"Deadlock - la pokenest vale %c",id_pokenest);
//		if (unPoke->identificadorPokenest==id_pokenest){
//			log_info(archivoLog,"Deadlock - Tiene asignado a %c", unPoke->identificadorPokenest);
//			totAsignado=totAsignado+1;
//		}
//	}
//	log_info(archivoLog,"Deadlock - Total asignado %d", totAsignado);
//	return totAsignado;
//}
//int asignarCantidadDePokemons (int entr, int poke){
//	t_entrenador* entrAsig=list_get(entrenadoresBloqueados,entr);
//	log_info(archivoLog,"Deadlock - Se asignara al Entrenador %c",entrAsig->simbolo);
//	t_pokenest* pokenestAsig=(t_pokenest*)list_get(mapa->pokeNests,poke);
//	log_info(archivoLog,"Deadlock - Se asignara el pokemon %c", pokenestAsig->identificador);
//	mAsignacion[entr][poke]=tiene_estos_pokemons(entrAsig->pokemons,pokenestAsig->identificador);
//	log_info(archivoLog,"Deadlock - asignacion: %d ",mAsignacion[entr][poke]);
//	log_info(archivoLog,"Deadlock - %c tiene %d pokemons de %c",entrAsig->simbolo,tiene_estos_pokemons(entrAsig->pokemons,pokenestAsig->identificador),pokenestAsig->identificador);
//	//log_info(archivoLog,"%c tiene %d pokemons de %c", entrAsig->simbolo, mAsignacion[entr][poke], pokenestAsig->identificador);
//	return mAsignacion[entr][poke];
//}
//int noHayEntrenadoresBloqueados(){
//	return list_is_empty(entrenadoresBloqueados);
//}
//void crearMatrizAsignacionParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	mAsignacion=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
//		for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//			mAsignacion[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
//		}
//}
//
//void crearMatrizNecesidadParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	mNecesidad=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
//			for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//				mNecesidad[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
//			}
//}
//void completarMatrizAsignacionParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	int numPokenest;
//	if (noHayEntrenadoresBloqueados()){
//		log_info(archivoLog,"No hay entrenadores bloqueados");
//	}else{
//		//log_info(archivoLog, "Hay %d entrenadoresBloqueados", list_size(entrenadoresBloqueados));
//		//log_info(archivoLog, "Hay %d pokenests", list_size(mapa->pokeNests));
//	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//				for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//				mAsignacion[numEntrenador][numPokenest]=asignarCantidadDePokemons(numEntrenador,numPokenest);
//				int asignado=mAsignacion[numEntrenador][numPokenest];
//				//log_info(archivoLog,"Deadlock - Lo asignado %d", asignado);
//				//Esto que sigue es solo para verificar que todo este bien cargado
//				//log_info(archivoLog,"ya se completo la matriz de asignacion para alguien");
//				t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
//				t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
//				log_info(archivoLog,"%c tiene asignados %d %c",e->simbolo,mAsignacion[numEntrenador][numPokenest],p->identificador);
//				}
//			}
//	}
//
//}
//int asignarNecesidad(int numEntrenador, int numPokenest){
//	t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
//	t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
//	if (e->pokenestBloqueante->identificador==p->identificador){
//		return 1;
//	}else{
//		return 0;
//	}
//}
//void completarMatrizNecesidadParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	int numPokenest;
//	if (noHayEntrenadoresBloqueados()){
//		log_info(archivoLog,"No hay entrenadoresbloqueados");
//	}else{
//	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//		for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//			mNecesidad[numEntrenador][numPokenest]=asignarNecesidad(numEntrenador,numPokenest);
//			log_info(archivoLog,"%c necesita %d de %c", ((t_entrenador*)list_get(entrenadoresBloqueados,numEntrenador))->simbolo, mNecesidad[numEntrenador][numPokenest],((t_pokemon_custom*)list_get(mapa->pokeNests,numPokenest))->id);
//		}
//	}
//	}
//}
//int pokemonsDisponiblesPara (t_pokenest* p){
//	int cantPoke;
//	int cant=0;
//	for (cantPoke=0;cantPoke<list_size(p->pokemons);cantPoke++){
//		t_pokemon_custom* pk=(t_pokemon_custom*)list_get(p->pokemons,cantPoke);
//		if (pk->disponible){
//			cant=cant+1;
//		}
//	}
//	log_info(archivoLog,"Deadlock - Hay %d pokemons disponibles de %c",cant,p->identificador);
//	return cant;
//}
//int * crearVectorPokemonsDisponibles(int *vecPokeDisp){
//	int numPokenest;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		vecPokeDisp[numPokenest]=pokemonsDisponiblesPara(list_get(mapa->pokeNests,numPokenest));
//	}
//	return vecPokeDisp;
//}
//void loguearVectorDisponibles(int *vecPokeDisp){
//	int numPokenest;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		t_pokenest* pknst=list_get(mapa->pokeNests,numPokenest);
//		vecPokeDisp[numPokenest]=list_size(pknst->pokemons);
//		log_info(archivoLog,"Deadlock - Hay %d disponible(s) para: %c",vecPokeDisp[numPokenest],pknst->identificador);
//	}
//}
//
//bool hayMasDe1EntrenadorBloqueado (){
//	return (list_size(entrenadoresBloqueados)>1);
//}
//void* deadlock(void* arg){
//	log_trace(archivoLog, "Deadldock - Arranca");
//	while (1){
//		sleep(2);
//		int tamPokenests=list_size(mapa->pokeNests);
//		int *vecPokeDisp=(int*)malloc(tamPokenests*sizeof(int));
//		log_info(archivoLog,"Deadlock - Hilo deadlock espera proxima vez de chequeo");
//		crearVectorPokemonsDisponibles(vecPokeDisp);
//		log_info(archivoLog,"Deadlock - Se crea la matriz de asignacion de pokemons para entrenadoresBloqueados ");
//		crearMatrizAsignacionParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz para los bloqueados ");
//		completarMatrizAsignacionParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Se crea la matriz de necesidad");
//			crearMatrizNecesidadParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz de necesidad");
//			completarMatrizNecesidadParaEntrenadoresBloqueados();
//
//		//sleep(0.1*(mapa->retardo));
//			log_info(archivoLog,"--------------------------------------------------------------------------");
//		log_info(archivoLog,"Deadlock - Hilo deadlock comienza su deteccion");
//
//		if (noHayEntrenadoresBloqueados()){
//					log_info(archivoLog,"Deadlock - No hay entrenadores bloqueados aun");
//			//t_list* entrenadoresEnDeadlock;
//t_list* posiblesEntrenadoresEnDeadlock;
//int sizeTrainersOnMap;
//int **mAsignacion;
//int **mNecesidad;
//int *vecPokeDisp;
//typedef struct entrPosiblesDeadlock{
//	t_entrenador* e;
//	int status;
//}t_entrPosibleDeadlock;
//
//
//extern pthread_mutex_t mutexEntrBQ;
//
////bool(*condition)(void*)
//
//bool estaDisponible (t_pokemon_custom* unPoke){
//	return (unPoke->disponible==1);
//}
//
//bool noHayPokemonsDisponiblesPara(t_entrenador* unE, int** vecDisp){
//	int numPokenest;
//	bool noHayDisp=false;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		if (unE->pokenestBloqueante==(t_pokenest*)list_get(mapa->pokeNests,numPokenest)){
//			if (*vecDisp[numPokenest]<=0){
//				noHayDisp=true;
//			}
//		}
//	}
//	return noHayDisp;
//}
//
//bool poseePokemonSolicitado (t_entrenador* unE){
//
//	return (list_count_satisfying(unE->pokemons,(void*)estaDisponible)>0);
//}
//void juntarTodosLosEntrenadores(t_list* posiblesEntrenadoresEnDeadlock) {
//	sizeTrainersOnMap=list_size(entrenadoresListos)+list_size(entrenadoresBloqueados);
//			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresListos);
//			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresBloqueados);
//
//}
//int find_pokenest_on_map(char idPokenest){
//	int size_pokenests_on_map=list_size(mapa->pokeNests);
//	int i,pos=-1;
//	for (i=0;i<size_pokenests_on_map;i++){
//		t_pokenest* unaP=list_get(mapa->pokeNests,i);
//		if (unaP->identificador==idPokenest){
//			pos=i;
//		}
//	}
//	return pos;
//}
//void verificarNecesidadDe(int posTrainer, char id_poke_to_cmp, t_entrenador* e_to_cmp){
//	int i;
//	int pknst=find_pokenest_on_map(id_poke_to_cmp);
//	for (i=0;i<list_size(entrenadoresBloqueados);i++){
//		if (mNecesidad[i][pknst]==1){
//			t_entrenador* e=(t_entrenador*)list_get(entrenadoresBloqueados,i);
//			e_to_cmp=e;
//		}
//	}
//}
//bool chequearAsignacionAjena (int id, t_list*posiblesEntrsEnDL, t_entrenador* trainer_to_compare,t_entrenador*orig_tr){
//	int total_trainers_to_check=list_size(entrenadoresBloqueados);
//	//log_info(archivoLog,"Deadlock - La cantidad de entrenadores bloqueados para chequear asignacion ajena es: %d", total_trainers_to_check);
//	int i=0;
//	bool otroTieneElPokeQueNecesito=false;
//	//log_info(archivoLog,"Deadlock - Chequeo que alguien tenga el pokemon solicitado");
//	for (i=0;i<total_trainers_to_check;i++){
//		log_info(archivoLog,"Deadlock - entro al for de asignaciones ajenas");
//		log_info(archivoLog,"Deadlock - Asignacion en %d %d: %d",i,id,mAsignacion[i][id]);
//		if (mAsignacion[i][id]>0){
//			log_info(archivoLog,"Deadlock - veo la asignacion de");
//			t_entrenador* trainer=(t_entrenador*)list_get(entrenadoresBloqueados,i);
//			log_info(archivoLog,"Deadlock - %c", trainer->simbolo);
//			if (trainer->simbolo!=orig_tr->simbolo){
//			log_info(archivoLog,"Obtengo entrenador %c", trainer->simbolo);
//			t_entrPosibleDeadlock* trainer_on_deadlock=(t_entrPosibleDeadlock*)malloc(sizeof(t_entrPosibleDeadlock));
//			trainer_on_deadlock->e=trainer;
//			trainer_on_deadlock->status=1;
//			trainer_to_compare->simbolo=trainer->simbolo;
//			//trainer_to_compare=trainer;
//			log_info(archivoLog,"trainer_to_compare es: %c",trainer_to_compare->simbolo);
//			list_add(posiblesEntrsEnDL,trainer_on_deadlock);
//			log_info(archivoLog,"Deadlock - agrego a %c como posible entrenador en deadlock",trainer_on_deadlock->e->simbolo);
//
//			//log_info(archivoLog,"Deadlock - el entrenador %c tiene a %c",(trainer_on_deadlock->e)->simbolo,((t_pokenest*)list_get(mapa->pokeNests,id))->identificador);
//
//			}else{}
//		otroTieneElPokeQueNecesito=true;
//		}
//	}
//	return otroTieneElPokeQueNecesito;
//}
//
//
//
//t_list* analizarDeadlock (){
//	log_info(archivoLog,"Deadlock - Hay %d entrenadoresBloqueados", list_size(entrenadoresBloqueados));
//	t_entrenador* entrenadorQueDeterminaDeadlock=(t_entrenador*)list_get(entrenadoresBloqueados,0);
//	log_info(archivoLog,"Leo al primer entrenador: %c", entrenadorQueDeterminaDeadlock->simbolo);
//	t_entrenador* trainer_to_compare=(t_entrenador*)malloc(sizeof(t_entrenador));
//	t_list* entrsPosiblesDeadlock=list_create();
//	//Elijo fin para cortar el barrido de entrenadores bloqueados
//	int noHayaEsperaCircular =1;
//	//Agrego al primer entrenador a los posibles "deadlock"
//	log_info(archivoLog,"Deadlock - Elijo a %c como entrenador determinante del deadlock", entrenadorQueDeterminaDeadlock->simbolo);
//	//Comienzo el barrido de los bloqueados
//	int size_trainers_maybe_on_deadlock,iEntrDeadlock;
//	size_trainers_maybe_on_deadlock=list_size(entrenadoresBloqueados);
//	log_info(archivoLog,"Deadlock - hay %d entrenadores para analizar", size_trainers_maybe_on_deadlock);
//	/*if ((size_trainers_maybe_on_deadlock<2)&&(noHayPokemonsDisponiblesPara(entrenadorQueDeterminaDeadlock, vecDisp))){
//		log_info(archivoLog,"Deadlock - El entrenador %c esta en inanicion",entrenadorQueDeterminaDeadlock->simbolo);
//		return entrsPosiblesDeadlock;
//	}else{*/
//		while (noHayaEsperaCircular==1){
//			for (iEntrDeadlock=0;iEntrDeadlock<size_trainers_maybe_on_deadlock;iEntrDeadlock++){
//				//Elijo al primer entrenador y me fijo si alguien mas tiene lo que necesita
//				t_entrenador* unE=list_get(entrenadoresBloqueados,iEntrDeadlock);
//				//log_info(archivoLog,"Deadlock - Se analiza el bloqueo de %c", unE->simbolo);
//				int pokeOnMatrix=find_pokenest_on_map(unE->pokenestBloqueante->identificador);
//				//log_info(archivoLog,"La pokenest en el mapa es la numero %d",pokeOnMatrix);
//				log_info(archivoLog,"Deadlock - La pokenest bloqueante de %c es: %c",unE->simbolo,unE->pokenestBloqueante->identificador);
//				chequearAsignacionAjena(pokeOnMatrix,entrsPosiblesDeadlock, trainer_to_compare,unE);
//				log_info(archivoLog,"el simbolo del actual %c",trainer_to_compare->simbolo);
//				log_info(archivoLog,"el simbolo del determinante %c",entrenadorQueDeterminaDeadlock->simbolo);
//				if (trainer_to_compare->simbolo==entrenadorQueDeterminaDeadlock->simbolo){
//					log_info(archivoLog,"Deadlock - Hay espera circular");
//					noHayaEsperaCircular =0;
//					}
//			}
//		}
//
//	return entrsPosiblesDeadlock;
//}
//bool hayEntrenadoresEnDeadlock(t_list* entrenadores){
//	if( list_is_empty(entrenadores)){
//		return false;
//	}else{
//		return true;
//	}
//}
//bool batallaActivada(){
//	return mapa->batalla==1;
//}
//
//int tiene_estos_pokemons(t_list* pokemons, char id_pokenest){
//	int tam=list_size(pokemons);
//	log_info(archivoLog,"Deadlock - Cuento la cantidad de pokmns que tiene un entrenador %d", tam);
//	int i,totAsignado;
//	totAsignado=0;
//	for (i=0;i<tam;i++){
//		log_info(archivoLog,"Deadlock - entro al foor de tiene_Estos_pokemons");
//		t_pokemon_custom* unPoke=(t_pokemon_custom*)list_get(pokemons,i);
//		log_info(archivoLog,"Deadlock - el pokemon actual es %c", unPoke->identificadorPokenest);
//		/*char vecAux[2];
//		vecAux[0]=unPoke->identificadorPokenest;
//		vecAux[1]=id_pokenest;*/
//		log_info(archivoLog,"Deadlock - unPoke vale %c",unPoke->identificadorPokenest);
//		log_info(archivoLog,"Deadlock - la pokenest vale %c",id_pokenest);
//		if (unPoke->identificadorPokenest==id_pokenest){
//			log_info(archivoLog,"Deadlock - Tiene asignado a %c", unPoke->identificadorPokenest);
//			totAsignado=totAsignado+1;
//		}
//	}
//	log_info(archivoLog,"Deadlock - Total asignado %d", totAsignado);
//	return totAsignado;
//}
//int asignarCantidadDePokemons (int entr, int poke){
//	t_entrenador* entrAsig=list_get(entrenadoresBloqueados,entr);
//	log_info(archivoLog,"Deadlock - Se asignara al Entrenador %c",entrAsig->simbolo);
//	t_pokenest* pokenestAsig=(t_pokenest*)list_get(mapa->pokeNests,poke);
//	log_info(archivoLog,"Deadlock - Se asignara el pokemon %c", pokenestAsig->identificador);
//	mAsignacion[entr][poke]=tiene_estos_pokemons(entrAsig->pokemons,pokenestAsig->identificador);
//	log_info(archivoLog,"Deadlock - asignacion: %d ",mAsignacion[entr][poke]);
//	log_info(archivoLog,"Deadlock - %c tiene %d pokemons de %c",entrAsig->simbolo,tiene_estos_pokemons(entrAsig->pokemons,pokenestAsig->identificador),pokenestAsig->identificador);
//	//log_info(archivoLog,"%c tiene %d pokemons de %c", entrAsig->simbolo, mAsignacion[entr][poke], pokenestAsig->identificador);
//	return mAsignacion[entr][poke];
//}
//int noHayEntrenadoresBloqueados(){
//	return list_is_empty(entrenadoresBloqueados);
//}
//void crearMatrizAsignacionParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	mAsignacion=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
//		for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//			mAsignacion[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
//		}
//}
//
//void crearMatrizNecesidadParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	mNecesidad=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
//			for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//				mNecesidad[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
//			}
//}
//void completarMatrizAsignacionParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	int numPokenest;
//	if (noHayEntrenadoresBloqueados()){
//		log_info(archivoLog,"No hay entrenadores bloqueados");
//	}else{
//		//log_info(archivoLog, "Hay %d entrenadoresBloqueados", list_size(entrenadoresBloqueados));
//		//log_info(archivoLog, "Hay %d pokenests", list_size(mapa->pokeNests));
//	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//				for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//				mAsignacion[numEntrenador][numPokenest]=asignarCantidadDePokemons(numEntrenador,numPokenest);
//				int asignado=mAsignacion[numEntrenador][numPokenest];
//				//log_info(archivoLog,"Deadlock - Lo asignado %d", asignado);
//				//Esto que sigue es solo para verificar que todo este bien cargado
//				//log_info(archivoLog,"ya se completo la matriz de asignacion para alguien");
//				t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
//				t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
//				log_info(archivoLog,"%c tiene asignados %d %c",e->simbolo,mAsignacion[numEntrenador][numPokenest],p->identificador);
//				}
//			}
//	}
//
//}
//int asignarNecesidad(int numEntrenador, int numPokenest){
//	t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
//	t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
//	if (e->pokenestBloqueante->identificador==p->identificador){
//		return 1;
//	}else{
//		return 0;
//	}
//}
//void completarMatrizNecesidadParaEntrenadoresBloqueados(){
//	int numEntrenador;
//	int numPokenest;
//	if (noHayEntrenadoresBloqueados()){
//		log_info(archivoLog,"No hay entrenadoresbloqueados");
//	}else{
//	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
//		for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//			mNecesidad[numEntrenador][numPokenest]=asignarNecesidad(numEntrenador,numPokenest);
//			log_info(archivoLog,"%c necesita %d de %c", ((t_entrenador*)list_get(entrenadoresBloqueados,numEntrenador))->simbolo, mNecesidad[numEntrenador][numPokenest],((t_pokemon_custom*)list_get(mapa->pokeNests,numPokenest))->id);
//		}
//	}
//	}
//}
//int pokemonsDisponiblesPara (t_pokenest* p){
//	int cantPoke;
//	int cant=0;
//	for (cantPoke=0;cantPoke<list_size(p->pokemons);cantPoke++){
//		t_pokemon_custom* pk=(t_pokemon_custom*)list_get(p->pokemons,cantPoke);
//		if (pk->disponible){
//			cant=cant+1;
//		}
//	}
//	log_info(archivoLog,"Deadlock - Hay %d pokemons disponibles de %c",cant,p->identificador);
//	return cant;
//}
//int * crearVectorPokemonsDisponibles(int *vecPokeDisp){
//	int numPokenest;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		vecPokeDisp[numPokenest]=pokemonsDisponiblesPara(list_get(mapa->pokeNests,numPokenest));
//	}
//	return vecPokeDisp;
//}
//void loguearVectorDisponibles(int *vecPokeDisp){
//	int numPokenest;
//	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
//		t_pokenest* pknst=list_get(mapa->pokeNests,numPokenest);
//		vecPokeDisp[numPokenest]=list_size(pknst->pokemons);
//		log_info(archivoLog,"Deadlock - Hay %d disponible(s) para: %c",vecPokeDisp[numPokenest],pknst->identificador);
//	}
//}
//
//bool hayMasDe1EntrenadorBloqueado (){
//	return (list_size(entrenadoresBloqueados)>1);
//}
//void* deadlock(void* arg){
//	log_trace(archivoLog, "Deadldock - Arranca");
//	while (1){
//		sleep(2);
//		int tamPokenests=list_size(mapa->pokeNests);
//		int *vecPokeDisp=(int*)malloc(tamPokenests*sizeof(int));
//		log_info(archivoLog,"Deadlock - Hilo deadlock espera proxima vez de chequeo");
//		crearVectorPokemonsDisponibles(vecPokeDisp);
//		log_info(archivoLog,"Deadlock - Se crea la matriz de asignacion de pokemons para entrenadoresBloqueados ");
//		crearMatrizAsignacionParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz para los bloqueados ");
//		completarMatrizAsignacionParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Se crea la matriz de necesidad");
//			crearMatrizNecesidadParaEntrenadoresBloqueados();
//		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz de necesidad");
//			completarMatrizNecesidadParaEntrenadoresBloqueados();
//
//		//sleep(0.1*(mapa->retardo));
//			log_info(archivoLog,"--------------------------------------------------------------------------");
//		log_info(archivoLog,"Deadlock - Hilo deadlock comienza su deteccion");
//
//		if (noHayEntrenadoresBloqueados()){
//					log_info(archivoLog,"Deadlock - No hay entrenadores bloqueados aun");
//				}else{
//					if(hayMasDe1EntrenadorBloqueado()){
//
//			log_info(archivoLog,"Deadlock - Creo lista de entrenadores para analizar deadlock. Tiene %d entrenadores",list_size(entrenadoresBloqueados));
//			t_list* entrenadoresAnalizados=list_create();
//			log_info(archivoLog,"Deadlock - Se analizaran %d entrenadores", list_size(entrenadoresBloqueados));
//			log_info(archivoLog,"Deadlock - Creo lista de entrenadores en DEADLOCK");
//			list_add_all(entrenadoresAnalizados,analizarDeadlock());
//			log_info(archivoLog,"La cantidad entrenadores que analizo el hilo deadlock es %d",list_size(entrenadoresAnalizados));
//		if (hayEntrenadoresEnDeadlock(entrenadoresAnalizados)){
//			if (batallaActivada()){
//				/*t_entrenador* entrenadorAMatar=(t_entrenador*)malloc(sizeof(t_entrenador));
//				if (entrenadorAMatar==NULL){
//					log_info(archivoLog,"Deadlock - Fallo el malloc del entrenador a matar");
//				}else{
//				log_info(archivoLog,"Deadlock - Se guarda memoria para entrenador a matar");
//				}
//				int entrenadoresEnBatalla=list_size(entrenadoresBloqueados);
//				int nroBatalla;
//				for (nroBatalla=0;nroBatalla<(entrenadoresEnBatalla-1);nroBatalla++){
//					t_entrenador* eRotatorio=list_get(entrenadoresBloqueados,nroBatalla);
//					t_pokemon_custom* unPoke=elegir_mejor_pokemon(entrenadorAMatar);
//					t_pokemon_custom* unPoke2=elegir_mejor_pokemon(eRotatorio);
//					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke->id, eRotatorio->simbolo );
//					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke2->id, entrenadorAMatar->simbolo );
//					t_pokemon* pokePerdedor=pkmn_battle((t_pokemon*)unPoke,(t_pokemon*)unPoke2);
//					if (pokePerteneceA(pokePerdedor,eRotatorio)){
//						entrenadorAMatar=eRotatorio;
//					}*/
//
//		}
//					else{
//		log_info(archivoLog,"Hay DEADLOCK pero no hay batalla pokemon configurada");
//		}
//		}
//			free(posiblesEntrenadoresEnDeadlock);
//			free(entrenadoresAnalizados);
//				free(vecPokeDisp);
//				free(mAsignacion);
//				free(mNecesidad);
//
//	}
//	}
//	}
//	return arg;
//}
//
////Estructura para la espera
//typedef struct
//{
//	uint8_t * const espera;
//	int head;
//	int tail;
//	const int maxLen;
//}esperaCircular_t;
//
//
////Funcion de la espera
//int esperaCircularPush(esperaCircular_t *c, uint8_t data)
//{
//	int next = c->head + 1;
//	if (next >= c->maxLen)
//		next = 0;
//	// Si esta llena
//	if (next == c->tail)
//		return -1;  // error
//	c->espera[c->head] = data;
//	c->head = next;
//	return 0;
//}
//
//int esperaCircularPop(esperaCircular_t *c, uint8_t *data)
//{
//	// si la cabeza no esta adelante de la cola
//	if (c->head == c->tail)
//		return -1;  // error
//	*data = c->espera[c->tail];
//	c->espera[c->tail] = 0;  // limpia espera(no es necesario)
//	int next = c->tail + 1;
//	if(next >= c->maxLen)
//		next = 0;
//	c->tail = next;
//	return 0;
//}
//
//
///*
////falta asociar a la fila y columna con la estructura en mapas (t_list* entrenadoresBloqueados y el int cantidad de pokemonesAsignados)
//typedef struct matrizAsignada {
//	int fila_entrenadoresBloqueados;
//	int columna_PokeAsignados;
//	int **matriz;
//	matriz = new int *[fila_entrenadoresBloqueados];
//	for(int i=0; i<fila_entrenadoresBloqueados; i++);
//	matriz[i] = new int[columna_PokeAsignados];
//}t_matriz_asignada;
//*/	}else{
//					if(hayMasDe1EntrenadorBloqueado()){
//
//			log_info(archivoLog,"Deadlock - Creo lista de entrenadores para analizar deadlock. Tiene %d entrenadores",list_size(entrenadoresBloqueados));
//			t_list* entrenadoresAnalizados=list_create();
//			log_info(archivoLog,"Deadlock - Se analizaran %d entrenadores", list_size(entrenadoresBloqueados));
//			log_info(archivoLog,"Deadlock - Creo lista de entrenadores en DEADLOCK");
//			list_add_all(entrenadoresAnalizados,analizarDeadlock());
//			log_info(archivoLog,"La cantidad entrenadores que analizo el hilo deadlock es %d",list_size(entrenadoresAnalizados));
//		if (hayEntrenadoresEnDeadlock(entrenadoresAnalizados)){
//			if (batallaActivada()){
//				/*t_entrenador* entrenadorAMatar=(t_entrenador*)malloc(sizeof(t_entrenador));
//				if (entrenadorAMatar==NULL){
//					log_info(archivoLog,"Deadlock - Fallo el malloc del entrenador a matar");
//				}else{
//				log_info(archivoLog,"Deadlock - Se guarda memoria para entrenador a matar");
//				}
//				int entrenadoresEnBatalla=list_size(entrenadoresBloqueados);
//				int nroBatalla;
//				for (nroBatalla=0;nroBatalla<(entrenadoresEnBatalla-1);nroBatalla++){
//					t_entrenador* eRotatorio=list_get(entrenadoresBloqueados,nroBatalla);
//					t_pokemon_custom* unPoke=elegir_mejor_pokemon(entrenadorAMatar);
//					t_pokemon_custom* unPoke2=elegir_mejor_pokemon(eRotatorio);
//					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke->id, eRotatorio->simbolo );
//					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke2->id, entrenadorAMatar->simbolo );
//					t_pokemon* pokePerdedor=pkmn_battle((t_pokemon*)unPoke,(t_pokemon*)unPoke2);
//					if (pokePerteneceA(pokePerdedor,eRotatorio)){
//						entrenadorAMatar=eRotatorio;
//					}*/
//
//		}
//					else{
//		log_info(archivoLog,"Hay DEADLOCK pero no hay batalla pokemon configurada");
//		}
//		}
//			free(posiblesEntrenadoresEnDeadlock);
//			free(entrenadoresAnalizados);
//				free(vecPokeDisp);
//				free(mAsignacion);
//				free(mNecesidad);
//
//	}
//	}
//	}
//	return arg;
//}
//
////Estructura para la espera
//typedef struct
//{
//	uint8_t * const espera;
//	int head;
//	int tail;
//	const int maxLen;
//}esperaCircular_t;
//
//
////Funcion de la espera
//int esperaCircularPush(esperaCircular_t *c, uint8_t data)
//{
//	int next = c->head + 1;
//	if (next >= c->maxLen)
//		next = 0;
//	// Si esta llena
//	if (next == c->tail)
//		return -1;  // error
//	c->espera[c->head] = data;
//	c->head = next;
//	return 0;
//}
//
//int esperaCircularPop(esperaCircular_t *c, uint8_t *data)
//{
//	// si la cabeza no esta adelante de la cola
//	if (c->head == c->tail)
//		return -1;  // error
//	*data = c->espera[c->tail];
//	c->espera[c->tail] = 0;  // limpia espera(no es necesario)
//	int next = c->tail + 1;
//	if(next >= c->maxLen)
//		next = 0;
//	c->tail = next;
//	return 0;
//}
//
//
///*
////falta asociar a la fila y columna con la estructura en mapas (t_list* entrenadoresBloqueados y el int cantidad de pokemonesAsignados)
//typedef struct matrizAsignada {
//	int fila_entrenadoresBloqueados;
//	int columna_PokeAsignados;
//	int **matriz;
//	matriz = new int *[fila_entrenadoresBloqueados];
//	for(int i=0; i<fila_entrenadoresBloqueados; i++);
//	matriz[i] = new int[columna_PokeAsignados];
//}t_matriz_asignada;
//*/
