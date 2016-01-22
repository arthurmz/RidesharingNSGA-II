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
	bool smaller_found = false;
	for (int i = 0; i < QTD_OBJECTIVES; i++){
		if (a->objetivos[i] < b->objetivos[i])
			smaller_found = true;
		if (a->objetivos[i] > b->objetivos[i])
			return false;
	}
	return smaller_found;
}

void add_dominated(Individuo *a, Individuo *b){
	if (a->dominates_list == NULL){
		a->dominates_list = malloc(32* sizeof(Individuo*));
		a->dominates_list_capacity = 32;
	}

	if (a->dominates_list_capacity <= a->dominates_list_count){
		a->dominates_list_capacity += 32;
		Individuo **temp = realloc(a->dominates_list, a->dominates_list_capacity * sizeof(Individuo*));
		if (temp != NULL) a->dominates_list = temp;
	}
	a->dominates_list[a->dominates_list_count] = b;
	a->dominates_list_count++;

}

/*Ordena os indivíduos segundo o critério de não dominação*/
void fast_nondominated_sort(Population *population, Fronts * fronts){
	/*====================Zerando o frontlist==================================*/
	for (int i = 0; i < fronts->size; i++){
		fronts->list[i]->size = 0;
	}
	fronts->size = 0;

	/*===================Zerando o dominated counts============================*/
	for (int i = 0; i < population->size; i++){
		population->list[i]->dominated_by_count = 0;
		population->list[i]->dominates_list_count = 0;
	}


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
void crowding_distance_assignment(Population *pop){
	for (int i = 0; i < pop->size; i++){
		pop->list[i]->crowding_distance = 0;
	}
	for (int k = 0; k < QTD_OBJECTIVES; k++){
		switch(k){
		case 0:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare0 );
			break;
		case 1:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare1 );
			break;
		case 2:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare2 );
			break;
		case 3:
			qsort(pop->list, pop->size, sizeof(Individuo*), compare3 );
			break;
		}

		pop->list[0]->crowding_distance = INT_MAX;
		pop->list[pop->size -1]->crowding_distance = INT_MAX;

		float obj_min = pop->list[0]->objetivos[k];//valor min do obj k
		float obj_max = pop->list[pop->size -1]->objetivos[k];//valor max do obj k

		float diff = fmax(0.0001, obj_max - obj_min);

		for (int z = 1; z < pop->size -1; z++){
			float prox_obj = pop->list[z+1]->objetivos[k];
			float ant_obj = pop->list[z-1]->objetivos[k];

			pop->list[z]->crowding_distance += (prox_obj - ant_obj) / diff;
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


/*Calculoa o tempo gasto para ir do ponto i ao ponto j, através de cada
 * request da rota.*/
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
		desfaz_insercao_carona_rota(rota, posicao_insercao, offset);
		carona->matched = false;
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

void desfaz_insercao_carona_rota(Rota *rota, int posicao_insercao, int offset){
	if (posicao_insercao <= 0 || offset <= 0) return;

	for (int i = posicao_insercao; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;

	for (int i = posicao_insercao+offset-1; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;
}

/*Remove a marcação de matched dos riders*/
void clean_riders_matches(Graph *g){
	for (int i = g->drivers; i < g->total_requests; i++){
		g->request_list[i].matched = false;
	}
}

void evaluate_objective_functions_pop(Population* p, Graph *g){
	for (int i = 0; i < p->size; i++){//Pra cada um dos indivíduos
		evaluate_objective_functions(p->list[i], g);
		clean_riders_matches(g);
	}
	//crowding_distance_assignment(p);
}

void evaluate_objective_functions(Individuo *idv, Graph *g){
	double distance = 0;
	double vehicle_time = 0;
	double rider_time = 0;
	double riders_unmatched = g->riders;
	for (int m = 0; m < idv->size; m++){//pra cada rota
		Rota *rota = &idv->cromossomo[m];

		vehicle_time += tempo_gasto_rota(rota, 0, rota->length-1);
		distance += distancia_percorrida(rota);

		for (int i = 0; i < rota->length-1; i++){//Pra cada um dos sources services
			Service *service = &rota->list[i];
			if (service->r->driver || !service->is_source)//só contabiliza os services source que não é o motorista
				continue;
			riders_unmatched--;
			//Repete o for até encontrar o destino
			//Ainda não considera o campo OFFSET contido no typedef SERVICE
			for (int j = i+1; j < rota->length; j++){
				Service *destiny = &rota->list[j];
				if(destiny->is_source || service->r != destiny->r)
					continue;

				rider_time += tempo_gasto_rota(rota, i, j);
				break;
			}
		}
	}

	idv->objetivos[TOTAL_DISTANCE_VEHICLE_TRIP] = distance;
	idv->objetivos[TOTAL_TIME_VEHICLE_TRIPS] = vehicle_time;
	idv->objetivos[TOTAL_TIME_RIDER_TRIPS] = rider_time;
	idv->objetivos[RIDERS_UNMATCHED] = riders_unmatched;

}

void copy(Population * source, Population *destiny){
	for (int i = 0; i < source->size; i++){
		for (int j = 0; j < source->list[i]->size; j++){
			for (int k = 0; k < source->list[i]->cromossomo[j].length; k++){
				destiny->list[i]->cromossomo[j].list[k].is_source = source->list[i]->cromossomo[j].list[k].is_source;
				destiny->list[i]->cromossomo[j].list[k].r = source->list[i]->cromossomo[j].list[k].r;
				destiny->list[i]->cromossomo[j].list[k].time = source->list[i]->cromossomo[j].list[k].time;
				destiny->list[i]->cromossomo[j].list[k].waiting_time = source->list[i]->cromossomo[j].list[k].waiting_time;
			}
			destiny->list[i]->cromossomo[j].length = source->list[i]->cromossomo[j].length;
		}
		destiny->list[i]->crowding_distance = source->list[i]->crowding_distance;
		destiny->list[i]->dominated_by_count = source->list[i]->dominated_by_count;//não copia a lista de dominados. chegando aqui tem que tá vazia
		destiny->list[i]->dominates_list_capacity = source->list[i]->dominates_list_capacity;
		destiny->list[i]->dominates_list_count = source->list[i]->dominates_list_count;
		destiny->list[i]->objetivos[0] = source->list[i]->objetivos[0];
		destiny->list[i]->objetivos[1] = source->list[i]->objetivos[1];
		destiny->list[i]->objetivos[2] = source->list[i]->objetivos[2];
		destiny->list[i]->objetivos[3] = source->list[i]->objetivos[3];
		destiny->list[i]->objetivos[4] = source->list[i]->objetivos[4];
		destiny->list[i]->rank = source->list[i]->rank;
		destiny->list[i]->size = source->list[i]->size;
	}
	destiny->max_capacity = source->max_capacity;//Espera que de todo jeito sejam do mesmo tamanho
	destiny->size = source->size;
	destiny->id_front = source->id_front;
}

/*Insere uma quantidade variável de caronas na rota informada
 * Utilizado na geração da população inicial, e na reparação dos indivíduos quebrados*/
void insere_carona_aleatoria_rota(int index_array[], Graph *g, Rota* rota, int tentativas){
	int qtd_caronas_inserir = VEHICLE_CAPACITY;
	shuffle(index_array, g->riders);

	for (int z = 0; z < g->riders; z++){
		if (qtd_caronas_inserir == 0 || tentativas == 0) break;
		Request * carona = &g->request_list[index_array[z]];
		if (carona->matched) {
			tentativas--;
			continue;
		}

		int posicao_inicial = 1 + (rand () % (rota->length-1));
		int offset = 1;//TODO, variar o offset

		bool conseguiu = insere_carona_rota(rota, carona, posicao_inicial, offset);

		if (conseguiu){
			qtd_caronas_inserir--;
		}
		else{
			tentativas--;
		}
	}
}


/*Inicia a população na memória e então:
 * Pra cada um dos drivers, aleatoriza a lista de Riders, e lê sequencialmente
 * até conseguir fazer match de N caronas. Se até o fim não conseguiu, aleatoriza e segue pro próximo rider*/
Population *generate_random_population(int size, Graph *g, bool insereCaronasAleatorias){
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
			if (driver == NULL)
				printf("Driver é nulo\n");

			//Insere o motorista na rota
			rota->list[0].r = driver;
			rota->list[0].is_source = true;
			rota->list[0].time = rota->list[0].r->pickup_earliest_time;//Sai na hora mais cedo
			rota->list[0].offset = 1;//Informa que o destino está logo à frente
			rota->list[1].r = driver;
			rota->list[1].is_source = false;
			rota->list[1].time = rota->list[0].r->delivery_earliest_time;//Chega na hora mais cedo
			rota->length = 2;

			if (insereCaronasAleatorias)
				insere_carona_aleatoria_rota(index_array, g, rota, 20);
		}
		//Depois de inserir todas as rotas, limpa a lista de matches
		//Para que o próximo indivíduo possa usa-las
		if (insereCaronasAleatorias)
			clean_riders_matches(g);
	}
	return p;
}



/*Pega os melhores N indivíduos do frontList e joga na população pai.
 * Os restantes vão pra população filho.
 * Remove da lista de pais e filhos as listas de dominação
 * "esvazia" o frontsList
 * */
void select_parents_by_rank(Fronts *frontsList, Population *parents, Population *offsprings, Graph *g){
	int lastPosition = 0;
	parents->size = 0;
	offsprings->size = 0;

	/*Para cada um dos fronts, enquanto a qtd de elementos dele couber inteiramente em parents, vai adicionando
	 * Caso contrário para. pois daí pra frente, só algums desses indivíduos irão para o parent
	 * o restante desse front em lastPosition e dos próximos fronts vão pro offsprings*/
	for (int i = 0; i < frontsList->size; i++){
		Population * front_i = frontsList->list[i];
		lastPosition = i;
		crowding_distance_assignment(front_i);
		if (parents->max_capacity - parents->size >= front_i->size){
			for (int j = 0; j < front_i->size; j++){
				parents->list[parents->size++] = front_i->list[j];
			}
		}
		else{
			break;
		}
	}

	//int offspringAdded = 0;

	int restantes_adicionar = parents->max_capacity - parents->size;//Qtd que tem que adicionar aos pais

	//Se restantes_adicionar > 0 então o front atual não comporta todos os elementos de parent
	if (restantes_adicionar > 0){
		sort_by_crowding_distance_assignment(frontsList->list[lastPosition]);//ordena
		for (int k = 0; k < restantes_adicionar; k++){
			parents->list[parents->size++] = frontsList->list[lastPosition]->list[k];//Adiciona o restante aos pais
		}
		//Inserindo no filho o restante desses indivíduos que não couberam nos pais
		for (int k = restantes_adicionar; k < frontsList->list[lastPosition]->size; k++){
			offsprings->list[offsprings->size++] = frontsList->list[lastPosition]->list[k];
		}
		lastPosition++;
	}


	/*Adicionar todos os restantes de bigpopulation aos filhos*/
	while (lastPosition < frontsList->size){
		for (int k = 0; k < frontsList->list[lastPosition]->size; k++){
			offsprings->list[offsprings->size++] = frontsList->list[lastPosition++]->list[k];
		}
	}
}

/*Copia o conteúdo das duas populações na terceira.
 * é uma cópia simples, onde assume-se que os indivíduos estão na heap
 * "esvazia" p1 e p2*/
void merge(Population *p1, Population *p2, Population *big_population){
	big_population->size = 0;//Zera o bigpopulation

	for (int i = 0; i < p1->size + p2->size; i++){
		if (i < p1->size){
			big_population->list[i] = p1->list[i];
		}
		else{
			big_population->list[i] = p2->list[i - p1->size];
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
	for (int r = 0; r < p->size; r++){
		idv->cromossomo[r].length = p->cromossomo[r].length;
		for (int w = 0; w < p->cromossomo[r].length; w++){
			idv->cromossomo[r].list[w].is_source = p->cromossomo[r].list[w].is_source;
			idv->cromossomo[r].list[w].r = p->cromossomo[r].list[w].r;
			idv->cromossomo[r].list[w].time = p->cromossomo[r].list[w].time;
			idv->cromossomo[r].list[w].waiting_time = p->cromossomo[r].list[w].waiting_time;
		}
	}

	idv->size = p->size;

	return idv;
}

/*seleção por torneio, k = 2*/
Individuo * tournamentSelection(Population * parents){
	Individuo * best = NULL;
	for (int i = 0; i < 2; i++){
		int pos = rand() % parents->size;
		Individuo * outro = parents->list[pos];
		if (outro == NULL)
			printf("outro é nulo\n");
		if (best == NULL || crowded_comparison_operator(outro, best))
			best = outro;
	}
	return best;
}

void crossover(Individuo * parent1, Individuo *parent2, Individuo *offspring1, Individuo *offspring2, Graph *g, float crossoverProbability){
	int rotaSize = g->drivers;
	offspring1->size = rotaSize;
	offspring2->size = rotaSize;

	int crossoverPoint = 1 + (rand() % (rotaSize-1));
	float accept = (float)rand() / RAND_MAX;

	if (accept < crossoverProbability){
		int i = 0;
		for (i = 0; i < crossoverPoint; i++){
			Rota rota = parent2->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring1->cromossomo[i].list[j].r = rota.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = rota.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring1->cromossomo[i].length = parent2->cromossomo[i].length;
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota rota = parent1->cromossomo[i];
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring1->cromossomo[i].list[j].r = rota.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = rota.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring1->cromossomo[i].length = parent1->cromossomo[i].length;
		}
		for (i = 0; i < crossoverPoint; i++){
			Rota rota = parent1->cromossomo[i];
			//Copiando os services da rota
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring2->cromossomo[i].list[j].r = rota.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = rota.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring2->cromossomo[i].length = parent1->cromossomo[i].length;
		}
		for (i = crossoverPoint; i < rotaSize; i++){
			Rota rota = parent2->cromossomo[i];
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring2->cromossomo[i].list[j].r = rota.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = rota.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring2->cromossomo[i].length = parent2->cromossomo[i].length;
		}

	}
	else{
		int i = 0;
		for (i = 0; i < rotaSize; i++){
			Rota rota = parent1->cromossomo[i];
			for (int j = 0; j < parent1->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring1->cromossomo[i].list[j].r = rota.list[j].r;
				offspring1->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring1->cromossomo[i].list[j].time = rota.list[j].time;
				offspring1->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring1->cromossomo[i].length = parent1->cromossomo[i].length;
		}
		for (i = 0; i < rotaSize; i++){
			Rota rota = parent2->cromossomo[i];
			for (int j = 0; j < parent2->cromossomo[i].length; j++){
				if (rota.list[j].r == NULL)
					printf("é nulo\n");
				offspring2->cromossomo[i].list[j].r = rota.list[j].r;
				offspring2->cromossomo[i].list[j].is_source = rota.list[j].is_source;
				offspring2->cromossomo[i].list[j].time = rota.list[j].time;
				offspring2->cromossomo[i].list[j].waiting_time = rota.list[j].waiting_time;
			}
			offspring2->cromossomo[i].length = parent2->cromossomo[i].length;
		}
	}
}

/*Remove todas as caronas que quebram a validação
 * Tenta inserir novas
 * Utiliza graph pra saber quem já fez match.
 * */
void repair(Individuo *offspring, Graph *g, int index_array[], int position){

	for (int i = 0; i < offspring->size; i++){//Pra cada rota do idv
		Rota *rota = &offspring->cromossomo[i];

		for (int j = 0; j < rota->length; j++){//pra cada um dos services SOURCES na rota
			//Se é matched então algum SOURCE anterior já usou esse request
			//Então deve desfazer a rota de j até o offset
			if ((rota->list[j].is_source && rota->list[j].r->matched)){
				int offset = 1;
				for (int k = j+1; k < rota->length; k++){//encontrando o offset
					if (rota->list[j].r == rota->list[k].r && !rota->list[k].is_source)
						break;
					offset++;
				}
				desfaz_insercao_carona_rota(rota, j, offset);
				//Nesse ponto, depois de remover, a rota vai estar ok
				insere_carona_aleatoria_rota(index_array, g, rota, 5);
			}
			else{
				rota->list[j].r->matched = true;
			}
		}
	}
}

void mutation(Individuo *ind, Graph *g){

}



/*Gera uma população de filhos, usando seleção, crossover e mutação*/
void crossover_and_mutation(Population *parents, Population *offspring,  Graph *g, float crossoverProbability ){
	int index_array[g->riders];
	for (int l = 0; l < g->riders; l++){
		index_array[l] = l;
	}

	offspring->size = 0;//Tamanho = 0, mas considera todos já alocados
	int i = 0;
	while (offspring->size < parents->size){

		//printf("Passo %d, parentsize %d offspringsize %d \n", i, parents->size, offspring->size);
		Individuo *parent1 = tournamentSelection(parents);
		Individuo *parent2 = tournamentSelection(parents);

		Individuo *offspring1 = offspring->list[i++];
		Individuo *offspring2 = offspring->list[i];

		crossover(parent1, parent2, offspring1, offspring2, g, crossoverProbability);

		clean_riders_matches(g);
		shuffle(index_array, g->riders);
		repair(offspring1, g, index_array, 1);

		clean_riders_matches(g);
		shuffle(index_array, g->riders);
		repair(offspring2, g, index_array, 2);

		mutation(offspring1, g);
		mutation(offspring2, g);
		offspring->size += 2;
	}
}

