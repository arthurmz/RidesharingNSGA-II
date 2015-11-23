/*
 ============================================================================
 Name        : RidesharingNSGAII-Clean.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Helper.h"
#include "NSGAII.h"

int maiddn(void) {
	//Parametros (variáveis)
	int POPULATION_SIZE = 10;
	int ITERATIONS = 100;

	Fronts *frontsList = new_front_list(POPULATION_SIZE*2);


	//Population *pop = generate_random_population(POPULATION_SIZE);
/*	Population *offspring = generate_offspring(pop);
	Population merge = new_population(population_size*2);

	int iteration = 0;
	while (iteration < ITERATIONS){
		merge(pop, offspring, merge);
		fast_nondominated_sort(merge, frontsList);
		choose_for_next_population(frontsList, pop);
		generate_offspring(offspring);
	}

	Fronts* frontsList = new_front_list(10);
	clean_front_list(frontsList);
*/
	//fast_nondominated_sort(Population *p, frontsList);
	//crowding_distance_assignment(frontsList->list[0]);
	return EXIT_SUCCESS;
}



/*Parametros: nome do arquivo*/
int main(int argc,  char** argv){
	if (argc < 4) {
		printf("Argumentos insuficientes\n");
		return 0;
	}
	srand (time(NULL));
	//Parametros (variáveis)
	int POPULATION_SIZE;
	int ITERATIONS;
	char *filename = argv[1];

	sscanf(argv[2], "%d", &POPULATION_SIZE);
	sscanf(argv[3], "%d", &ITERATIONS);
	Graph * g = (Graph*)parse_file(filename);
	if (g == NULL) return 0;

	Population * p = generate_random_population(POPULATION_SIZE, g);
	Population * offspring = generate_offspring(p);



	dealoc_population(p);
	dealoc_graph(g);
	return EXIT_SUCCESS;
}

