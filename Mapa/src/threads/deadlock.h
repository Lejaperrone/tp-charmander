/*
 * deadlock.h
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <nivel.h>
#include <pkmn/battle.h>
#include <sys/socket.h>
#include <commons/string.h>
#include "../socketLib.h"
#include "../commons/structures.h"
#include "../functions/recursos.h"
#include "../functions/collections_list_extension.h"
#include "../threads/planificador.h"

#ifndef THREADS_DEADLOCK_H_
#define THREADS_DEADLOCK_H_

int countPokemonsCapturadosFromPokenest(t_entrenador* entrenador, char identificador);
int pokemonsDisponibles(t_pokenest* pokenest);
void llenarEstructuras();

void informarDeadlock (t_entrenador* entrenador);
void algoritmoDeDeteccion();

t_pokemon_custom* obtenerPokemonMasFuerte(t_entrenador* entrenador);
void batalla();

void liberarMemoria();
void* deadlock(void* arg);

#endif /* THREADS_DEADLOCK_H_ */
