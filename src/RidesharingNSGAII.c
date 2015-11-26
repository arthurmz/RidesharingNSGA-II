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
	//Parametros (vari�veis)
	int POPULATION_SIZE;
	int ITERATIONS;
	float crossoverProbability = 0.75;
	char *filename = argv[1];

	sscanf(argv[2], "%d", &POPULATION_SIZE);
	sscanf(argv[3], "%d", &ITERATIONS);
	Graph * g = (Graph*)parse_file(filename);
	if (g == NULL) return 0;

	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);

	Population * parents = generate_random_population(POPULATION_SIZE, g);
	evaluate_objective_functions_pop(parents, g);
	Population * offspring = generate_offspring(parents, g, crossoverProbability);//Tem que garantir que os indiv�duos dessa popula��o s�o diferentes

	int i = 0;
	while(i < 100){
		printf("n�mero de repeti��es %d\n", i);
		Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

		printf("chegou aqui", i);
		merge(parents, offspring, big_population);
		fast_nondominated_sort(big_population, frontsList);
		free_population(parents);
		free_population(offspring);

		printf("chegou aqui 2\n", i);

		Population * next_parents = select_reduced_population(frontsList, POPULATION_SIZE, g);
		printf("chegou aqui b\n", i);
		free_population_fronts(frontsList);//Desaloca as popula��es, incluindo seus indiv�duos e o front!
		printf("chegou aqui cc \n");
		parents = next_parents;
		printf("chegou aqui a\n", i);
		offspring = generate_offspring(parents, g, crossoverProbability);
		printf("chegou aqui 3\n", i);
		i++;
	}

	//complete_free_population(parents);
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

