#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include "osada.h"
#include "dump.h"

int main (){
	int arch;
	arch=open("miArchivo.bin", O_RDWR, (mode_t)0600);

	struct stat sbuf;
	if (stat("miArchivo.bin", &sbuf) == -1) {
		perror("stat");
		exit(1);
	}

	printf("miArchivo.bin ocupa %li bytes\n", sbuf.st_size-1);

	int currentOffset = 0;

	//Mapeo el header
		osada_header* header;
		if(arch >= 0){
			header = mmap(0, OSADA_BLOCK_SIZE, PROT_WRITE, MAP_SHARED, arch, 0);
			if (header == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the header");
				exit(EXIT_FAILURE);
			}
			//dump
				dumpHeader(header);
		}

	//Mapeo el bitmap
		if(arch >= 0 && header!=NULL){
			currentOffset = currentOffset + OSADA_BLOCK_SIZE;
			int length = header->fs_blocks/8;
			printf("Bitmap tendra %d bytes\n", length);

			char * datosBitmap = mmap(0, length+currentOffset, PROT_WRITE, MAP_SHARED, arch, 0);
			if (datosBitmap == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the bitmap");
				exit(EXIT_FAILURE);
			}

			datosBitmap = datosBitmap+currentOffset;
			t_bitarray * bitmap = bitarray_create(datosBitmap, length);

			//dump
				dumpBitmap(bitmap);
		}

	//Mapeo  de tabla de archivos
		if(arch >= 0 && header!=NULL){
			currentOffset = currentOffset + header->bitmap_blocks * OSADA_BLOCK_SIZE;
			int length = 1024 * OSADA_BLOCK_SIZE;
			printf("Tabla de archivos tendra %d bytes\n", 1024*sizeof(osada_block));

			osada_file * tablaArchivos = (char*)mmap(0, length + currentOffset, PROT_WRITE, MAP_SHARED, arch, 0);
			if (tablaArchivos == MAP_FAILED) {
				close(arch);
				perror("Error mmapping the file table");
				exit(EXIT_FAILURE);
			}
			tablaArchivos = tablaArchivos + currentOffset;

			//dump
				dumpFileTable(tablaArchivos);
		}

	close(arch);

	return 1;
}
