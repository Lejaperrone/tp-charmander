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
void procesorPokemonEntrenadorGarbagecollector(t_pokemon_custom* pokemon);
void procesarEntrenadorGarbageCollector(t_entrenador* entrenador);
void procesarEntrenadoresGarbageCollector();

int recvWithGarbageCollector(int socket, char* package, int cantBytes, t_entrenador* entrenador);
int sendWithGarbageCollector(int socket, char* package, int cantBytes, t_entrenador* entrenador);

int calcularDistanciaEntrenadorObjetivo(t_entrenador* entrenador);
t_entrenador* obtenerProximoEntrenadorCercano();

t_entrenador* obtenerSiguienteEntrenadorPlanificadoRR(t_entrenador* entrenadorAnterior);
t_entrenador* obtenerSiguienteEntrenadorPlanificadoSRDF(t_entrenador* entrenadorAnterior);

void atenderEntrenador(t_entrenador* entrenador);
void atenderEntrenadorUbicacionPokenest(t_entrenador* entrenador);
void atenderEntrenadorMover(t_entrenador* entrenador);
void atenderEntrenadorCapturar(t_entrenador* entrenador);

void logEntrenadoresListos();
void logEntrenadoresBloqueados();
void logColasEntrenadores();

void* planificador(void* arg);

#endif /* THREADS_PLANIFICADOR_H_ */
