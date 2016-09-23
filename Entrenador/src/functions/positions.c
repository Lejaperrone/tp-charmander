#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../commons/structures.c"
#include "../commons/constants.h"

int coordenadasCoinciden(t_coordenadas coordenadas, t_coordenadas coordenadas2){
	if(coordenadas.x==coordenadas2.x && coordenadas.y==coordenadas2.y){
		return 1;
	}
	return 0;
}

int siguienteMovimiento(t_coordenadas coordenadas, t_objetivo* objetivo, int ultimoMovimiento){
	if(coordenadasCoinciden(coordenadas, objetivo->ubicacion)){
		return 0;
	}

	int distX = objetivo->ubicacion.x - coordenadas.x;
	int distY = objetivo->ubicacion.y - coordenadas.y;

	if(distX == 0){
		if(distY>0){
			return MOVERSE_DERECHA;
		}else{
			return MOVERSE_IZQUIERDA;
		}
	}

	if(distY ==0){
		if(distX>0){
			return MOVERSE_ABAJO;
		}else{
			return MOVERSE_ARRIBA;
		}
	}

	if(distX>0 && ultimoMovimiento!=MOVERSE_ABAJO){
		return MOVERSE_ABAJO;
	}else if(distX<0 && ultimoMovimiento!=MOVERSE_ARRIBA){
		return MOVERSE_ARRIBA;
	}else if(distY>0 && ultimoMovimiento!=MOVERSE_DERECHA){
		return MOVERSE_DERECHA;
	}else if(distY<0 && ultimoMovimiento!=MOVERSE_IZQUIERDA){
		return MOVERSE_IZQUIERDA;
	}

	return 0;
}
