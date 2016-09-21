/*
 * osada.h
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>

#ifndef OSADA_OSADA_H_
#define OSADA_OSADA_H_

int osada_readdir(char* path, t_list* directorios);
int osada_init(char* path);

#endif /* OSADA_OSADA_H_ */
