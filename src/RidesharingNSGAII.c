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


/*Copia o conteúdo das duas populações na terceira*/
void merge(Population *p1, Population *p2, Population *big_population){
	for (int i = 0; i < p1->size + p2->size; i++){
		if (i < p1->size){
			big_population->list[i] = p1->list[i];
		}
		else{
			big_population->list[i] = p2->list[i];
		}
	}
	big_population->size = p1->size + p2->size;
}


/*Gera uma população reduzida à partir dos fronts passados
 * TODO - Cada indivíduo deve ser uma cópia nova que ESQUECE a lista de dominados*/
Population * generate_reduced_population(Fronts *frontsList, int p_size, Graph *g){
	Population *p_next = (Population*) new_empty_population(p_size);
	int added = 0;
	int lastPosition = 0;
	for (int i = 0; i < frontsList->size; i++){
		Population * p_local = frontsList->list[i];

		if (p_size - added >= p_local->size){
			for (int j = 0; j < p_local->size; j++){
				Individuo *idv = new_individuo_by_individuo(p_local->list[j]);
				p_next->list[j] = idv;
				p_next->size++;
				added++;
			}
		}
		else{
			lastPosition = i;
			break;
		}
	}

	int restantes = p_size - added;
	if (restantes > 0 && lastPosition < frontsList->size){
		sort_by_crowding_distance_assignment(frontsList->list[lastPosition]);
		for (int k = 0; k < restantes; k++){
			Individuo *idv = new_individuo_by_individuo(frontsList->list[k]);
			p_next->list[p_next->size] = frontsList->list[k];
		}
	}

	return p_next;

}
for (int i = 0; i < size; i++){//Pra cada um dos indivíduos idv
		Individuo *idv = new_individuo(g->drivers, g->riders);
		p->list[i] = idv;
		p->size++;



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

	/*Merge da população - usado para evitar operações de alocação*/
	Population *big_population = (Population*) new_empty_population(POPULATION_SIZE*2);

	Fronts *frontsList = new_front_list(POPULATION_SIZE * 2);

	Population * parents = generate_random_population(POPULATION_SIZE, g);
	Population * offspring = generate_offspring(parents);//Tem que garantir que os indivíduos dessa população são diferentes

	int i = 0;
	while(i < 100){
		merge(parents, offspring, big_population);//big_population então guarda as referencias dos indv de parents e offspring
		empty_front_list(frontsList);//<<<<< Precisa desalocar os indivíduos!!
		fast_nondominated_sort(big_population, frontsList);//Aqui, todas as ref de indv foram encaixadas no frontList

		Population next_parents = generate_reduced_population(frontsList, POPULATION_SIZE, parents);//< Cria uma nova população de novos indivíduos
		free_population(parents);//Desaloca a população, inclusive os indivíduos!!
		parents = next_parents;
		offspring = generate_offspring(parents);//Aqui sim, offspring é uma população nova com novos indivíduos
	}

	dealoc_population(parents);
	dealoc_graph(g);
	return EXIT_SUCCESS;
}

