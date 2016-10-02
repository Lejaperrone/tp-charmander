/*
 * planificador.h
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#ifndef THREADS_PLANIFICADOR_H_
#define THREADS_PLANIFICADOR_H_

void* planificador(void* arg);
void atenderEntrenador(t_entrenador* entrenador);
void procesarEntrenadoresPreparados();

t_pokenest *find_pokenest_by_id(char id);

#endif /* THREADS_PLANIFICADOR_H_ */
