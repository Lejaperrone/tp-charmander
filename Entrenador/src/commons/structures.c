#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>

#ifndef COMMONS_STRUCTURES_C
#define COMMONS_STRUCTURES_C

typedef struct t_mapa_structs{
	char* nombre;
	t_list* objetivos;
	//Falta hoja de viaje y los objetivos
}t_mapa;

typedef struct t_entrenador_structs{
	char* nombre;
	char* simbolo;
	int vidas;
	int reintentos;
	t_list* hojaDeViaje;
}t_entrenador;



#endif
