/*
 * helper.h
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "NSGAII.h"
#include "StaticVariables.h"
#include <math.h>


Individuo * new_individuo(int drivers_qtd, int riders_qtd);
Population* new_empty_population(int max_capacity);

Fronts* new_front_list(int max_capacity);
void clean_front_list(Fronts * f);
void sort(Population *front_i, int k);
double haversine(Request *a, Request *b);
double haversine_helper(double lat1, double lon1, double lat2, double lon2);
double time_between_requests(Request *a, Request *b);
Graph * parse_file(char *filename);
void dealoc_graph(Graph*g);
void dealoc_population(Population *p);
void shuffle(int *array, int n);
void free_unused_population(Population *population);


#endif /* HELPER_H_ */
