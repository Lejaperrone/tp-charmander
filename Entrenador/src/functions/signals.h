/*
 * signals.h
 *
 *  Created on: 20/11/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "../commons/structures.c"

#ifndef FUNCTIONS_SIGNALS_H_
#define FUNCTIONS_SIGNALS_H_

void sigusr1_handler(int signum);
void sigterm_handler(int signum);

#endif /* FUNCTIONS_SIGNALS_H_ */
