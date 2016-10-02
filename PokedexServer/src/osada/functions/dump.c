/*
 * dump.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../commons/osada.h"
#include <commons/bitarray.h>

char* stringFromOsadaFileState(osada_file_state f){
    char *strings[] = { "DELETED", "REGULAR", "DIRECTORY", /* continue for rest of values */ };

    return strings[f];
}

void dumpHeader(osada_header* header){
	printf("%s - version %d\n",header->magic_number, header->version);
	printf("FS blocks: %d\n",header->fs_blocks);
	printf("Bitmap blocks: %d\n",header->bitmap_blocks);
	printf("Allocations table offset: %d\n",header->allocations_table_offset);
	printf("Data blocks: %d\n\n",header->data_blocks);
}

void dumpBitmap(t_bitarray * bitmap){
	printf("Bitmap contents:\n");

	int i, j, k, m, value, cantOcupados;
	j=-1;
	k=-1;
	m=-1;
	cantOcupados = 0;

	for(i=0; i<bitarray_get_max_bit(bitmap); i++){
		value = bitarray_test_bit(bitmap, i);
		if(value){
			cantOcupados++;
		}

		j++;
		if(j==7){
			k++;
			if(k==3){
				m++;
				if(m==1){
					printf("%d\n", value);
					m=-1;
				}else{
					printf("%d  ", value);
				}

				k=-1;
			}else{
				printf("%d ", value);
			}

			j=-1;

		}else{

			printf("%d", value);
		}
	}

	printf("\n");
	printf("%d used blocks - %d free\n", cantOcupados, bitarray_get_max_bit(bitmap)-cantOcupados);
}

void dumpFileTable(osada_file * tablaArchivos){
	int i;
	for(i=0; i<2048; i++){
		if(tablaArchivos[i].state != 0){
			printf("indice: %d\n", i);
			printf("Estado: %s\n",stringFromOsadaFileState(tablaArchivos[i].state));
			printf("Nombre de archivo: %s\n",tablaArchivos[i].fname);
			printf("Bloque padre: %d\n",tablaArchivos[i].parent_directory);
			printf("Tamanio del archivo: %d\n",tablaArchivos[i].file_size);
			printf("Fecha ultima modificacion: %d\n",tablaArchivos[i].lastmod);
			printf("Bloque inicial: %d\n",tablaArchivos[i].first_block);
		}
	}
}

void dumpAllocationsTable(osada_block_pointer* tablaAsignaciones){

}
