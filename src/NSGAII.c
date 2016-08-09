/*
 * NSGAII.c
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Helper.h"
#include "Calculations.h"
#include "NSGAII.h"

/** Rota usada para a cópia em operações de mutação etc.*/
Rota *ROTA_CLONE;
Rota *ROTA_CLONE_SWAP;//Outros clones para não conflitar as cópias
Rota *ROTA_CLONE_REMOVE_INSERT;//usado só no op de remove-insert
Rota *ROTA_CLONE_PUSH;

/** Aloca a ROTA_CLONE global */
void malloc_rota_clone(){
	/*Criando uma rota para cópia e validação das rotas*/
	ROTA_CLONE = (Rota*) calloc(1, sizeof(Rota));
	ROTA_CLONE->list = calloc(MAX_SERVICES_MALLOC_ROUTE, sizeof(Service));

	ROTA_CLONE_SWAP = (Rota*) calloc(1, sizeof(Rota));
	ROTA_CLONE_SWAP->list = calloc(MAX_SERVICES_MALLOC_ROUTE, sizeof(Service));

	ROTA_CLONE_REMOVE_INSERT = (Rota*) calloc(1, sizeof(Rota));
	ROTA_CLONE_REMOVE_INSERT->list = calloc(MAX_SERVICES_MALLOC_ROUTE, sizeof(Service));

	ROTA_CLONE_PUSH = (Rota*) calloc(1, sizeof(Rota));
	ROTA_CLONE_PUSH->list = calloc(MAX_SERVICES_MALLOC_ROUTE, sizeof(Service));
}

/**
 * Insere caronas aleatórias para todas as caronas da rota
 * IMPORTANTE: Antes de chamar, todos os caronas já feito match devem estar no grafo
 */
void insere_carona_aleatoria_individuo(Individuo * ind, bool full_search){
	shuffle(index_array_drivers,g->drivers);
	for (int i = 0; i < ind->size; i++){
		int j = index_array_drivers[i];
		insere_carona_aleatoria_rota(&ind->cromossomo[j], full_search);
	}
}

//Insere a carona na rota e empura os tempos de pickup e delivery.
//Completa a inserção mesmo se a rota ficar inválida

/*
 * Novo método de inserção de carona.
 *
 * Rota começa com o motorista inserido no seu earliest time
 * Um novo carona será inserido na hora =
 * MÍNIMO entre seu earliest time e
 * TEMPO(i-1) + DISTÂNCIA(i-1, i);
 *
 * Se esse TEMPO é > o latest_time(i) ROTA É INVÁLIDA!
 *
 * Se esse TEMPO é < o earliest_time(i)
 * os pontos na rota antes de i sofrem um PUSH FORWARD igual ao
 * MINIMO entre earliest_time(i) - TEMPO
 * e o menor push forward possivel nos pontos antes de i.
 *
 * Depois o push é feito normalmente nos pontos após i.
 */
bool insere_carona(Rota *rota, Request *carona, int posicao_insercao, int offset, bool is_source, int * insertFalso){
	Service * ant = NULL;
	Service * atual = NULL;
	Service * next = NULL;
	double PF;
	double nextTime;

	int ultimaPos = rota->length-1;
	//Empurra todo mundo depois da posição de inserção
	for (int i = ultimaPos; i >= posicao_insercao; i--){
		rota->list[i+1] = rota->list[i];
	}
	ant = &rota->list[posicao_insercao-1];
	atual = &rota->list[posicao_insercao];
	next = &rota->list[posicao_insercao+1];

	atual->r = carona;
	atual->is_source = is_source;
	atual->offset = offset;
	atual->service_time = calculate_service_time(atual, ant);

	TIMEWINDOW *tw = get_time_windows_service(atual);

	if (atual->service_time < tw->a){
		//Encontrando o menor PUSH possível
		double minPUSH = 9999999;
		for(int i = 0; i < posicao_insercao; i++){
			double push = get_latest_time_service(&rota->list[i]) - rota->list[i].service_time;
			if (push < minPUSH) minPUSH = push;
		}
		double pushFinal = fmin(minPUSH, tw->a - atual->service_time);
		push_forward_hard(rota, 0, pushFinal);//O push vai executar até exatamente o ponto atrasado!
		if(atual->service_time < tw->a) *insertFalso = 42;
	}
	else if(atual->service_time > tw->b){
		*insertFalso = 42;
	}

	nextTime = calculate_service_time(next, atual);
	PF = nextTime - next->service_time;
	rota->length++;//Deve aumentar o tamanho antes de fazer o PF
	if (PF > 0) {
		push_forward_hard(rota, posicao_insercao+1, PF);
	}
	return true;
}

