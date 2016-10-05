/*
 * planificador.h
 *
 *  Created on: 2/10/2016
 *      Author: utnso
 */

#ifndef THREADS_PLANIFICADOR_H_
#define THREADS_PLANIFICADOR_H_

void procesarEntrenadoresPreparados();
void procesarEntrenadoresBloqueados();

int recvWithGarbageCollector(int socket, char* paquete, int cantBytes, t_entrenador* entrenador);
int sendWithGarbageCollector(int socket, char* paquete, int cantBytes, t_entrenador* entrenador);

t_entrenador* obtenerSiguienteEntrenadorPlanificadoRR(t_entrenador* entrenadorAnterior);
t_entrenador* obtenerSiguienteEntrenadorPlanificadoSRDF(t_entrenador* entrenadorAnterior);

void atenderEntrenador(t_entrenador* entrenador);
void atenderEntrenadorUbicacionPokenest(t_entrenador* entrenador);
void atenderEntrenadorMover(t_entrenador* entrenador);
void atenderEntrenadorCapturar(t_entrenador* entrenador);

void* planificador(void* arg);

#endif /* THREADS_PLANIFICADOR_H_ */
