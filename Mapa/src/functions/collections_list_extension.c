/*
 * collections_list_extension.c
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <tad_items.h>
#include "../commons/structures.h"

void list_remove_custom(t_list* items, int i) {
    bool _search_by_socket(t_entrenador* item) {
        return item->socket == i;
    }

    list_remove_by_condition(items, (void*) _search_by_socket);
}

t_pokenest *find_pokenest_by_id(char id) {
	int _is_the_one(t_pokenest *p) {
			return (p->identificador==id);
	}
    return list_find(mapa->pokeNests, (void*) _is_the_one);
}
