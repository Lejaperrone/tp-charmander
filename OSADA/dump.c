/*
 * dump.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include <commons/bitarray.h>

char* stringFromOsadaFileState(osada_file_state f){
    char *strings[] = { "DELETED", "REGULAR", "DIRECTORY", /* continue for rest of values */ };

    return strings[f];
}

void dumpHeader(osada_header* header){
	printf("Identificador: %s\n",header->magic_number);
	printf("Version: %d\n",header->version);
	printf("FS tiene %d bloques\n",header->fs_blocks);
	printf("Bitmap tiene %d bloques\n",header->bitmap_blocks);
}

void dumpBitmap(t_bitarray * bitmap){
	int i,j, k, m;
	j=-1;
	k=-1;
	m=-1;
	printf("Bitmap:\n");
	for(i=0; i<bitarray_get_max_bit(bitmap); i++){
		j++;
		if(j==7){
			k++;
			if(k==3){
				m++;
				if(m==1){
					printf("%d\n", bitarray_test_bit(bitmap, i));
					m=-1;
				}else{
					printf("%d  ", bitarray_test_bit(bitmap, i));
				}

				k=-1;
			}else{
				printf("%d ", bitarray_test_bit(bitmap, i));
			}

			j=-1;

		}else{
			printf("%d", bitarray_test_bit(bitmap, i));
		}

	}
	printf("\n");
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
