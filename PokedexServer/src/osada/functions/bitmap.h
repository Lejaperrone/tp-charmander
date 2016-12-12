/*
 * bitmap.h
 *
 *  Created on: 10/12/2016
 *      Author: utnso
 */

#ifndef OSADA_FUNCTIONS_BITMAP_H_
#define OSADA_FUNCTIONS_BITMAP_H_

void osada_B_findFreeBlock(int* lugarLibre);
int osada_B_cantBloquesLibres();
int osada_B_reserveNewBlocks (int* n, int* bloqueArranque, int indice);
int osada_B_reserveNewBlocksForWriteDeleteLatter (int* n, int* bloqueArranque, int indice);

#endif /* OSADA_FUNCTIONS_BITMAP_H_ */
