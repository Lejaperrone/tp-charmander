#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef COMMONS_STRUCTURES_C
#define COMMONS_STRUCTURES_C

typedef struct t_entrenador_structs{
	char* nombre;
	char* simbolo;
	int vidas;
	int reintentos;
	//Falta hoja de viaje y los objetivos
}t_entrenador;

#endif
