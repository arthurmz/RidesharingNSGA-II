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

/*Ordena os indivíduos segundo o critério de não dominação*/
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

/*Pra poder usar a função qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compare0(const void *p, const void *q) {
    int ret;
    Individuo * x = (const Individuo *)p;
    Individuo * y = (const Individuo *)q;
    if (x->objetivos[0] == y->objetivos[0])
        ret = 0;
    else if (x->objetivos[0] < y->objetivos[0])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare1(const void *p, const void *q) {
    int ret;
    Individuo * x = (const Individuo *)p;
    Individuo * y = (const Individuo *)q;
    if (x->objetivos[1] == y->objetivos[1])
        ret = 0;
    else if (x->objetivos[1] < y->objetivos[1])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare2(const void *p, const void *q) {
    int ret;
    Individuo * x = (const Individuo *)p;
    Individuo * y = (const Individuo *)q;
    if (x->objetivos[2] == y->objetivos[2])
        ret = 0;
    else if (x->objetivos[2] < y->objetivos[2])
        ret = -1;
    else
        ret = 1;
    return ret;
}

int compare3(const void *p, const void *q) {
    int ret;
    Individuo * x = (const Individuo *)p;
    Individuo * y = (const Individuo *)q;
    if (x->objetivos[3] == y->objetivos[3])
        ret = 0;
    else if (x->objetivos[3] < y->objetivos[3])
        ret = -1;
    else
        ret = 1;
    return ret;
}





/*Deve ser chamado depois de determinar as funções objetivo*/
void crowding_distance_assignment(Population *front_i){
	for (int i = 0; i < front_i->size; i++){
		front_i->list[i]->crowding_distance = 0;
	}
	for (int k = 0; k < QTD_OBJECTIVES; k++){
		switch(k){
		case 0:
			qsort(front_i->list, front_i->size, sizeof(Individuo*), compare0 );
			break;
		case 1:
			qsort(front_i->list, front_i->size, sizeof(Individuo*), compare1 );
			break;
		case 2:
			qsort(front_i->list, front_i->size, sizeof(Individuo*), compare2 );
			break;
		case 3:
			qsort(front_i->list, front_i->size, sizeof(Individuo*), compare3 );
			break;
		}

		front_i->list[0]->crowding_distance = INT_MAX;
		front_i->list[front_i->size -1]->crowding_distance = INT_MAX;

		float obj_min = front_i->list[0]->objetivos[k];//valor max do obj k
		float obj_max = front_i->list[front_i->size -1]->objetivos[k];//valor min do obj k


		float diff = fmax(0.001, abs(obj_max - obj_min));


		for (int z = 1; z < front_i->size -1; z++){
			float prox_obj = front_i->list[z+1]->objetivos[k];
			float ant_obj = front_i->list[z-1]->objetivos[k];

			front_i->list[z]->crowding_distance += abs(prox_obj - ant_obj) / diff;
		}

	}
}

/*Pra poder usar a função qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compareByCrowdingDistanceMax(const void *p, const void *q) {
    int ret;
    Individuo * x = (const Individuo *)p;
    Individuo * y = (const Individuo *)q;
    if (x->crowding_distance == y->crowding_distance)
        ret = 0;
    else if (x->crowding_distance > y->crowding_distance)
        ret = -1;
    else
        ret = 1;
    return ret;
}

void sort_by_crowding_distance_assignment(Population *front){
	crowding_distance_assignment(front);
	qsort(front->list, front->size, sizeof(Individuo*), compareByCrowdingDistanceMax );
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

void evaluate_objective_functions_pop(Population* p, Graph *g){
	for (int i = 0; i < p->size; i++){
		evaluate_objective_functions(p->list[i], g);
	}
	crowding_distance_assignment(p);
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
		clean_riders_matches(g);
	}
	return p;
}



/*Gera uma população reduzida à partir dos fronts passados
 * TODO - Cada indivíduo deve ser uma cópia nova que ESQUECE a lista de dominados*/
