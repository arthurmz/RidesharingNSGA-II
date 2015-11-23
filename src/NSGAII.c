/*
 * NSGAII.c
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#include <limits.h>
#include <stdio.h>
#include "StaticVariables.h"
#include "Helper.h"
#include "NSGAII.h"



/*Adiciona o indivíduo de rank k no front k de FRONTS
 * Atualiza o size de FRONTS caso o rank seja maior*/
void add_Individuo_front(Fronts * fronts, Individuo *p){
	Population *fronti = fronts->list[p->rank];
	fronti->list[fronti->size] = p;
	fronti->size++;
	if (fronts->size < p->rank + 1)
		fronts->size = p->rank + 1;
}

/*Verifica se A domina B (melhor em pelo menos 1 obj)*/
bool dominates(Individuo *a, Individuo *b){
	bool minor_found = false;
	for (int i = 0; i < QTD_OBJECTIVES; i++){
		if (a->objetivos[i] < b->objetivos[i])
			minor_found = true;
		if (a->objetivos[i] > b->objetivos[i])
			return false;
	}
	return minor_found;
}

void add_dominated(Individuo *b, Individuo *a){
	int index = b->dominates_list_count;
	b->dominates_list[index] = a;
	b->dominated_by_count++;
}

void fast_nondominated_sort(Population *population, Fronts * fronts){

	//Primeiro passo, computando as dominancias
	for (int i = 0; i < population->size; i++){
		Individuo *a = population->list[i];
		for (int j = 0; j < population->size; j++){
			if (i == j) continue;
			Individuo *b = population->list[j];
			if (dominates(a,b)){
				b->dominated_by_count++;
				add_dominated(a, b);
			}
			else if (dominates(b,a)){
				a->dominated_by_count++;
				add_dominated(b, a);
			}
		}
		if (a->dominated_by_count == 0){
			a->rank = 0;
			add_Individuo_front(fronts, a);
		}
	}

	int index_front = 0;
	//Iterando enquanto existirem novos fronts
	while (index_front < fronts->size){
		Population * front_i = fronts->list[index_front];
		//Iterando sobre os elementos do front index_front
		for (int i = 0; i < front_i->size; i++){
			Individuo *p = front_i->list[i];

			for (int k = 0; k < p->dominates_list_count; k++){
				Individuo *indv_dominated = p->dominates_list[k];
				indv_dominated->dominated_by_count--;
				if (indv_dominated->dominated_by_count == 0){
					indv_dominated->rank = index_front+1;
					add_Individuo_front(fronts, indv_dominated);
				}
			}
		}
		index_front++;
	}
}

/**/
void crowding_distance_assignment(Population *front_i){
	for (int i = 0; i < front_i->size; i++){
		front_i->list[i]->crowding_distance = 0;
	}
	for (int k = 0; k < QTD_OBJECTIVES; k++){
		sort(front_i, k);

		front_i->list[0]->crowding_distance = INT_MAX;
		front_i->list[front_i->size -1]->crowding_distance = INT_MAX;

		float fmax = front_i->list[0]->objetivos[k];//valor max do obj k
		float fmin = front_i->list[front_i->size -1]->objetivos[k];//valor min do obj k


		for (int z = 1; z < front_i->size -1; z++){
			float prox_obj = front_i->list[z+1]->objetivos[k];
			float ant_obj = front_i->list[z-1]->objetivos[k];
			front_i->list[z]->crowding_distance += (prox_obj - ant_obj) / (fmax-fmin);
		}

	}

}

bool crowded_comparison_operator(Individuo *a, Individuo *b){
	return (a->rank < b->rank || (a->rank == b->rank && a->crowding_distance > b->crowding_distance));
}

/*Constroi um novo grafo em memória*/
Graph *new_graph(int drivers, int riders, int total_requests){
	Graph * g = calloc(1, sizeof(Graph));
	g->request_list = calloc(total_requests, sizeof(Request));
	g->drivers = drivers;
	g->riders = riders;
	g->total_requests = total_requests;
	return g;
}

bool is_rota_valida(Service *rota){
	return false;
}

/*
 * A0101B = tamanho 6
 * 			de 0 a 5
 * 	Inserir na posiçao 0 não pode pq já tem o motorista
 * 	Inserir na posição 1, empurra os demais pra frente
 * 	Inserir o destino é contado à partir da origem (offset)
 * 	offset = 0 não pode pq é o proprio origem, 1 pode e é o próximo,
 * 	2 é o que pula um e insere.
 * */
bool insere_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset){
	if (posicao_insercao <= 0 || offset <= 0) return false;

	int ultimaPos = rota->length-1;

	for (int i = ultimaPos; i >= posicao_insercao; i--){
		rota->list[i+1] = rota->list[i];
	}
	rota->list[posicao_insercao].r = carona;
	rota->list[posicao_insercao].source = true;

	for (int i = ultimaPos+1; i >= posicao_insercao + offset; i--){
		rota->list[i+1] = rota->list[i];
	}
	rota->list[posicao_insercao+offset].r = carona;
	rota->list[posicao_insercao+offset].source = false;

	rota->length += 2;
	carona->matched = true;

	return(is_rota_valida(rota));
}

void desfaz_insercao_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset){
	if (posicao_insercao <= 0 || offset <= 0) return;

	for (int i = posicao_insercao; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;

	for (int i = posicao_insercao+offset-1; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;
	carona->matched = false;
}


Population *generate_random_population(int size, Graph *g){
	Population *p = (Population*) new_empty_population(size);

	/*TODO inserir sempre os indivíduos sem matchs*/

	for (int i = 0; i < size; i++){//Pra cada um dos indivíduos idv
		Individuo *idv = new_individuo(g->drivers, g->riders);
		p->list[i] = idv;

		for (int j = 0; j < g->drivers ; j++){//pra cada uma das rotas
			Rota * rota = &idv->cromossomo[j];
			Request * driver = &g->request_list[j];

			//Insere o motorista na rota
			rota->list[0].r = driver;
			rota->list[0].source = true;
			rota->list[1].r = driver;
			rota->list[1].source = false;
			rota->length = 2;


			//Insere mais N caronas
			int qtd_caronas_inserir = rand() % 5;//Outro parâmetro tirado do bolso
			int caronas_inseridos = 0;
			bool inserir_novo_carona = true;
			while (inserir_novo_carona){

				int posicao_inicial = 1;
				int offset = 1;
				Request * carona = NULL;

				bool conseguiu = insere_carona_rota(rota, carona, posicao_inicial, offset);
				if (!conseguiu){
					desfaz_insercao_carona_rota(rota, carona, posicao_inicial, offset);
				}

				if (caronas_inseridos == qtd_caronas_inserir)
					inserir_novo_carona = false;
			}
		}
	}
	return p;
}

/*Insere um "request" Driver ou Rider em uma determinada rota
 * na posição N*/
void insert_request_route(){

}

