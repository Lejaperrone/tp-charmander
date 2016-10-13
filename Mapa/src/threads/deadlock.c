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
typedef struct entrPosiblesDeadlock{
	t_entrenador* e;
	int status;
}t_entrPosibleDeadlock;


extern pthread_mutex_t mutexEntrBQ;

//bool(*condition)(void*)

bool estaDisponible (t_pokemon_custom* unPoke){
	return (unPoke->disponible==1);
}

bool noHayPokemonsDisponiblesPara(t_entrenador* unE, int** vecDisp){
	int numPokenest;
	bool noHayDisp=false;
	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
		if (unE->pokenestBloqueante==(t_pokenest*)list_get(mapa->pokeNests,numPokenest)){
			if (*vecDisp[numPokenest]<=0){
				noHayDisp=true;
			}
		}
	}
	return noHayDisp;
}

bool poseePokemonSolicitado (t_entrenador* unE){

	return (list_count_satisfying(unE->pokemons,(void*)estaDisponible)>0);
}
void juntarTodosLosEntrenadores(t_list* posiblesEntrenadoresEnDeadlock) {
	sizeTrainersOnMap=list_size(entrenadoresListos)+list_size(entrenadoresBloqueados);
			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresListos);
			list_add_all(posiblesEntrenadoresEnDeadlock,entrenadoresBloqueados);

}

bool chequearAsignacionAjena (int id, t_list **entrsBQ, int** vecDisp, t_list**posiblesEntrsEnDL){
	int total_trainers__to_check=list_size(*entrsBQ);
	log_info(archivoLog,"Deadlock - La cantidad de entrenadores bloqueados para chequear asignacion ajena es: %d", total_trainers__to_check);
	int i;
	bool hayAsignacionAjena=false;
	for (i=0;i<total_trainers__to_check;i++){
		if (mAsignacion[i][id]>0){
			t_entrenador* trainer=(t_entrenador*)list_get(entrenadoresBloqueados,i);
			t_entrPosibleDeadlock* trainer_on_deadlock=(t_entrPosibleDeadlock*)malloc(sizeof(t_entrPosibleDeadlock));
			trainer_on_deadlock->e=trainer;
			trainer_on_deadlock->status=1;
			//esto a modo de verificacion
			t_pokenest* pk=list_get(mapa->pokeNests,id);
			char pok=pk->identificador;
			log_info(archivoLog,"Deadlock - El entrenador %c tiene a %c",trainer_on_deadlock->e->simbolo,pok);
			hayAsignacionAjena=true;
			list_add(*posiblesEntrsEnDL,trainer_on_deadlock);
			free(trainer_on_deadlock);
			}
		}
	return hayAsignacionAjena;
}

int find_pokenest_on_map(char idPokenest){
	int size_pokenests_on_map=list_size(mapa->pokeNests);
	int i,pos=-1;
	for (i=0;i<size_pokenests_on_map;i++){
		t_pokenest* unaP=list_get(mapa->pokeNests,i);
		if (unaP->identificador==idPokenest){
			pos=i;
		}
	}
	return pos;
}

