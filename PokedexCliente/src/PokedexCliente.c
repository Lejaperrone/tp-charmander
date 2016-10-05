#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <tad_items.h>
#include <commons/log.h>
#include <stddef.h>
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include "functions/log.h"

#include "socketLib.h"

#define IP "127.0.0.1" //Deberia estar por variable de entorno
#define PUERTO "7666" //Deberia estar por variable de entorno
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

t_log* archivoLog;
char* puntoPontaje;

// Variables de entorno
	//char *PUERTO;
	//char *IP;

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


/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations bb_oper = {
		//.getattr = fuse_getattr,
		//.readdir = fuse_readdir,
		//.open = fuse_open,
		//.read = fuse_read,
};
/*
static int fuse_getattr(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_readdir(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_open(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
static int fuse_read(const char *path, struct stat *stbuf) {
	// Todo Realizar el send de la peticion
	return 0;
}
*/

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

int main(int argc, char *argv[]){

	// Creo archivo log
		archivoLog = crearArchivoLog();

	//Recivo parametros por linea de comandos
		if(argc != 2){
			log_info(archivoLog,"El pokedexCliente no tiene los parametros correctamente seteados.");
			return 1;
		}
		puntoPontaje = argv[1]; //tmp
		log_info(archivoLog,"Punto de montaje: %s", puntoPontaje);

	//IP = getenv("POKEIP");
	//PUERTO = getenv("POKEPORT");

		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
		memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
		if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
			perror("Invalid arguments!");
			return EXIT_FAILURE;
		}

		int pokedexCliente;

		create_socketClient(&pokedexCliente, IP, PUERTO);
		printf("Conectado al servidor\n");
		log_info(archivoLog, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

	//Funcion que se encarga de montar y delegar todo al Kernel
		fuse_main(args.argc, args.argv, &bb_oper, NULL);
		log_info(archivoLog, "Levanto fuse\n");

		close(pokedexCliente);
		return 0;
}

