/*
 * structures.h
 *
 *  Created on: 5/10/2016
 *      Author: utnso
 */

#ifndef COMMONS_STRUCTURES_H_
#define COMMONS_STRUCTURES_H_

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

char *PORT;
char *IP_SERVER;


int pokedexServer; //Renombar a socket
t_log* archivoLog;
char* puntoPontaje;

/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */
struct t_runtime_options {
	char* welcome_msg;
} runtime_options;
/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }


/** keys for FUSE_OPT_ options */
enum {
	KEY_VERSION,
	KEY_HELP,
};

pthread_mutex_t mutexSocket;

#endif /* COMMONS_STRUCTURES_H_ */
