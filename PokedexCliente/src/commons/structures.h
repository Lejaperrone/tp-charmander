/*
 * structures.h
 *
 *  Created on: 5/10/2016
 *      Author: utnso
 */

#ifndef COMMONS_STRUCTURES_H_
#define COMMONS_STRUCTURES_H_


#define IP "127.0.0.1" //Deberia estar por variable de entorno
#define PUERTO "7666" //Deberia estar por variable de entorno
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

// Variables de entorno
	//char *PUERTO;
	//char *IP;

int pokedexCliente;
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

/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {
		// Este es un parametro definido por nosotros
		CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};

#endif /* COMMONS_STRUCTURES_H_ */
