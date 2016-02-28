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
	int PRINT_ALL_GENERATIONS = 0;
	float crossoverProbability = 0.7;
	float mutationProbability = 0.1;
	char *filename = argv[1];

	sscanf(argv[2], "%d", &POPULATION_SIZE);
	sscanf(argv[3], "%d", &ITERATIONS);
	if (argc >= 5)
		sscanf(argv[4], "%d", &PRINT_ALL_GENERATIONS);
	Graph * g = (Graph*)parse_file(filename);
	if (g == NULL) return 0;

	/*============================================*/
	/*Configurando o index_array usado na aleatorização
	 * da ordem de leitura DAS ROTAS*/
	int index_array[g->drivers];
	for (int l = 0; l < g->drivers; l++){
		index_array[l] = l;
	}
	shuffle(index_array, g->drivers);

	/*Calculando os caronas que são combináveis para cada motorista*/
	Individuo * individuo_teste = generate_random_individuo(g, index_array, false);
	for (int i = 0; i < g->drivers; i++){
		Request * motoristaGrafo = &g->request_list[i];
		for (int j = g->drivers; j < g->total_requests; j++){
			Request * carona = &g->request_list[j];
			Rota * rotaIndividuoTeste = &individuo_teste->cromossomo[i];
			if ( insere_carona_rota(rotaIndividuoTeste, carona, 1, 1) ){
				motoristaGrafo->matchable_riders_list[motoristaGrafo->matchable_riders++] = carona;
				desfaz_insercao_carona_rota(rotaIndividuoTeste, 1, 1);
			}
		}
	}



	/*Imprimindo quantos caronas cada motorista consegue fazer match*/
	/*int qtd = 0;
	printf("quantos matches cada motorista consegue\n");
	for (int i = 0; i < g->drivers; i++){
		if (g->request_list[i].matchable_riders > 0)
			qtd++;
		printf("%d: ",g->request_list[i].matchable_riders);
		for (int j = 0; j < g->request_list[i].matchable_riders; j++){
			printf("%d ", g->request_list[i].matchable_riders_list[j]->req_no);
		}
		printf("\n");
	}

	printf("qtd mínima que deveria conseguir: %d\n", qtd);*/

	/*============================================*/

	
	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);
	Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

	
	Population * parents = generate_random_population(POPULATION_SIZE, g, index_array, true);
	Population * children = generate_random_population(POPULATION_SIZE, g, index_array, false);
	evaluate_objective_functions_pop(parents, g);

	int i = 0;
	while(i < ITERATIONS){
		printf("Iteracao %d...\n", i);
		evaluate_objective_functions_pop(children, g);
		merge(parents, children, big_population);
		fast_nondominated_sort(big_population, frontsList);

		//Aloca os melhores entre os pais e filhos (que foram parar em frontsList) e joga em pais
		//O restante irá para os filhos, que de qualquer forma será sobreescrito pelo crossover.
		select_parents_by_rank(frontsList, parents, children, g);
		crossover_and_mutation(parents, children, g, crossoverProbability);
		if (PRINT_ALL_GENERATIONS)
			print(children);
		i++;
	}
	
	//Ao sair do loop, verificamos uma ultima vez o melhor gerado entre os pais e filhos

	evaluate_objective_functions_pop(parents, g);
	evaluate_objective_functions_pop(children, g);
	merge(parents, children, big_population);
	fast_nondominated_sort(big_population, frontsList);

	evaluate_objective_functions_pop(frontsList->list[0], g);
	printf("Imprimindo o ultimo front obtido:\n");
	sort_by_objective(frontsList->list[0], RIDERS_UNMATCHED);
	print(frontsList->list[0]);

	//printf("Imprimindo o front children\n");
	//print(children);
	//complete_free_population(parents);
	//dealoc_graph(g);
	return EXIT_SUCCESS;
}