/*mINIMIZANDO O TEMPO DE ESPERA QUANDO TIVER COLOCANDO O PRIMEIRO CARONA.*/
void encaixando_carona(Rota *rota){
	if(rota->length == 4){
		/*Minimizar o tempo de espera quando está tentando adicionar um único carona à uma rota vazia.
		 * Útil para garantir que, se um carona for encaixavel na rota, essa posição válida do encaixe vai ser encontrada.
		 * Assim podemos contabilizar os riders combináveis de cada driver. Além de, no algoritmo melhorado, não depender de PF nem PB*/

		Service * sv1 =  &rota->list[0];
		Service * sv2 =  &rota->list[1];

		double waitingTime = waiting_time_services(sv1, sv2);
		double remainder = rem(sv1);
		double push = fmin(remainder, waitingTime);

		sv1->service_time += push;

		/*NUNCA VAI TER TEMPO DE ESPERA ENTRE SV3 E SV4.
		 * A rota original já seta as horas de pickup e delivery do motorista como as mais cedo.
		 * QUALQUER carona adicionado faria a hora de delivery no minimo aumentar, nunca diminuir.
		 */
	}
}

bool insere_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset, bool inserir_de_fato, int *insercaoFalso){

	clone_rota(rota, &ROTA_CLONE);
	bool isRotaValida = false;
	insere_carona(ROTA_CLONE, carona, posicao_insercao, offset, true, insercaoFalso);
	insere_carona(ROTA_CLONE, carona, posicao_insercao+offset, 0, false, insercaoFalso);
	encaixando_carona(ROTA_CLONE);

	isRotaValida = is_rota_valida(ROTA_CLONE);

	if (isRotaValida && inserir_de_fato){
		carona->matched = true;
		carona->id_rota_match = ROTA_CLONE->id;
		clone_rota(ROTA_CLONE, &rota);

		increase_capacity(ROTA_CLONE);
		increase_capacity(rota);
	}

	return isRotaValida;
}

/*Insere uma quantidade variável de caronas na rota informada
 * Utilizado na geração da população inicial, e na reparação dos indivíduos quebrados
 * IMPORTANTE: Antes de chamar, os caronas devem estar determinados.
 * full_search: não para de tentar inserir depois de conseguir o primeiro match*/
bool insere_carona_aleatoria_rota(Rota* rota, bool full_search){
	Request * request = &g->request_list[rota->id];
	int qtd_caronas_inserir = request->matchable_riders;
	if (qtd_caronas_inserir == 0 || qtd_caronas_combinados(rota) == qtd_caronas_inserir) return false;
	
	fill_shuffle(index_array_caronas_inserir, 0, qtd_caronas_inserir);

	if (full_search){
		bool ook = false;
		for (int z = 0; z < qtd_caronas_inserir; z++){
			int p = index_array_caronas_inserir[z];
			Request * carona = request->matchable_riders_list[p];
			if (!carona->matched){
				if (qtd_caronas_combinados(rota) == qtd_caronas_inserir)
					return false;
				bool inseriu = false;
				//fill_shuffle(index_array_posicao_inicial,1, rota->length-1);
				for (int posicao_inicial = 1; posicao_inicial < rota->length; posicao_inicial++){
					int insercaoFalso = 0;
					//int posicao_inicial = index_array_posicao_inicial[pi-1];
					//fill_shuffle(index_array_offset, 1, rota->length - posicao_inicial);
					for (int offset = 1; offset <= rota->length - posicao_inicial; offset++){
						//int offset = index_array_offset[ot-1];
						inseriu = insere_carona_rota(rota, carona, posicao_inicial, offset, true, &insercaoFalso);
						if(inseriu) {
							ook = true;
							break;
						}
						else if(insercaoFalso == 42){
							break;
						}
					}
					if(inseriu) {
						break;
					}
				}
			}
		}
		return ook;
	}
	else{
		for (int z = 0; z < qtd_caronas_inserir; z++){
			int p = index_array_caronas_inserir[z];
			Request * carona = request->matchable_riders_list[p];
			if (!carona->matched){
				if (qtd_caronas_combinados(rota) == qtd_caronas_inserir)
					return false;
				bool inseriu = false;
				fill_shuffle(index_array_posicao_inicial,1, rota->length-1);
				for (int pi = 1; pi < rota->length; pi++){
					int insercaoFalso = 0;
					int posicao_inicial = index_array_posicao_inicial[pi-1];
					fill_shuffle(index_array_offset, 1, rota->length - posicao_inicial);
					for (int ot = 1; ot <= rota->length - posicao_inicial; ot++){
						int offset = index_array_offset[ot-1];
						inseriu = insere_carona_rota(rota, carona, posicao_inicial, offset, true, &insercaoFalso);
						if(inseriu) return true;
						if(insercaoFalso == 42) break;
					}
				}
			}
		}
	}
	return false;
}


