/*
 * NSGAII.c
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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
	if (b->dominates_list == NULL){
		b->dominates_list = malloc(16* sizeof(Individuo*));
		b->dominates_list_capacity = 16;
	}

	if (b->dominates_list_capacity <= b->dominates_list_count){
		b->dominates_list_capacity += 16;
		Individuo **temp = realloc(b->dominates_list, b->dominates_list_capacity * sizeof(Individuo*));
		if (temp != NULL) b->dominates_list = temp;
	}
	b->dominates_list[b->dominates_list_count] = a;
	a->ref_count++;
	b->dominates_list_count++;

}

void fast_nondominated_sort(Population *population, Fronts * fronts){
	fronts->size = 0;//Como
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

void sort_by_crowding_distance_assignment(Population *front){
	crowding_distance_assignment(front);
	int i, j, max;
	Individuo *aux;
	int tam = front->size;
	for (i = 0; i < (tam-1); i++){
		max = i;
		for (j = (i+1); j < tam; j++) {
			if(front->list[j]->crowding_distance > front->list[max]->crowding_distance)
			max = j;
		}
		if (i != max) {
			aux = front->list[i];
			front->list[i] = front->list[max];
			front->list[max] = aux;
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


bool is_dentro_janela_tempo(Rota * rota){

	for (int i = 0; i < rota->length-1; i++){
		Service *source = &rota->list[i];
		if (!source->is_source) continue;
		for (int j = i+1; j < rota->length; j++){
			Service *destiny = &rota->list[j];
			if(destiny->is_source || source->r != destiny->r) continue;

			if ( ! ((source->r->pickup_earliest_time <= source->time && source->time <= source->r->pickup_latest_time)
				&& (destiny->r->delivery_earliest_time <= destiny->time && destiny->time <= destiny->r->delivery_latest_time)))
				return false;
		}
	}
	return true;

}
/*Verifica se durante toda a rota a carga permanece dentro do limite
 * e se todos os passageiros embarcados são desembarcados, terminando com carga 0*/
bool is_carga_dentro_limite(Rota *rota){
	int temp_load = 0;
	for (int i = 1; i < rota->length-1; i++){
		Service *a = &rota->list[i];

		if(a->is_source){
			temp_load += RIDER_DEMAND;
			if (temp_load > VEHICLE_CAPACITY) return false;
		}
		else{
			temp_load -= RIDER_DEMAND;
			if (temp_load < 0) return false;
		}
	}

	if (temp_load != 0) return false;
	return true;
}

double distancia_percorrida(Rota * rota){
	double accDistance =0;
	for (int i = 0; i < rota->length -1; i++){
		Request *a = rota->list[i].r;
		Request *b = rota->list[i+1].r;
		accDistance += haversine(a,b);
	}

	return accDistance;
}

/*Verifica se a distancia percorrida pelo motorista é respeitada*/
bool is_distancia_motorista_respeitada(Rota * rota){
	Service * source = &rota->list[0];
	Service * destiny = &rota->list[rota->length-1];
	double MTD = AD + BD * haversine(source->r, destiny->r);//Maximum Travel Distance
	double accDistance = distancia_percorrida(rota);
	return accDistance <= MTD;
}

double tempo_gasto_rota(Rota *rota, int i, int j){
	double accTime =0;
	for (int k = i; k < j; k++){
		Request *a = rota->list[k].r;
		Request *b = rota->list[k+1].r;
		accTime += time_between_requests(a,b);
	}
	return accTime;
}

/*Verifica se o tempo do request partindo do índice I e chegando no índice J é respeitado*/
bool is_tempo_respeitado(Rota *rota, int i, int j){
	Service * source = &rota->list[i];
	Service * destiny = &rota->list[j];
	double MTT = AT + BT * time_between_requests(source->r, destiny->r);
	double accTime = tempo_gasto_rota(rota, i, j);
	return accTime <= MTT;
}

/*Verifica se os tempos de todos os requests nessa rota estão sendo respeitados*/
bool is_tempos_respeitados(Rota *rota){
	for (int i = 0; i < rota->length-1; i++){//Pra cada um dos sources
		Service *source = &rota->list[i];
		if (!source->is_source) continue;
		for (int j = i+1; j < rota->length; j++){//Repete o for até encontrar o destino
			Service *destiny = &rota->list[j];
			if(destiny->is_source || source->r != destiny->r) continue;

			if (!is_tempo_respeitado(rota, i, j)) return false;
		}
	}
	return true;
}


