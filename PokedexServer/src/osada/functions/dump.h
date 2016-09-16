/*
 * dump.h
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */

#include <commons/bitarray.h>
#include "../commons/osada.h"

#ifndef DUMP_H_
#define DUMP_H_

	char* stringFromOsadaFileState(osada_file_state f);
	void dumpHeader(osada_header* header);
	void dumpBitmap(t_bitarray * bitmap);
	void dumpFileTable(osada_file * tablaArchivos);
	void dumpAllocationsTable(osada_block_pointer* tablaAsignaciones);

#endif /* DUMP_H_ */
