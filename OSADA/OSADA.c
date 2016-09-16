#define OSADA_BLOCK_SIZE 64
#define OSADA_FILENAME_LENGTH 17
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <commons/bitarray.h>
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

static inline char *stringFromOsadaFileState(osada_file_state f)
{
    static const char *strings[] = { "DELETED", "REGULAR", "DIRECTORY", /* continue for rest of values */ };

    return strings[f];
}

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

size_t int2size_t(int val) {
    return (val < 0) ? __SIZE_MAX__ : (size_t)((unsigned)val);
}

int size_t2int(size_t val) {
    return (val <= INT_MAX) ? (int)((ssize_t)val) : -1;
}

int main (){
	int arch;
	arch=open("miArchivo.bin", O_RDWR, (mode_t)0600);

	struct stat sbuf;
	if (stat("miArchivo.bin", &sbuf) == -1) {
		perror("stat");
		exit(1);
	}

	printf("miArchivo.bin ocupa %li bytes\n", sbuf.st_size-1);

	//Mapeo el header
		osada_header* header;
		if(arch >= 0){
			header = (osada_header*)mmap((caddr_t)0, int2size_t(sizeof(osada_header)), PROT_WRITE, MAP_SHARED, arch, 0);

			if (header == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the header");
				exit(EXIT_FAILURE);
			}
			printf("Identificador: %s\n",header->magic_number);
			printf("Version: %d\n",header->version);
			printf("FS tiene %d bloques\n",header->fs_blocks);
			printf("Bitmap tiene %d bloques\n",header->bitmap_blocks);
		}

	//Mapeo el bitmap
		if(arch >= 0 && header!=NULL){
			int tamanioBitmap = header->bitmap_blocks*sizeof(osada_block) + sizeof(osada_header);
			printf("Bitmap tendra %d bytes\n", header->bitmap_blocks*sizeof(osada_block));

			char * datosBitmap = (char*)mmap((caddr_t)0, int2size_t(tamanioBitmap), PROT_WRITE, MAP_SHARED, arch, 0);
			if (datosBitmap == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the bitmap");
				exit(EXIT_FAILURE);
			}


			t_bitarray * bitmap = bitarray_create(datosBitmap+sizeof(osada_header), int2size_t(tamanioBitmap- sizeof(osada_header)));
			int i;
			printf("bitmap:\n");
			for(i=0; i<size_t2int(bitarray_get_max_bit(bitmap)); i++){
				printf("%d", bitarray_test_bit(bitmap, i));
			}
		}

	//Mapeo  de tabla de archivos
		if(arch >= 0 && header!=NULL){
			int tamanioArchivos = header->bitmap_blocks*sizeof(osada_block) + sizeof(osada_header) + 1024*sizeof(osada_block) ;
			printf("Tabla de archivos tendra %d bytes\n", 1024*sizeof(osada_block));

			osada_file * tablaArchivos = (char*)mmap((caddr_t)0, int2size_t(tamanioArchivos), PROT_WRITE, MAP_SHARED, arch, 0);
			tablaArchivos = tablaArchivos+header->bitmap_blocks*sizeof(osada_block) + sizeof(osada_header);
			if (tablaArchivos == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the bitmap");
				exit(EXIT_FAILURE);
			}

			int i;
			for(i=0; i<2048; i++){
				printf("Estado: %s\n",stringFromOsadaFileState(tablaArchivos->state));
				printf("Nombre de archivo: %s\n",tablaArchivos->fname);
				printf("Bloque padre: %d\n",tablaArchivos->parent_directory);
				printf("Tamanio del archivo: %d\n",tablaArchivos->file_size);
				printf("Fecha ultima modificacion: %d\n",tablaArchivos->lastmod);
				printf("Bloque inicial: %d\n",tablaArchivos->first_block);

				tablaArchivos++;
			}
		}

	close(arch);

	return 1;
}
