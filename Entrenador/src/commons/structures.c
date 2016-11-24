#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#ifndef COMMONS_STRUCTURES
#define COMMONS_STRUCTURES

typedef struct t_coordenadas_structs{
	int x;
	int y;
}t_coordenadas;

typedef struct t_objetivo_structs{
	char* nombre;
	t_coordenadas ubicacion;
	int logrado;
	time_t tiempoBloqueado;
}t_objetivo;

typedef struct t_mapa_structs{
	char* nombre;
	t_list* objetivos;
	char* puerto;
	char* ip;
	t_coordenadas miPosicion;
	int terminado;
}t_mapa;


typedef struct t_entrenador_structs{
	char* nombre;
	char simbolo;
	int vidas;
	int reintentos;
	t_list* hojaDeViaje;
	int muertes;
	int deadlocks;
	time_t tiempoTotalAventura;
	time_t tiempoBloqueado;
}t_entrenador;



#endif