t_list* analizarDeadlock (int** vecDisp,t_list* entr){
	log_info(archivoLog,"Deadlock - Hay %d entrenadoresBloqueados", list_size(entr));
	t_entrenador* entrenadorQueDeterminaDeadlock=(t_entrenador*)list_get(entr,0);
	log_info(archivoLog,"Leo al primer entrenador: %c", entrenadorQueDeterminaDeadlock->simbolo);
	t_list* entrsPosiblesDeadlock=list_create();
	//Elijo fin para cortar el barrido de entrenadores bloqueados
	int fin =0;
	//Agrego al primer entrenador a los posibles "deadlock"
	log_info(archivoLog,"Deadlock - Elijo a %c como entrenador determinante del deadlock", entrenadorQueDeterminaDeadlock->simbolo);
	//Comienzo el barrido de los bloqueados
	int size_trainers_maybe_on_deadlock,iEntrDeadlock;
	size_trainers_maybe_on_deadlock=list_size(entr);
	log_info(archivoLog,"Deadlock - hay %d entrenadores para analizar", size_trainers_maybe_on_deadlock);
	/*if ((size_trainers_maybe_on_deadlock<2)&&(noHayPokemonsDisponiblesPara(entrenadorQueDeterminaDeadlock, vecDisp))){
		log_info(archivoLog,"Deadlock - El entrenador %c esta en inanicion",entrenadorQueDeterminaDeadlock->simbolo);
		return entrsPosiblesDeadlock;
	}else{*/
		while (fin!=1){
			for (iEntrDeadlock=0;iEntrDeadlock<size_trainers_maybe_on_deadlock;iEntrDeadlock++){
				//Elijo al primer entrenador y me fijo si alguien mas tiene lo que necesita
				t_entrenador* unE=list_get(entr,iEntrDeadlock);
				log_info(archivoLog,"Deadlock - Se analiza el bloqueo de %c", unE->simbolo);
				int pokeOnMatrix=find_pokenest_on_map(unE->pokenestBloqueante->identificador);
				log_info(archivoLog,"Deadlock - La pokenest bloqueante es: %c",unE->pokenestBloqueante->identificador);
				chequearAsignacionAjena(pokeOnMatrix,&entr, vecDisp,&entrsPosiblesDeadlock);
				if ((t_entrenador*)list_get(entrsPosiblesDeadlock,iEntrDeadlock)==entrenadorQueDeterminaDeadlock){
					fin =1;
					}
			}
		}

	return entrsPosiblesDeadlock;
}
bool hayEntrenadoresEnDeadlock(t_list* entrenadores){
	if( list_is_empty(entrenadores)){
		return false;
	}else{
		return true;
	}
}
bool batallaActivada(){
	return mapa->batalla==1;
}

int tiene_estos_pokemons(t_list* pokemons, char id_pokenest){
	int tam=list_size(pokemons);
	int i,totAsignado;
	totAsignado=0;
	for (i=0;i<tam;i++){
		t_pokemon_custom* unPoke=(t_pokemon_custom*)list_get(pokemons,i);
		if (unPoke->id==id_pokenest){
			totAsignado++;
		}
	}
	return totAsignado;
}
int asignarCantidadDePokemons (int entr, int poke){
	t_entrenador* entrAsig=list_get(entrenadoresBloqueados,entr);
	log_info(archivoLog,"Se asignaran a %c",entrAsig->simbolo);
	t_pokenest* pokenestAsig=(t_pokenest*)list_get(mapa->pokeNests,poke);
	log_info(archivoLog,"Se asignara 1 de %c",pokenestAsig->identificador);
	int r=tiene_estos_pokemons(entrAsig->pokemons,pokenestAsig->identificador);
	log_info(archivoLog,"%c tiene %d pokemons de %c", entrAsig->simbolo, r, pokenestAsig->identificador);
	log_info(archivoLog,"r vale %d",r);
	return r;
}
int noHayEntrenadoresBloqueados(){
	return list_is_empty(entrenadoresBloqueados);
}
void crearMatrizAsignacionParaEntrenadoresBloqueados(){
	int numEntrenador;
	mAsignacion=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
		for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
			mAsignacion[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
		}
}