/** Remove o carona que tem source na posicção Posicao_remocao
 * Retorna o valor do offset para encontrar o destino do carona removido
 * Retorna 0 caso não seja possível fazer a remoção do carona
 */
int desfaz_insercao_carona_rota(Rota *rota, int posicao_remocao){
	if (posicao_remocao > rota->length-2 || posicao_remocao <= 0 || rota->length < 4 || !rota->list[posicao_remocao].is_source) {
		printf("Erro ao desfazer a inserção\n");
		return 0;
	}
	
	int offset = 1;
	for (int k = posicao_remocao+1; k < rota->length; k++){//encontrando o offset
		if (rota->list[posicao_remocao].r == rota->list[k].r && !rota->list[k].is_source)
			break;
		offset++;
	}

	for (int i = posicao_remocao; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;

	for (int i = posicao_remocao+offset-1; i < rota->length-1; i++){
		rota->list[i] = rota->list[i+1];
	}
	rota->length--;

	return offset;
}

/*Remove a marcação de matched dos riders*/
void clean_riders_matches(Graph *g){
	for (int i = g->drivers; i < g->total_requests; i++){
		g->request_list[i].matched = false;
	}
}

/**Avalia as funções objetivo de um indivíduo
 * sem verificar o grafo
 */
void evaluate_objective_functions(Individuo *idv, Graph *g){
	double distance = 0;
	double vehicle_time = 0;
	double rider_time = 0;
	double riders_unmatched = g->riders;
	for (int m = 0; m < idv->size; m++){//pra cada rota
		Rota *rota = &idv->cromossomo[m];

		vehicle_time += tempo_gasto_rota(rota, 0, rota->length-1);
		distance += distancia_percorrida(rota);

		for (int i = 1; i < rota->length-2; i++){//Pra cada um dos sources services
			Service *service = &rota->list[i];
			if (service->r->driver || !service->is_source)//só contabiliza os services source que não é o motorista
				continue;
			riders_unmatched--;
			//Repete o for até encontrar o destino
			//Ainda não considera o campo OFFSET contido no typedef SERVICE
			for (int j = i+1; j < rota->length-1; j++){
				Service *destiny = &rota->list[j];
				if(destiny->is_source || service->r != destiny->r)
					continue;

				rider_time += tempo_gasto_rota(rota, i, j);
				if (rider_time < 0){
					printf("negativo!");
				}
				break;
			}
		}
	}

	idv->objetivos_bruto[TOTAL_DISTANCE_VEHICLE_TRIP] = distance;
	idv->objetivos_bruto[TOTAL_TIME_VEHICLE_TRIPS] = vehicle_time;
	idv->objetivos_bruto[TOTAL_TIME_RIDER_TRIPS] = rider_time;
	idv->objetivos_bruto[RIDERS_UNMATCHED] = riders_unmatched;

	idv->objetivos[TOTAL_DISTANCE_VEHICLE_TRIP] = (distance - TOTAL_DISTANCE_VEHICLE_TRIP_LOWER_BOUND)  / (TOTAL_DISTANCE_VEHICLE_TRIP_UPPER_BOUND - TOTAL_DISTANCE_VEHICLE_TRIP_LOWER_BOUND);
	idv->objetivos[TOTAL_TIME_VEHICLE_TRIPS] = (vehicle_time - TOTAL_TIME_VEHICLE_TRIPS_LOWER_BOUND) / (TOTAL_TIME_VEHICLE_TRIPS_UPPER_BOUND - TOTAL_TIME_VEHICLE_TRIPS_LOWER_BOUND);
	idv->objetivos[TOTAL_TIME_RIDER_TRIPS] = (rider_time - TOTAL_TIME_RIDER_TRIPS_LOWER_BOUND) / (TOTAL_TIME_RIDER_TRIPS_UPPER_BOUND - TOTAL_TIME_RIDER_TRIPS_LOWER_BOUND);
	idv->objetivos[RIDERS_UNMATCHED] = (riders_unmatched - RIDERS_UNMATCHED_LOWER_BOUND) / (RIDERS_UNMATCHED_UPPER_BOUND - RIDERS_UNMATCHED_LOWER_BOUND);
}


void evaluate_objective_functions_pop(Population* p, Graph *g){
	for (int i = 0; i < p->size; i++){//Pra cada um dos indivíduos
		evaluate_objective_functions(p->list[i], g);
	}
}


/*Ordena a população de acordo com o objetivo 0, 1, 2, 3*/
void sort_by_objective(Population *pop, int obj){
	switch(obj){
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
}


int compare_rotas(const void *p, const void *q){
	int ret;
	Request * x = *(Request **)p;
	Request * y = *(Request **)q;
	if (x->matchable_riders == y->matchable_riders)
		ret = 0;
	else if (x->matchable_riders < y->matchable_riders)
		ret = -1;
	else
		ret = 1;
	return ret;
}

/*Empurra os tempos de pickup e delivery de forma fixa. Economizando nos waiting times
 * A rota pode ficar inválida no final*/
void push_forward_hard(Rota *rota, int position, double pushf){
	Service * atual = &rota->list[position];
	if (pushf <= 0) return;
	atual->service_time+= pushf;

	for (int i = position+1; i < rota->length; i++){
		if (pushf <= 0)
			break;
		atual = &rota->list[i];
		Service * ant = &rota->list[i-1];

		double waiting_time = atual->service_time - ant->service_time -  minimal_time_between_services(ant, atual);
		waiting_time = fmax(0, waiting_time);
		pushf = fmax(0, pushf - waiting_time);

		atual->service_time+= pushf;
		if(atual->service_time == get_earliest_time_service(atual))
			break;//Situação em que o push é feito de 0 ao ponto position
	}
}

void push_forward_mutation_op(Rota * rota){
	clone_rota(rota, &ROTA_CLONE_PUSH);
	int position = get_random_int(0, ROTA_CLONE_PUSH->length-1);
	Service * atual = &ROTA_CLONE_PUSH->list[position];
	double maxPushf = get_latest_time_service(atual) -  atual->service_time;
	double pushf = maxPushf * ((double)rand() / RAND_MAX);

	push_forward_hard(ROTA_CLONE_PUSH, position, pushf);

	bool rotaValida = is_rota_valida(ROTA_CLONE_PUSH);
	if (rotaValida){
		clone_rota(ROTA_CLONE_PUSH, &rota);
	}
}

/*Tenta empurar os services uma certa quantidade de tempo
 * retorna true se conseguiu fazer algum push forward
 * forcar_clone:  Mantem as alterações mesmo se o push forward não for feito
 * ou a rota for considerada inválida.*/
bool push_forward(Rota * rota, int position, double pushf, bool forcar_clone){
	clone_rota(rota, &ROTA_CLONE_PUSH);

	if (position == -1)
		position = get_random_int(0, ROTA_CLONE_PUSH->length-1);
	Service * atual = &ROTA_CLONE_PUSH->list[position];
	double maxPushf = get_latest_time_service(atual) -  atual->service_time;

	if (pushf == -1){
		pushf = maxPushf * ((double)rand() / RAND_MAX);
	}
	else{
		pushf = fmin (pushf, maxPushf);
	}

	if (pushf <= 0) return false;

	atual->service_time+= pushf;

	for (int i = position+1; i < ROTA_CLONE_PUSH->length; i++){
		if (pushf <= 0)
			break;
		atual = &ROTA_CLONE_PUSH->list[i];
		Service * ant = &ROTA_CLONE_PUSH->list[i-1];
		double bt = get_latest_time_service(atual);

		double waiting_time = atual->service_time - ant->service_time -  minimal_time_between_services(ant, atual);
		waiting_time = fmax(0, waiting_time);
		pushf = fmax(0, pushf - waiting_time);
		pushf = fmin(pushf, bt - atual->service_time);

		atual->service_time+= pushf;
	}
	bool rotaValida = is_rota_valida(ROTA_CLONE_PUSH);
	if (rotaValida || forcar_clone){
		clone_rota(ROTA_CLONE_PUSH, &rota);
	}
	return rotaValida;
}

/*Puxa os services uma certa quantidade de tempo
 * "Soft" porque se em algum passo não puder fazer o pushb todo, faz o resto que dá pra fazer
 * e continua até o fim. embora a rota final ainda possa ser inválida.
 */
void push_backward_soft(Rota *rota, int position, double pushb){
	//bool rotaValidaAntes = is_rota_valida(rota);
	Service * atual = &rota->list[position];
	//double maisCedoPossivel = get_earliest_time_service(atual);
	//if (position > 0){
	//	Service * ant = &rota->list[position-1];
	//	double srvTime = calculate_service_time(atual, ant);
	//	if (srvTime > maisCedoPossivel)
	//		maisCedoPossivel = srvTime;
	//}

	//double maxPushb = atual->service_time - maisCedoPossivel;
	//pushb = fmin (pushb, maxPushb);

	if (pushb <= 0) return;

	atual->service_time-= pushb;

	for (int i = position+1; i < rota->length; i++){
		if (pushb == 0)
			break;
		atual = &rota->list[i];
		double at = get_earliest_time_service(atual);

		pushb = fmin(pushb, atual->service_time - at);

		atual->service_time-= pushb;
	}
}

/*pode falhar, por isso faz clone*/
void push_backward_mutation_op(Rota * rota, int position){
	if (position == -1)
		position = get_random_int(0, rota->length-1);
	Service * atual = &rota->list[position];
	double maisCedoPossivel = get_earliest_time_service(atual);
	if (position > 0){
		Service * ant = &rota->list[position-1];
		double srvTime = calculate_service_time(atual, ant);
		if (srvTime > maisCedoPossivel)
			maisCedoPossivel = srvTime;
	}
	double maxPushb = atual->service_time - maisCedoPossivel;
	if (maxPushb == 0) return;
	double pushb = maxPushb * ((double)rand() / RAND_MAX);

	clone_rota(rota, &ROTA_CLONE_PUSH);
	push_backward_soft(ROTA_CLONE_PUSH, position, pushb);
	bool rotaValida = is_rota_valida(ROTA_CLONE_PUSH);
	if (rotaValida){
		clone_rota(ROTA_CLONE_PUSH, &rota);
	}
}

/*Tenta puxar os services uma certa quantidade de tempo
 * Se position = -1, gera aleatoriamente a posição*/
bool push_backward(Rota * rota, int position, double pushb, bool forcar_clone){
	clone_rota(rota, &ROTA_CLONE_PUSH);

	if (position == -1)
		position = get_random_int(0, ROTA_CLONE_PUSH->length-1);
	Service * atual = &ROTA_CLONE_PUSH->list[position];
	double ets = get_earliest_time_service(atual);
	double maisCedoPossivel = ets;

	if (position > 0){
		Service * ant = &ROTA_CLONE_PUSH->list[position-1];
		double srvTime = calculate_service_time(atual, ant);
		if (srvTime > maisCedoPossivel)
			maisCedoPossivel = srvTime;
	}

	double maxPushb = atual->service_time - maisCedoPossivel;

	if (pushb == -1){
		pushb = maxPushb * ((double)rand() / RAND_MAX);
	}
	else{
		pushb = fmin (pushb, maxPushb);
	}

	if (pushb <= 0) return false;

	atual->service_time-= pushb;

	for (int i = position+1; i < ROTA_CLONE_PUSH->length; i++){
		if (pushb == 0)
			break;
		atual = &ROTA_CLONE_PUSH->list[i];
		double at = get_earliest_time_service(atual);

		pushb = fmin(pushb, atual->service_time - at);

		atual->service_time-= pushb;
	}
	bool rotaValida = is_rota_valida(ROTA_CLONE_PUSH);
	if (rotaValida || forcar_clone){
		clone_rota(ROTA_CLONE_PUSH, &rota);
	}
	return rotaValida;
}


/** Transfere o carona de uma rota para outra
 *
 * Escolhe uma rota aleatória com carona.
 * Invalida o match temporariamente
 * Escolhe uma rota que possa fazer match com o carona
 * Tenta inserir o carona
 * Se conseguiu, remove o carona da rota original
 */
bool transfer_rider(Rota * rotaRemover, Individuo *ind, Graph * g){
	Rota * rotaInserir = NULL;
	Request * caronaInserir;

	//Procurando um carona qualquer
	int pos = get_random_carona_position(rotaRemover);
	if (pos == -1)
		return false;//Se não achou, retorna
	else
		caronaInserir = rotaRemover->list[pos].r;

	//Se 0 não tem nada pra fazer, se 1 então só pode na própria rota
	if (caronaInserir->matchable_riders < 2)
		return false;

	int k = rand() % caronaInserir->matchable_riders;
	rotaInserir = &ind->cromossomo[caronaInserir->matchable_riders_list[k]->id];

	//Troca a rota, se a escolhida aleatoriamente foi a própria rotaRemover
	if (rotaInserir == rotaRemover){
		if (k < caronaInserir->matchable_riders-1)
			k++;
		else
			k = 0;
		rotaInserir = &ind->cromossomo[caronaInserir->matchable_riders_list[k]->id];
	}

	bool conseguiu = false;
	//Invalida o carona
	caronaInserir->matched = false;

	for (int posicaoInserir = 1; posicaoInserir < rotaInserir->length; posicaoInserir++){
		int insereFalso = 0;
		for (int offset = 1; offset <= rotaInserir->length - posicaoInserir; offset++){
			conseguiu = insere_carona_rota(rotaInserir, caronaInserir, posicaoInserir, offset, true, &insereFalso);
			if(conseguiu || insereFalso == 42) break;
		}
		if(conseguiu) break;
	}

	//Se conseguiu inserir, remove o carona do rotaRemover
	if (conseguiu)
		desfaz_insercao_carona_rota(rotaRemover,pos);
	//Sempre vai ser válido em uma ou outra outra
	caronaInserir->matched = true;

	return conseguiu;
}


/** O bug mais difícil de descobrir:
 * get_random_int retornava qualquer coisa dentre o
 * primeiro carona e o ultimo source de carona.
 * mas se vc tem a seguinte rota
 * M C+ C+ C+ C+ M-
 *
 * ele poderia escolher o valor 3, que é um DESTINO do carona
 * A solução: além de adicionar uma verificação da posição de remoção do
 * desfaz_insercao_carona_rota, é fazer um mecanismo de buscar números aleatórios
 * ímpares.
 *
 *
 * Outro bug complicado: Remove_insert necessita que os matches das
 * rotas estejam determinados. para poder saber de onde tirar e onde colocar.
 *
 *
 * Update final:
 * As posições podem sim ser ímpares.
 * Retorna false se o resultado for inválido
 * (Seria um erro pois o push_backward é esperado gerar alteraçãos válidas)
 *
 */
bool remove_insert(Rota * rota){
	//Criando um clone local(como backup!!)

	clone_rota(rota, &ROTA_CLONE_REMOVE_INSERT);
	if (ROTA_CLONE_REMOVE_INSERT->length < 4) return false;
	int positionSources[(ROTA_CLONE_REMOVE_INSERT->length-2)/2];
	//Procurando as posições dos sources
	int k = 0;
	for (int i = 1; i < ROTA_CLONE_REMOVE_INSERT->length-2; i++){
		if (ROTA_CLONE_REMOVE_INSERT->list[i].is_source)
			positionSources[k++] = i;
	}
	int position = positionSources[rand() % (ROTA_CLONE_REMOVE_INSERT->length-2)/2];
	Request * carona = ROTA_CLONE_REMOVE_INSERT->list[position].r;
	int offset = desfaz_insercao_carona_rota(ROTA_CLONE_REMOVE_INSERT, position);//Desfaz a inserção mas o carona continua marcado, para evitar recolocar.

	//Calculando o push backward máximo
	//double horaMaisCedo = calculate_service_time(&ROTA_CLONE_REMOVE_INSERT->list[position], &ROTA_CLONE_REMOVE_INSERT->list[position-1]);
	//double PB = ROTA_CLONE_REMOVE_INSERT->list[position].service_time - horaMaisCedo;
	//if (PB > 0){
	//}

	//O push backward pode falhar, por isso a preferência é tentar um aleatório.
	push_backward_mutation_op(ROTA_CLONE_REMOVE_INSERT, position);

	if (offset > 1 && position+offset < ROTA_CLONE_REMOVE_INSERT->length-1){
		//Calculando o push backward máximo
		//double horaMaisCedoOffset = calculate_service_time(&ROTA_CLONE_REMOVE_INSERT->list[position+offset], &ROTA_CLONE_REMOVE_INSERT->list[position+offset-1]);
		//double PBOffset = ROTA_CLONE_REMOVE_INSERT->list[position+offset].service_time - horaMaisCedoOffset;
		//if (PBOffset > 0){
			//push_backward_soft(ROTA_CLONE_REMOVE_INSERT, position+offset, PBOffset);
		//}

		push_backward_mutation_op(ROTA_CLONE_REMOVE_INSERT, position+offset);
	}

	bool ok = insere_carona_aleatoria_rota(ROTA_CLONE_REMOVE_INSERT, false);
	if (is_rota_valida(ROTA_CLONE_REMOVE_INSERT) && ok){
		clone_rota(ROTA_CLONE_REMOVE_INSERT, &rota);
		carona->matched = false;
		return true;
	}
	else{
		carona->matched = true;//POR QUE NUNCA ENTRA AQUI???????????
	}
	return false;
}

/*Escolhe um ponto aleatório e então troca o service de posição com o próximo */
bool swap_rider(Rota * rota){
	if (rota->length < 6) return false;
	clone_rota(rota, &ROTA_CLONE_SWAP);
	int ponto_swap = get_random_int(1, ROTA_CLONE_SWAP->length-3);
	Service service_temp = ROTA_CLONE_SWAP->list[ponto_swap];
	ROTA_CLONE_SWAP->list[ponto_swap] = ROTA_CLONE_SWAP->list[ponto_swap+1];
	ROTA_CLONE_SWAP->list[ponto_swap+1] = service_temp;

	Service *ant = &ROTA_CLONE_SWAP->list[ponto_swap-1];
	Service *atual = &ROTA_CLONE_SWAP->list[ponto_swap];
	Service *next = &ROTA_CLONE_SWAP->list[ponto_swap+1];

	if (atual->r == next->r)
		return false;

	atual->service_time = calculate_service_time(atual, ant);
	double nextTime = calculate_service_time(next, atual);

	double PF = nextTime - next->service_time;

	/*bool ordemValida = is_ordem_respeitada(ROTA_CLONE_SWAP);
	if (!ordemValida) {
		printf("muito improvável\n");
		return false;
	}*/

	push_forward_hard(ROTA_CLONE_SWAP, ponto_swap+1, PF);

	if(is_rota_valida(ROTA_CLONE_SWAP)){
		clone_rota(ROTA_CLONE_SWAP, &rota);
		return true;
	}
	return false;

}

/**
 * Repara o indivíduo, retirando todas as caronas repetidas.
 * No fim, as caronas com match são registradas no grafo
 */
void repair(Individuo *offspring, Graph *g){
	clean_riders_matches(g);
	for (int i = 0; i < offspring->size; i++){//Pra cada rota do idv
		Rota *rota = &offspring->cromossomo[i];

		//pra cada um dos services SOURCES na rota
		for (int j = 1; j < rota->length-1; j++){
			//Se é matched então algum SOURCE anterior já usou esse request
			//Então deve desfazer a rota de j até o offset
			if (rota->list[j].is_source && rota->list[j].r->matched){
				desfaz_insercao_carona_rota(rota, j);//Diminui length em duas unidades
				j--;
			}
			else if (rota->list[j].is_source){//Somente "senão", pois o tamanho poderia ter diminuido aí em cima.
				rota->list[j].r->matched = true;
				rota->list[j].r->id_rota_match = rota->id;
			}
		}
	}
}

void mutation(Individuo *ind, Graph *g, double mutationProbability){
	repair(ind, g);
	//shuffle(index_array_drivers_mutation, g->drivers);

	for (int r = 0; r < ind->size; r++){
		double accept = (double)rand() / RAND_MAX;
		if (accept < mutationProbability){
			//int k = index_array_drivers_mutation[r];
			Rota * rota  = &ind->cromossomo[r];

			int op = rand() % 3;
			switch(op){
				case (0):{
					remove_insert(rota);
					break;
				}
				case (1):{
					transfer_rider(rota,ind, g);
					break;
				}
				case (2):{
					swap_rider(rota);
					break;
				}
				/*case (3):{
					push_backward_mutation_op(rota,-1);
					break;
				}
				case (4):{
					push_forward_mutation_op(rota);
					break;
				}*/
			}
		}
	}
}



void crossover(Individuo * parent1, Individuo *parent2, Individuo *offspring1, Individuo *offspring2, Graph *g, double crossoverProbability){
	int rotaSize = g->drivers;
	offspring1->size = rotaSize;
	offspring2->size = rotaSize;

	int crossoverPoint = get_random_int(1, rotaSize-2);
	double accept = (double)rand() / RAND_MAX;

	if (accept < crossoverProbability){

		copy_rota(parent2, offspring1, 0, crossoverPoint);
		copy_rota(parent1, offspring1, crossoverPoint, rotaSize);
		copy_rota(parent1, offspring2, 0, crossoverPoint);
		copy_rota(parent2, offspring2, crossoverPoint, rotaSize);

		repair(offspring1, g);
		insere_carona_aleatoria_individuo(offspring1, false);
		repair(offspring2, g);
		insere_carona_aleatoria_individuo(offspring2, false);
	}
	else{
		copy_rota(parent1, offspring1, 0, rotaSize);
		copy_rota(parent2, offspring2, 0, rotaSize);
	}
}


/*Gera uma população de filhos, usando seleção, crossover e mutação*/
void crossover_and_mutation(Population *parents, Population *offspring,  Graph *g, double crossoverProbability, double mutationProbability){
	offspring->size = 0;//Tamanho = 0, mas considera todos já alocados
	int i = 0;
	while (offspring->size < parents->size){
		Individuo *parent1 = tournamentSelection(parents);
		Individuo *parent2 = tournamentSelection(parents);

		Individuo *offspring1 = offspring->list[i++];
		Individuo *offspring2 = offspring->list[i];

		crossover(parent1, parent2, offspring1, offspring2, g, crossoverProbability);

		mutation(offspring1, g, mutationProbability);
		mutation(offspring2, g, mutationProbability);
		offspring->size += 2;
	}
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


/*Pra poder usar a função qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compare0(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
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
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
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
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
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
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->objetivos[3] == y->objetivos[3])
        ret = 0;
    else if (x->objetivos[3] < y->objetivos[3])
        ret = -1;
    else
        ret = 1;
    return ret;
}

/*Adiciona o indivíduo de rank k no front k de FRONTS
 * Atualiza o size de FRONTS caso o rank seja maior*/
void add_Individuo_front(Fronts * fronts, Individuo *p){
	Population *fronti = fronts->list[p->rank];
	if (fronts->size < p->rank + 1){
	  fronti->size = 0;
	  fronts->size++;
	}
	
	fronti->list[fronti->size] = p;
	fronti->size++;
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
				add_dominated(a, b);
			}
			else if (dominates(b,a)){
				a->dominated_by_count++;
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
				if (indv_dominated->dominated_by_count > 0){
					indv_dominated->dominated_by_count--;
				}
				if (indv_dominated->dominated_by_count == 0){
					indv_dominated->rank = index_front+1;
					add_Individuo_front(fronts, indv_dominated);
					indv_dominated->dominated_by_count = -1;//Evita que o mesmo idv seja re-add em outras iteracoes
					//count_reps++;
				}
			}
		}
		index_front++;
	}
}


