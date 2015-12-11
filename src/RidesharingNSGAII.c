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

/*Parametros: nome do arquivo
 *
 * Inicia com 3 popula��es
 * Pais - Indiv�duos alocados
 * Filhos -Indiv�duos alocados
 * Big_population - indiv�duos N�O alocados*/
int main(int argc,  char** argv){

	/*Setup======================================*/
	if (argc < 4) {
		printf("Argumentos insuficientes\n");
		return 0;
	}
	srand (time(NULL));
	//Parametros (vari�veis)
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

	//Aloca uma lista de indiv�duos com o dobro da capacidade de uma popula��o
	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);
	//Aloca uma lista de popula��es inicializadas cujos indiv�duos n�o est�o inicializados
	Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

	printf("Gerando uma popula��o de indiv�duos v�lidos\n");
	Population * parents = generate_random_population(POPULATION_SIZE, g, true);
	//Gera uma popula��o de indiv�duos v�lidos
	printf("Gerando a popula��o de filhos\n");
	Population * children = generate_random_population(POPULATION_SIZE, g, false);

	printf("Avaliando cada um dos objetivos de cada um dos indiv�duos da popula��o\n");
	evaluate_objective_functions_pop(parents, g);

	//Insere os
	printf("Fast Nondominated Sort da popula��o inicial\n");
	fast_nondominated_sort(parents, frontsList);
	printf("Select Parents By rank do primeiro front para os pais\n");
	select_parents_by_rank(frontsList, parents, children, g);

	printf("Crossover e muta��o inicial\n");
	printf("Parentes size %d\n", parents->size);
	printf("Children size %d\n", children->size);
	crossover_and_mutation(parents, children, g, crossoverProbability);

	printf("Avaliando cada um dos objetivos de cada um dos indiv�duos dos FILHOS\n");
	evaluate_objective_functions_pop(children, g);

	printf("Imprimindo a propula��o inicial:\n");
	print(parents);
	printf("Imprimindo os filhos\n");
	print(children);

	printf("Realizando o primeiro Merge\n");
	merge(parents, children, big_population);

	printf("Fast Nondominated Sort inicial, resultado\n");
	fast_nondominated_sort(big_population, frontsList);
	print(frontsList->list[0]);
/*

	int i = 0;
	while(i < ITERATIONS){
		evaluate_objective_functions_pop(children, g);
		merge(parents, children, big_population);
		fast_nondominated_sort(big_population, frontsList);
		//Aloca os melhores entre os pais e filhos (que foram parar em frontsList) e joga em pais
		//O restante ir� para os filhos, que de qualquer forma ser� sobreescrito pelo crossover.
		select_parents_by_rank(frontsList, parents, children, g);
		crossover_and_mutation(parents, children, g, crossoverProbability);
		i++;
	}

	merge(parents, children, big_population);
	fast_nondominated_sort(big_population, frontsList);
	evaluate_objective_functions_pop(frontsList->list[0], g);
	printf("Imprimindo o ultimo front obtido:\n");
	print(frontsList->list[0]);

	 */
	//complete_free_population(parents);
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

