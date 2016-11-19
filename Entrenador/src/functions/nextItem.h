/*
 * nextItem.h
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
#include "../commons/structures.c"
#include "../commons/constants.h"
#include "log.h"
#include "config.h"
#include "positions.h"
#include "processLogic.h"
#include <time.h>

#ifndef FUNCTIONS_NEXTITEM_H_
#define FUNCTIONS_NEXTITEM_H_

t_mapa* getNextMap();
t_objetivo* getNextObjective(t_mapa* mapa);

#endif /* FUNCTIONS_NEXTITEM_H_ */
