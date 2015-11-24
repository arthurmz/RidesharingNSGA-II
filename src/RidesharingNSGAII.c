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

//	Fronts *frontsList = new_front_list(POPULATION_SIZE*2);


	//Population *pop = generate_random_population(POPULATION_SIZE);
	//Population *offspring = generate_offspring(pop);
	//Population merge = new_population(population_size*2);

	//int iteration = 0;
	//while (iteration < ITERATIONS){
		//merge(pop, offspring, merge);
		//fast_nondominated_sort(merge, frontsList);
		//choose_for_next_population(frontsList, pop);
		//generate_offspring(offspring);
	//}

	//Fronts* frontsList = new_front_list(10);
	//clean_front_list(frontsList);

	//fast_nondominated_sort(Population *p, frontsList);
	//crowding_distance_assignment(frontsList->list[0]);
	return EXIT_SUCCESS;
}

Population * generate_next_population(frontsList){

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

	Fronts *frontsList = new_front_list(POPULATION_SIZE*2);

	Population * pop = generate_random_population(POPULATION_SIZE, g);
	fast_nondominated_sort(pop, frontsList);
	/*Population p_next = reduce_population(frontsList);
	Population *offspring = generate_offspring(frontsList);
	Population large_population = new_population(POPULATION_SIZE*2);

	int iterations = 0;
	while(iterations < ITERATIONS){
		fast_nondominated_sort(large_population, Fronts * frontsList);
		Population p_next = reduce_population(frontsList);
		generate_offspring(p_next);


		iterations++;
	}
	*/

	dealoc_population(pop);
	dealoc_graph(g);
	return EXIT_SUCCESS;
}