void crearMatrizNecesidadParaEntrenadoresBloqueados(){
	int numEntrenador;
	mNecesidad=(int**)malloc(list_size(entrenadoresBloqueados)*sizeof(int*));
			for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
				mNecesidad[numEntrenador]=(int*)malloc(list_size(mapa->pokeNests)*sizeof(int));
			}
}
void completarMatrizAsignacionParaEntrenadoresBloqueados(){
	int numEntrenador;
	int numPokenest;
	if (noHayEntrenadoresBloqueados()){
		log_info(archivoLog,"No hay entrenadores bloqueados");
	}else{
	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
				for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
				mAsignacion[numEntrenador][numPokenest]=asignarCantidadDePokemons(numEntrenador,numPokenest);
				//Esto que sigue es solo para verificar que todo este bien cargado
				log_info(archivoLog,"ya se completo la matriz de asignacion para alguien");
				t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
				t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
				log_info(archivoLog,"%c tiene asignados %d %c",e->simbolo,mAsignacion[numEntrenador][numPokenest],p->identificador );
				}
			}
	}

}
int asignarNecesidad(int numEntrenador, int numPokenest){
	t_entrenador* e =list_get(entrenadoresBloqueados,numEntrenador);
	t_pokenest* p=list_get(mapa->pokeNests,numPokenest);
	if (e->pokenestBloqueante->identificador==p->identificador){
		return 1;
	}else{
		return 0;
	}
}
void completarMatrizNecesidadParaEntrenadoresBloqueados(){
	int numEntrenador;
	int numPokenest;
	if (noHayEntrenadoresBloqueados()){
		log_info(archivoLog,"No hay entrenadoresbloqueados");
	}else{
	for (numEntrenador=0;numEntrenador<list_size(entrenadoresBloqueados);numEntrenador++){
		for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
			mNecesidad[numEntrenador][numPokenest]=asignarNecesidad(numEntrenador,numPokenest);
		}
	}
	}
}
int pokemonsDisponiblesPara (t_pokenest* p){
	int cantPoke;
	int cant=0;
	for (cantPoke=0;cantPoke<list_size(p->pokemons);cantPoke++){
		t_pokemon_custom* pk=(t_pokemon_custom*)list_get(p->pokemons,cantPoke);
		if (pk->disponible){
			cant=cant+1;
		}
	}
	log_info(archivoLog,"Deadlock - Hay %d pokemons disponibles de %c",cant,p->identificador);
	return cant;
}
int * crearVectorPokemonsDisponibles(int *vecPokeDisp){
	int numPokenest;
	/*if (list_is_empty(entrenadoresBloqueados)){
		log_info(archivoLog,"Deadlock - No hay entrenadores bloqueados");
		for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
			t_pokenest* pknst=list_get(mapa->pokeNests,numPokenest);
			log_info(archivoLog,"Deadlock - obtengo los pokemons disponibles para %c",pknst->identificador);
			vecPokeDisp[numPokenest]=list_size(pknst->pokemons);
			log_info(archivoLog,"Deadlock: %d disponibles de %c",vecPokeDisp[numPokenest],pknst->identificador);
			}
	}else{
	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
		int totalDeUnaPokenest=0;
		for(numEntr=0;numEntr<list_size(entrenadoresBloqueados);numEntr++){
			//totalDeUnaPokenest=totalDeUnaPokenest-mAsignacion[numEntr][numPokenest];
			t_pokenest* unaPkns=list_get()
		}
		vecPokeDisp[numPokenest]=totalDeUnaPokenest;
	}
	}*/
	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
		vecPokeDisp[numPokenest]=pokemonsDisponiblesPara(list_get(mapa->pokeNests,numPokenest));
	}
	return vecPokeDisp;
}
void loguearVectorDisponibles(int *vecPokeDisp){
	int numPokenest;
	for (numPokenest=0;numPokenest<list_size(mapa->pokeNests);numPokenest++){
		t_pokenest* pknst=list_get(mapa->pokeNests,numPokenest);
		vecPokeDisp[numPokenest]=list_size(pknst->pokemons);
		log_info(archivoLog,"Deadlock - Hay %d disponible(s) para: %c",vecPokeDisp[numPokenest],pknst->identificador);
	}
}
/*t_pokemon_custom* elegir_mejor_pokemon(t_entrenador* e){
	int cantidadPokemons=list_size(e->pokemons);
	int i;
	t_pokemon_custom* mejorPoke=(t_pokemon_custom*)malloc(sizeof(t_pokemon_custom));
	mejorPoke=list_get(e->pokemons,0);
	log_info(archivoLog,"Por el momento el mejor pokemon de %c es %c", e->simbolo, mejorPoke->id);
	for (i=0;i<cantidadPokemons;i++){
		t_pokemon_custom* otroPoke=(t_pokemon_custom*)list_get(e->pokemons,i);
		if (otroPoke->nivel>mejorPoke->nivel){
			mejorPoke=otroPoke;
		}

	}
	return mejorPoke;
}

bool pokePerteneceA(t_pokemon* unP, t_entrenador* unE){
	bool pertenece=false;
	int i,size_trainer_poke=list_size(unE->pokemons);
	for (i=0;i<size_trainer_poke;i++){
			t_pokemon_custom* unPoke=list_get(unE->pokemons,i);
	if(unP==(t_pokemon*)unPoke){
		pertenece=true;
	}
	}
	return pertenece;
}*/

