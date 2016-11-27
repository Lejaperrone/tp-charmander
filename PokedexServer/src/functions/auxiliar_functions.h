/*
 * auxiliar_functions.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#ifndef FUNCTIONS_AUXILIAR_FUNCTIONS_H_
#define FUNCTIONS_AUXILIAR_FUNCTIONS_H_

	char* obtenerNombreDelDirectorio(char* path);
	void devolverResultadoAlCliente(int resultadoDeOsada,int socketCliente);
	int recibirNombreDeLaFuncion(int socketCliente, char* nombreFuncion);
	void recibirParametrosDeReadDir(int socketCliente,char* path);
	void recibirTamanioDelPath(int socketCliente, int* tamanio);
	void recibirPath(int socketCliente,char** path, int tamanioPath);
	void recibirBuffer(int socketCliente, file_attr* getAttr);
	void enviarBufferLleno(int socketCliente, file_attr* getAttr);

#endif /* FUNCTIONS_AUXILIAR_FUNCTIONS_H_ */
