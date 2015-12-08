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

void print(Population *p){
	for (int i = 0; i < p->size; i++){
		Individuo *id = p->list[i];
		printf("%f %f %f %f\n",id->objetivos[0], id->objetivos[1], id->objetivos[2], id->objetivos[3]);
	}
}

/*Parametros: nome do arquivo
 *
 * Inicia com 3 populações
 * Pais - Indivíduos alocados
 * Filhos -Indivíduos alocados
 * Big_population - indivíduos NÃO alocados*/
int main(int argc,  char** argv){

	/*Setup======================================*/
	if (argc < 4) {
		printf("Argumentos insuficientes\n");
		return 0;
	}
	srand (time(NULL));
	//Parametros (variáveis)
	int POPULATION_SIZE;
	int ITERATIONS;
	float crossoverProbability = 0.75;
	float mutationProbability = 0.1;
	char *filename = argv[1];

	sscanf(argv[2], "%d", &POPULATION_SIZE);
	sscanf(argv[3], "%d", &ITERATIONS);
	Graph * g = (Graph*)parse_file(filename);
	if (g == NULL) return 0;
	/*Setup======================================*/

	//Aloca uma lista de indivíduos com o dobro da capacidade de uma população
	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);
	//Aloca uma lista de populações inicializadas cujos indivíduos não estão inicializados
	Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

	//Gera uma população de indivíduos válidos
	Population * parents = generate_random_population(POPULATION_SIZE, g);
	//Gera uma população de indivíduos válidos
	Population * children = generate_random_population(POPULATION_SIZE, g);

	//Avalia cada um dos objetivos de cada um dos indivíduos da população
	evaluate_objective_functions_pop(parents, g);

	//Insere os
	//fast_nondominated_sort(parents, frontsList);
	//select_parents_by_rank(frontsList, parents, children, g);

	crossover_and_mutation(parents, children, g, crossoverProbability);

	evaluate_objective_functions_pop(children, g);
	printf("Imprimindo a propulação inicial:\n");
	print(parents);
	printf("Imprimindo os filhos\n");
	print(children);

	/*
	printf("Imprimindo a primeira rodada do merge\n");
	merge(parents, children, big_population);
	fast_nondominated_sort(big_population, frontsList);
	print(frontsList->list[0]);
	*/

	int i = 0;
	while(i < ITERATIONS){
		evaluate_objective_functions_pop(children, g);
		merge(parents, children, big_population);
		fast_nondominated_sort(big_population, frontsList);
		//Aloca os melhores entre os pais e filhos (que foram parar em frontsList) e joga em pais
		//O restante irá para os filhos, que de qualquer forma será sobreescrito pelo crossover.
		select_parents_by_rank(frontsList, parents, children, g);
		crossover_and_mutation(parents, children, g, crossoverProbability);
		i++;
	}

	merge(parents, children, big_population);
	fast_nondominated_sort(big_population, frontsList);
	evaluate_objective_functions_pop(frontsList->list[0], g);
	printf("Imprimindo o ultimo front obtido:\n");
	print(frontsList->list[0]);


	//complete_free_population(parents);
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

