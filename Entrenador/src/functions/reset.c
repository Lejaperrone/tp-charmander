/*
 * reset.c
 *
 *  Created on: 20/11/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "../commons/constants.h"
#include "../commons/structures.c"
#include <commons/collections/dictionary.h>
#include <commons/string.h>

int doesFileExist(char* filename){
  FILE* fptr = fopen(filename, "r");
  if (fptr != NULL)
  {
    fclose(fptr);
    return 1;
  }
  return 0;
}

int remove_directory(const char *path)
{
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d){
      struct dirent *p;
      r = 0;

      while (!r && (p=readdir(d))){
          int r2 = -1;
          char *buf;
          size_t len;

          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
          {
             continue;
          }

          len = path_len + strlen(p->d_name) + 2;
          buf = malloc(len);

          if (buf){
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);

             if (!stat(buf, &statbuf)){
                if (S_ISDIR(statbuf.st_mode)){
                   r2 = remove_directory(buf);
                } else {
                   r2 = unlink(buf);
                }
             }

             free(buf);
          }

          r = r2;
      }

      closedir(d);
   }

   return r;
}

void reiniciarObjetivo(t_objetivo* objetivo){
	objetivo->ubicacion.x = -1;
	objetivo->ubicacion.y = -1;
	objetivo->logrado=0;
}

void reiniciarMapa(t_mapa* mapa){
	mapa->miPosicion.x = 1;
	mapa->miPosicion.y = 1;
	mapa->terminado = 0;

	list_iterate(mapa->objetivos, (void*)reiniciarObjetivo);

	char* pathMedalla = string_new();
	string_append(&pathMedalla, pokedexPath);
	string_append(&pathMedalla, "/Entrenadores/");
	string_append(&pathMedalla, name);
	string_append(&pathMedalla, "/medallas/medalla-");
	string_append(&pathMedalla, mapa->nombre);
	string_append(&pathMedalla, ".jpg");

	if(doesFileExist(pathMedalla)){
		if(unlink(pathMedalla) == -1){
			log_info(archivoLog, "No pudo borrarse la medalla: %c", pathMedalla);
		}
	}

	char* pathBill = string_new();
	string_append(&pathBill, pokedexPath);
	string_append(&pathBill, "/Entrenadores/");
	string_append(&pathBill, name);
	string_append(&pathBill, "/Dir de Bill/");

	remove_directory(pathBill);
}

void reiniciarJuego(){
	char* pathMedallas = string_new();
	string_append(&pathMedallas, pokedexPath);
	string_append(&pathMedallas, "/Entrenadores/");
	string_append(&pathMedallas, name);
	string_append(&pathMedallas, "/medallas/");

	remove_directory(pathMedallas);
}

void actualizarMetadata(){
	char* path = string_new();
	string_append(&path, pokedexPath);
	string_append(&path, "/Entrenadores/");
	string_append(&path, name);
	string_append(&path, "/metadata");

	char* pathTemp = string_new();
	string_append(&pathTemp, pokedexPath);
	string_append(&pathTemp, "/Entrenadores/");
	string_append(&pathTemp, name);
	string_append(&pathTemp, "/temp_metadata");


	FILE * fp;
	FILE * fptemp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(path, "rb");
	fptemp = fopen(pathTemp, "wb+");
	if (fp != NULL){
		if (fptemp != NULL){
			while ((read = getline(&line, &len, fp)) != -1) {
				if(string_equals_ignore_case(string_substring(line,0,10),"reintentos")){
					fprintf(fptemp, "reintentos=%i\n", entrenador->reintentos);
				}else if(string_equals_ignore_case(string_substring(line,0,5),"vidas")){
					fprintf(fptemp, "vidas=%i\n", entrenador->vidas);
				}else{
					fprintf(fptemp, "%s", line);
				}
			}
			fclose(fptemp);
		}
		fclose(fp);
	}

	unlink(path);
	rename(pathTemp, path);
}