/*Deve ser chamado depois de determinar as funções objetivo*/
void crowding_distance_assignment(Population *pop){
	for (int i = 0; i < pop->size; i++){
		pop->list[i]->crowding_distance = 0;
	}
	for (int k = 0; k < QTD_OBJECTIVES; k++){

		sort_by_objective(pop, k);

		pop->list[0]->crowding_distance = FLT_MAX;
		pop->list[pop->size -1]->crowding_distance = FLT_MAX;

		double obj_min = pop->list[0]->objetivos[k];//valor min do obj k
		double obj_max = pop->list[pop->size -1]->objetivos[k];//valor max do obj k

		double diff = fmax(0.0001, obj_max - obj_min);

		for (int z = 1; z < pop->size -1; z++){
			double prox_obj = pop->list[z+1]->objetivos[k];
			double ant_obj = pop->list[z-1]->objetivos[k];

			if (pop->list[z]->crowding_distance != FLT_MAX)
				pop->list[z]->crowding_distance += (prox_obj - ant_obj) / diff;
		}

	}
}

/*Pra poder usar a função qsort com N objetivos,
 * precisamos implementar os n algoritmos de compare*/
int compareByCrowdingDistanceMax(const void *p, const void *q) {
    int ret;
    Individuo * x = *(Individuo **)p;
    Individuo * y = *(Individuo **)q;
    if (x->crowding_distance == y->crowding_distance)
        ret = 0;
    else if (x->crowding_distance > y->crowding_distance)
        ret = -1;
    else
        ret = 1;
    return ret;
}

bool crowded_comparison_operator(Individuo *a, Individuo *b){
	return (a->rank < b->rank || (a->rank == b->rank && a->crowding_distance > b->crowding_distance));
}

void sort_by_crowding_distance_assignment(Population *front){
	qsort(front->list, front->size, sizeof(Individuo*), compareByCrowdingDistanceMax );
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
			offsprings->list[offsprings->size] = frontsList->list[lastPosition]->list[k];
			offsprings->size++;
		}
		lastPosition++;
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

















