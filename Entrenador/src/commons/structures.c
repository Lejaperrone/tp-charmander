#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef COMMONS_STRUCTURES_C
#define COMMONS_STRUCTURES_C

typedef struct t_ruta_mapa_structs{
	char* nombre;
	char ** objetivos;
	int cantObjetivos;
	//Falta hoja de viaje y los objetivos
}t_ruta_mapa;

typedef struct t_entrenador_structs{
	char* nombre;
	char* simbolo;
	int vidas;
	int reintentos;
	t_ruta_mapa** hojaDeRuta;
	int cantMapas;
	//Falta hoja de viaje y los objetivos
}t_entrenador;



#endif