Population * select_reduced_population(Fronts *frontsList, int p_size, Graph *g){
	Population *newPopulation = (Population*) new_empty_population(p_size);
	int added = 0;
	int lastPosition = 0;
	for (int i = 0; i < frontsList->size; i++){
		Population * front_i = frontsList->list[i];

		if (p_size - added >= front_i->size){
			for (int j = 0; j < front_i->size; j++){
				Individuo *idv = new_individuo_by_individuo(front_i->list[j], g);
				newPopulation->list[j] = idv;
				newPopulation->size++;
				added++;
			}
		}
		else{
			lastPosition = i;
			break;
		}
	}

	int restantes = p_size - added;
	//Só entra aqui se a quantidade de indivíduos no front de posição lastPosition é maior do que o restante à adc
	if (restantes > 0 && lastPosition < frontsList->size){
		//Population *front_i = frontsList->list[t];
		sort_by_crowding_distance_assignment(frontsList->list[lastPosition]);
		for (int k = 0; k < restantes; k++){
			Individuo *idv = new_individuo_by_individuo(frontsList->list[lastPosition]->list[k], g);
			newPopulation->list[newPopulation->size] = idv;
			newPopulation->size++;
		}
	}

	return newPopulation;

}

/*Copia o conteúdo das duas populações na terceira.
 * é uma cópia simples, onde assume-se que os indivíduos estão na heap */
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


Individuo * new_individuo_by_individuo(Individuo *p, Graph * g){
	Individuo *idv = new_individuo(g->drivers, g->riders);

	idv->crowding_distance = p->crowding_distance;
	idv->dominated_by_count = p->dominated_by_count;
	for (int k = 0; k < QTD_OBJECTIVES; k++){
		idv->objetivos[k] = p->objetivos[k];
	}
	return idv;
}

/*seleção por torneio, k = 2*/
Individuo * tournamentSelection(Population * parents){
	Individuo * best = NULL;
	for (int i = 0; i < 2; i++){
		int pos = rand() % parents->size;
		Individuo * outro = parents->list[pos];
		if (best == NULL || crowded_comparison_operator(outro, best))
			best = outro;
	}
	return best;
}

void crossover(Individuo * parent1, Individuo *parent2, Individuo *offspring1, Individuo *offspring2, float crossoverProbability){
	int rotaSize = parent1->size;
	offspring1->size = rotaSize;
	offspring2->size = rotaSize;

	int crossoverPoint = 1 + (rand() % (rotaSize-1));
	float accept = (float)rand() / RAND_MAX;

	if (accept < crossoverProbability){
		int i = 0;
		for (i = 0; i < crossoverPoint; i++){
			Rota r = parent2->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				offspring1->cromossomo[i].list[j].r = r.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = r.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota r = parent1->cromossomo[i];
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				offspring1->cromossomo[i].list[j].r = r.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = r.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = 0; i < crossoverPoint; i++){
			Rota r = parent1->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				offspring2->cromossomo[i].list[j].r = r.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = r.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota r = parent2->cromossomo[i];
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				offspring2->cromossomo[i].list[j].r = r.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = r.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}

	}
	else{
		int i = 0;
		for (i = 0; i < crossoverPoint; i++){
			Rota r = parent1->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				offspring1->cromossomo[i].list[j].r = r.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = r.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota r = parent1->cromossomo[i];
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				offspring1->cromossomo[i].list[j].r = r.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = r.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = 0; i < crossoverPoint; i++){
			Rota r = parent2->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				offspring2->cromossomo[i].list[j].r = r.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = r.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota r = parent2->cromossomo[i];
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				offspring2->cromossomo[i].list[j].r = r.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = r.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = r.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = r.list[j].waiting_time;
			}
		}
	}
}



/*Gera uma população de filhos, usando seleção, crossover e mutação*/
Population * generate_offspring(Population *parents, Graph *g, float crossoverProbability ){
	Population *offspring = (Population*) new_empty_population(parents->size);

	Individuo *parent1 = tournamentSelection(parents);
	Individuo *parent2 = tournamentSelection(parents);

	Individuo *offspring1 = new_individuo(g->drivers, g->riders);
	Individuo *offspring2 = new_individuo(g->drivers, g->riders);

	crossover(parent1, parent2, offspring1, offspring2, crossoverProbability);


	return offspring;
}

