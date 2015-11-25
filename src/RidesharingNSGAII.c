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

	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);

	Population * parents = generate_random_population(POPULATION_SIZE, g);
	Population * offspring = generate_offspring(parents);//Tem que garantir que os indivíduos dessa população são diferentes

	int i = 0;
	while(i < 100){
		Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

		merge(parents, offspring, big_population);
		fast_nondominated_sort(big_population, frontsList);
		free_population(parents);
		free_population(offspring);

		Population * next_parents = select_reduced_population(frontsList, POPULATION_SIZE, g);
		free_population_fronts(frontsList);//Desaloca as populações, incluindo seus indivíduos e o front!
		parents = next_parents;
		offspring = generate_offspring(parents);
	}

	complete_free_population(parents);
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

