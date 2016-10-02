#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <commons/bitarray.h>
#include "commons/osada.h"
#include "commons/declarations.h"
#include "functions/dump.h"

void initOsada (char* pathOsadaDrive){
	//Abro el archivo
		int arch;
		arch=open(pathOsadaDrive, O_RDWR, (mode_t)0600);
		if(arch==-1){
			perror("No se pudo abrir el drive Osada");
			exit(EXIT_FAILURE);
		}

	//Obtengo el  tamanio  del archivo
		struct stat sbuf;
		if (stat(pathOsadaDrive, &sbuf) == -1) {
			close(arch);
			perror("No se pudo obtener los atributos del drive Osada");
			exit(EXIT_FAILURE);
		}

	//Mappeo el archivo
		char* fileMapped = mmap(0, sbuf.st_size-1, PROT_WRITE, MAP_SHARED, arch, 0);
		if (fileMapped == MAP_FAILED) {
			close(arch);
			perror("No se pudo mappear el drive Osada");
			exit(EXIT_FAILURE);
		}

	//Header
		osada_drive.header = (osada_header*)fileMapped;
		//dumpHeader(header);
		fileMapped = fileMapped  + OSADA_BLOCK_SIZE; //Muevo el offset
	//Bitmap
		osada_drive.bitmap = bitarray_create_with_mode(fileMapped, osada_drive.header->fs_blocks/8, MSB_FIRST);
		//dumpBitmap(bitmap);
		fileMapped = fileMapped  + osada_drive.header->bitmap_blocks * OSADA_BLOCK_SIZE; //Muevo el offset
	//Tabla de archivos
		osada_drive.directorio = (osada_file *)fileMapped;
		//dumpFileTable(directorio);
		fileMapped = fileMapped  + 1024 * OSADA_BLOCK_SIZE; //Muevo el offset
	//Tabla de asignaciones
		osada_drive.asignaciones = (osada_block_pointer*)fileMapped;
		//dumpAllocationsTable(asignaciones);
		fileMapped = fileMapped + (osada_drive.header->fs_blocks - osada_drive.header->allocations_table_offset - osada_drive.header->data_blocks) * OSADA_BLOCK_SIZE;
	//Data
		osada_drive.data = (osada_block*)fileMapped;
	//munmap????
		close(arch);
}
