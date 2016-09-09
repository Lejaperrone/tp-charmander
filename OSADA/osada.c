/*
 * osada.c
 *
 *  Created on: 8/9/2016
 *      Author: utnso
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define OSADA_BLOCK_SIZE 64
#define OSADA_FILENAME_LENGTH 17
typedef unsigned char osada_block[OSADA_BLOCK_SIZE];
typedef uint32_t osada_block_pointer;

// set __attribute__((packed)) for this whole section
// See http://stackoverflow.com/a/11772340/641451
#pragma pack(push, 1)

typedef struct {
	unsigned char magic_number[7]; // OSADAFS
	uint8_t version;
	uint32_t fs_blocks; // total amount of blocks
	uint32_t bitmap_blocks; // bitmap size in blocks
	uint32_t allocations_table_offset; // allocations table's first block number
	uint32_t data_blocks; // amount of data blocks
	unsigned char padding[40]; // useless bytes just to complete the block size
} osada_header;

_Static_assert( sizeof(osada_header) == sizeof(osada_block), "osada_header size does not match osada_block size");



typedef enum __attribute__((packed)) {
    DELETED = '\0',
    REGULAR = '\1',
    DIRECTORY = '\2',
} osada_file_state;

_Static_assert( sizeof(osada_file_state) == 1, "osada_file_state is not a char type");

typedef struct {
	osada_file_state state;
	unsigned char fname[OSADA_FILENAME_LENGTH];
	uint16_t parent_directory;
	uint32_t file_size;
	uint32_t lastmod;
	osada_block_pointer first_block;
} osada_file;

_Static_assert( sizeof(osada_file) == (sizeof(osada_block) / 2.0), "osada_file size does not half osada_block size");

osada_header header;
//uint32_t bitmap[header.fs_blocks/8/OSADA_BLOCK_SIZE];

#pragma pack(pop)

int main(){
	FILE* arch;
	if ((arch=fopen("/home/utnso/git/tp-2016-2c-Chamba/miFilesystem.dat","r"))==NULL){
		printf("No se pudo leer el archivo\n");
	}else{
		while (!feof(arch)){
			fread(&header,OSADA_BLOCK_SIZE,1,arch);
			//fread(&bitmap,header.fs_blocks/8/OSADA_BLOCK_SIZE,1,arch);

		}
		printf("el ID es: %u\n",header.magic_number);
		printf("la version es: %u\n",header.version);
		printf("el tamanio del FS en bloquen es: %u\n",header.fs_blocks);
		printf("el tamanio del Bitmap en bloques es: %u\n",header.bitmap_blocks);
		printf("el inicio de tabla de asignaciones es: %u\n",header.allocations_table_offset);
		printf("los tamanios de datos son: %u\n", header.data_blocks);
		printf("el relleno es: %u\n", header.padding);
		//printf("el bitmap es: %u\n", bitmap);
	}

return 1;
}