/*Restrição 2 e 3 do artigo é garantida pois sempre que um carona é adicionado
 * seu destino tbm é adicionado
 * Restrição 4 é garantida pois ao fazer o match o carona não pode ser usado pra outras inserções
 * Restrição 5, uma vez que só pode ser feito match uma vez, tá garantido
 * Restrição 6 tem que garantir que a hora que eu chego no ponto J não pode ser maior do que a
 * soma da hora de chegada no ponto anterior com o tempo de viajgem entre IJ.
 * Restrição 7 tem que garantir que a janela de tempo está sendo satisfeitaa TODO
 * Restrição 8 já é atendida pela forma de inserção do carona
 * Restirção 9 e 10 A carga da rota tem que ser verificada a cada inserção
 * OU seja, tem que verificar que a todo instante a carga está dentro do limite
 * Restrição 11 é garantida pela forma como é feita a inserção
 * Restrição 12 e 13 é a verificação da distancia e tempo do motorista
 * Restrição 14 é a verificação de tempo do carona*/
bool is_rota_valida(Rota *rota){

	/*Verificando se os tempos de chegada em cada ponto atende às janelas de tempo de cada request (Driver e Rider)*/
	bool janela_tempo = is_dentro_janela_tempo(rota);
	bool carga_dentro_limite = is_carga_dentro_limite(rota);
	bool tempos_respeitados = is_tempos_respeitados(rota);
	bool distancia_motorista_respeitada = is_distancia_motorista_respeitada(rota);

	return janela_tempo && carga_dentro_limite && tempos_respeitados && distancia_motorista_respeitada;
}

/*Calcula a hora em que chega no próximo ponto, à partir do ponto informado
 * Considera que o tempo de espera do anterior já está calculado*/
double calculate_time_at(Service * actual, Service *ant){
	double next_time = 0;
	if (ant->is_source){
		next_time = ant->time + ant->r->service_time_at_source + time_between_requests(ant->r, actual->r) + ant->waiting_time;
	}
	else{
		next_time = ant->time + ant->r->service_time_at_delivery + time_between_requests(ant->r, actual->r);
	}
	return next_time;
}

/*Atualiza as Horas de chegada e o tempo de espera em cada ponto*/
void update_times(Rota *rota){
	for (int i = 0; i < rota->length-1; i++){
		Service *ant = &rota->list[i];
		Service *actual = &rota->list[i+1];

		actual->time = calculate_time_at(actual, ant);
		if (actual->is_source)
			actual->waiting_time = fmax(0, actual->r->pickup_earliest_time - actual->time);
		else
			actual->waiting_time = 0;
	}
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
	if (posicao_insercao <= 0 || offset <= 0 || carona == NULL) return false;

	int ultimaPos = rota->length-1;
	//Empurra todo mundo depois da posição de inserção
	for (int i = ultimaPos; i >= posicao_insercao; i--){
		rota->list[i+1] = rota->list[i];
	}
	//Empurra todo mundo depois da posição do offset
	for (int i = ultimaPos+1; i >= posicao_insercao + offset; i--){
		rota->list[i+1] = rota->list[i];
	}

	//Insere o conteúdo do novo carona
	rota->list[posicao_insercao].r = carona;
	rota->list[posicao_insercao].is_source = true;
	//Insere o conteúdo do destino do carona
	rota->list[posicao_insercao+offset].r = carona;
	rota->list[posicao_insercao+offset].is_source = false;

	rota->length += 2;
	carona->matched = true;

	update_times(rota);

	if (!is_rota_valida(rota)){
		desfaz_insercao_carona_rota(rota, carona, posicao_insercao, offset);
		update_times(rota);
		return false;
	}
	return true;
}

/*Pega carona aleatória não visitada
 * Por enquanto não será usado*/
