#define OSADA_BLOCK_SIZE 64
#define OSADA_FILENAME_LENGTH 17
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
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
osada_header header;

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

#pragma pack(pop)

int main (){
	int i;
	FILE* arch;
	arch=fopen("miArchivo.bin","r");
	//fseek(arch, 0L, SEEK_SET);
	long tamArch;
	tamArch=ftell(arch);

	struct stat sbuf;
	if (stat("miArchivo.bin", &sbuf) == -1) {
		perror("stat");
		exit(1);
	}


	printf("miArchivo.bin ocupa %li bytes\n", sbuf.st_size-1);
	char* datos=malloc(sizeof(char*));
	fread(datos, OSADA_BLOCK_SIZE*8, 1, arch);
	//agregar por si falla el abrir archivo
	int j=0;
	for (i=0;i<7;i++){
		header.magic_number[i]=datos[i];
		j++;
	}
	printf("Identificador: %s\n",header.magic_number);
	//--------------------------------------------------------------
	for(i=0;i<1;i++){
			header.version=datos[j];
			j++;
		}
	printf("Version: %d\n",header.version);
	char bq[4];
	for (i=0;i<4;i++){
			bq[i]=datos[j];
			printf("leo bq: %c\n",bq[i]);
			printf("j vale %d\n",j);
			printf("leo datos: %c\n",datos[j]);
			j++;
	}
	header.fs_blocks=atoi(bq);
	printf("FS tiene %d bloques\n",header.fs_blocks);
	//header.fs_blocks=atoi(bq);
	printf("Tamanio del FS: %d\n",header.fs_blocks);
	/*header.bitmap_blocks=header.fs_blocks/8/OSADA_BLOCK_SIZE;
	printf("Tamanio del bitmap: %d\n",header.bitmap_blocks);
	header.allocations_table_offset=1+header.bitmap_blocks+1024;
	printf("Inicio Tabla Asignaciones: %d\n",header.allocations_table_offset);
	header.data_blocks=header.fs_blocks-1-header.bitmap_blocks-1024-header.allocations_table_offset;
	printf("Tamanio de datos [bloques]: %d\n",header.data_blocks);*/
	return 1;
}
