/*
 * helper.h
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "NSGAII.h"


Individuo * new_individuo(int drivers_qtd, int riders_qtd);
Population* new_empty_population(int max_capacity);

Fronts* new_front_list(int max_capacity);
//void clean_front_list(Fronts * f);
Graph * parse_file(char *filename);
void dealoc_graph(Graph*g);
//void dealoc_population(Population *p);
void shuffle(int *array, int n);
void free_population(Population *population);
void dealoc_full_population(Population *population);
void dealoc_empty_population(Population *population);
void dealoc_fronts(Fronts * f);
void print(Population *p);
Graph *new_graph(int drivers, int riders, int total_requests);

#endif /* HELPER_H_ */
