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

void list_remove_custom(t_list* items, char id) {
    bool _search_by_id(char* item) {
        return *item == id;
    }

    list_remove_by_condition(items, (void*) _search_by_id);
}
