#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>

#ifndef COMMONS_STRUCTURES_C
#define COMMONS_STRUCTURES_C

typedef struct t_coordenadas_structs{
	int x;
	int y;
}t_coordenadas;

typedef struct t_objetivo_structs{
	char* nombre;
	t_coordenadas ubicacion;
	int logrado;
}t_objetivo;

typedef struct t_mapa_structs{
	char* nombre;
	t_list* objetivos;
	char* puerto;
	char* ip;
	t_coordenadas miPosicion;
}t_mapa;

typedef struct t_entrenador_structs{
	char* nombre;
	char simbolo;
	int vidas;
	int reintentos;
	t_list* hojaDeViaje;
}t_entrenador;



#endif
