/*
 * reset.h
 *
 *  Created on: 20/11/2016
 *      Author: utnso
 */

#include "../commons/structures.c"

#ifndef FUNCTIONS_RESET_H_
#define FUNCTIONS_RESET_H_

int doesFileExist(char* filename);
int remove_directory(const char *path);
void reiniciarObjetivo(t_objetivo* objetivo);
void reiniciarMapa(t_mapa* mapa);
void reiniciarJuego();
void actualizarMetadata();

#endif /* FUNCTIONS_RESET_H_ */
