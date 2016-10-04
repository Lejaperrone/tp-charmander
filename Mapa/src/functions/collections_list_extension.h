/*
 * collections_list_extension.h
 *
 *  Created on: 11/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <tad_items.h>

#ifndef FUNCTIONS_COLLECTIONS_LIST_EXTENSION_H_
#define FUNCTIONS_COLLECTIONS_LIST_EXTENSION_H_

	void list_remove_custom(t_list* items, char id);
	t_pokenest *find_pokenest_by_id(char id);

#endif /* FUNCTIONS_COLLECTIONS_LIST_EXTENSION_H_ */