void* deadlock(void* arg){
	log_trace(archivoLog, "Deadldock - Arranca");
	while (1){
		sleep(2);
		int tamPokenests=list_size(mapa->pokeNests);
		int *vecPokeDisp=(int*)malloc(tamPokenests*sizeof(int));
		log_info(archivoLog,"Deadlock - Hilo deadlock espera proxima vez de chequeo");
		crearVectorPokemonsDisponibles(vecPokeDisp);
		log_info(archivoLog,"Deadlock - Se crea la matriz de asignacion de pokemons para entrenadoresBloqueados ");
		crearMatrizAsignacionParaEntrenadoresBloqueados();
		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz para los bloqueados ");
		completarMatrizAsignacionParaEntrenadoresBloqueados();
		log_info(archivoLog,"Deadlock - Se crea la matriz de necesidad");
			crearMatrizNecesidadParaEntrenadoresBloqueados();
		log_info(archivoLog,"Deadlock - Comienzo a llenar la matriz de necesidad");
			completarMatrizNecesidadParaEntrenadoresBloqueados();

		//sleep(0.1*(mapa->retardo));

		log_info(archivoLog,"Deadlock - Hilo deadlock comienza su deteccion");

		if (noHayEntrenadoresBloqueados()){
					log_info(archivoLog,"Deadlock - No hay entrenadores bloqueados aun");
				}else{
			t_list* entrenadoresParaAnalizar= list_create();
			log_info(archivoLog,"Deadlock - Creo lista de entrenadores para analizar deadlock. Tiene %d entrenadores",list_size(entrenadoresBloqueados));
			t_list* entrenadoresAnalizados=list_create();
			list_add_all(entrenadoresParaAnalizar,entrenadoresBloqueados);
			log_info(archivoLog,"Deadlock - Se analizaran %d entrenadores", list_size(entrenadoresParaAnalizar));
			log_info(archivoLog,"Deadlock - Creo lista de entrenadores en DEADLOCK");
			list_add_all(entrenadoresAnalizados,analizarDeadlock(&vecPokeDisp,entrenadoresParaAnalizar));
		if (hayEntrenadoresEnDeadlock(entrenadoresAnalizados)){
			if (batallaActivada()){
				/*t_entrenador* entrenadorAMatar=(t_entrenador*)malloc(sizeof(t_entrenador));
				if (entrenadorAMatar==NULL){
					log_info(archivoLog,"Deadlock - Fallo el malloc del entrenador a matar");
				}else{
				log_info(archivoLog,"Deadlock - Se guarda memoria para entrenador a matar");
				}
				int entrenadoresEnBatalla=list_size(entrenadoresBloqueados);
				int nroBatalla;
				for (nroBatalla=0;nroBatalla<(entrenadoresEnBatalla-1);nroBatalla++){
					t_entrenador* eRotatorio=list_get(entrenadoresBloqueados,nroBatalla);
					t_pokemon_custom* unPoke=elegir_mejor_pokemon(entrenadorAMatar);
					t_pokemon_custom* unPoke2=elegir_mejor_pokemon(eRotatorio);
					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke->id, eRotatorio->simbolo );
					log_info(archivoLog,"Se ha elegido a %c para el entrenador %c",unPoke2->id, entrenadorAMatar->simbolo );
					t_pokemon* pokePerdedor=pkmn_battle((t_pokemon*)unPoke,(t_pokemon*)unPoke2);
					if (pokePerteneceA(pokePerdedor,eRotatorio)){
						entrenadorAMatar=eRotatorio;
					}*/

		}else{
		log_info(archivoLog,"Hay DEADLOCK pero no hay batalla pokemon configurada");
		}
			free(posiblesEntrenadoresEnDeadlock);
			free(entrenadoresParaAnalizar);
				free(entrenadoresAnalizados);
				free(vecPokeDisp);
				free(mAsignacion);
				free(mNecesidad);
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