Request *get_carona_aleatoria(Graph *g){
	int min = g->drivers;
	int max = g->riders;

	int rnd = min + (rand() % max);
	Request * carona = &g->request_list[rnd];

	if (carona->matched){
		carona = NULL;
		Request * carona_tmp = &g->request_list[g->drivers];
		for (int i = g->drivers; i < g->total_requests; i++){
			if (!carona_tmp->matched){
				carona = carona_tmp;
				break;
			}
		}
	}
	return carona;
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

/*Remove a marcação de matched dos riders*/
void clean_riders_matches(Graph *g){
	for (int i = g->drivers; i < g->total_requests; i++){
		g->request_list[i].matched = false;
	}
}

void evaluate_objective_functions(Individuo *idv, Graph *g){
	double distance = 0;
	double vehicle_time = 0;
	double rider_time = 0;
	double riders_unmatched = g->riders;
	for (int m = 0; m < idv->size; m++){//pra cada rota
		Rota *r = &idv->cromossomo[m];

		vehicle_time += tempo_gasto_rota(r, 0, r->length-1);
		distance += distancia_percorrida(r);

		for (int i = 0; i < r->length-1; i++){//Pra cada um dos sources
			Service *source = &r->list[i];
			if (!source->is_source)
				continue;
			if (source->r == r->list[0].r)
				riders_unmatched --;
			for (int j = i+1; j < r->length; j++){//Repete o for até encontrar o destino
				Service *destiny = &r->list[j];
				if(destiny->is_source || source->r != destiny->r || source->r == r->list[0].r)
					continue;

				rider_time += tempo_gasto_rota(r, i, j);
				break;
			}
		}
	}
	idv->objetivos[TOTAL_DISTANCE_VEHICLE_TRIP] = distance;
	idv->objetivos[TOTAL_TIME_VEHICLE_TRIPS] = vehicle_time;
	idv->objetivos[TOTAL_TIME_RIDER_TRIPS] = rider_time;
	idv->objetivos[RIDERS_UNMATCHED] = riders_unmatched;

}


/*Inicia a população na memória e então:
 * Pra cada um dos drivers, aleatoriza a lista de Riders, e lê sequencialmente
 * até conseguir fazer match de N caronas. Se até o fim não conseguiu, aleatoriza e segue pro próximo rider*/
Population *generate_random_population(int size, Graph *g){
	Population *p = (Population*) new_empty_population(size);

	/*TODO inserir sempre os indivíduos sem matchs*/
	int index_array[g->riders];
	for (int l = 0; l < g->riders; l++){
		index_array[l] = l;
	}

	for (int i = 0; i < size; i++){//Pra cada um dos indivíduos idv
		Individuo *idv = new_individuo(g->drivers, g->riders);
		p->list[i] = idv;
		p->size++;

		for (int j = 0; j < g->drivers ; j++){//pra cada uma das rotas
			Rota * rota = &idv->cromossomo[j];
			Request * driver = &g->request_list[j];

			//Insere o motorista na rota
			rota->list[0].r = driver;
			rota->list[0].is_source = true;
			rota->list[0].time = rota->list[0].r->pickup_earliest_time;//Sai na hora mais cedo
			rota->list[1].r = driver;
			rota->list[1].is_source = false;
			rota->list[1].time = rota->list[0].r->delivery_earliest_time;//Chega na hora mais cedo
			rota->length = 2;


			//Insere mais N caronas
			int qtd_caronas_inserir = rand() % 5;//Outro parâmetro tirado do bolso
			int caronas_inseridos = 0;
			shuffle(index_array, g->riders);

			for (int z = 0; z < g->riders; z++){
				Request * carona = &g->request_list[index_array[z]];
				if (carona->matched) continue;

				int posicao_inicial = 1 + (rand () % (rota->length-1));
				int offset = 1;//TODO, variar o offset

				bool conseguiu = insere_carona_rota(rota, carona, posicao_inicial, offset);

				if (conseguiu){
					//printf("Carona inserido com sucesso!\n");
					caronas_inseridos++;
				}

				if (caronas_inseridos == qtd_caronas_inserir) break;
			}
		}
		evaluate_objective_functions(idv, g);
		clean_riders_matches(g);
	}
	return p;
}

/*Gera uma população de filhos, usando seleção, crossover e mutação*/
Population * generate_offspring(Population *parents){
	Population *offspring = (Population*) new_empty_population(parents->size);

	return offspring;
}
