/*
 * nextItem.c
 *
 *  Created on: 19/11/2016
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <tad_items.h>
#include <commons/log.h>
#include <signal.h>
#include <commons/collections/dictionary.h>
#include "commons/structures.c"
#include "commons/constants.h"
#include "functions/log.h"
#include "functions/config.h"
#include "functions/positions.h"
#include "functions/processLogic.h"
#include <time.h>

t_mapa* getNextMap(){
	bool _mapaNotFinish(t_mapa* mapa){
		return (mapa->terminado == 0);
	}

	return list_find(entrenador->hojaDeViaje, (void*)_mapaNotFinish);
}

t_objetivo* getNextObjective(t_mapa* mapa){
	bool _objectiveNotFinish(t_objetivo* objetivo){
		return (objetivo->logrado == 0);
	}

	return list_find(entrenador->hojaDeViaje, (void*)_objectiveNotFinish);
}


