/*
 * NSGAII.h
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#ifndef NSGAII_H_
#define NSGAII_H_

#include <stdbool.h>

/*Driver or Rider*/
typedef struct Request{
	bool driver;//true -driver, false -rider
	bool matched;//true se for um carona já combinado
	int req_no;
	double request_arrival_time;//A hora em que esse request foi descoberto (desconsiderado pq o problema é estático)
	double service_time_at_source;//Tempo gasto para atender o source (Diferente da HORA em que chega no source)
	double service_time_at_delivery;//Tempo gasto para atender o destino (Diferente da HORA em que chega no destino)
	double pickup_location_longitude;
	double pickup_location_latitude;
	double delivery_location_long;
	double delivery_location_latitude;
	double pickup_earliest_time;
	double pickup_latest_time;
	double delivery_earliest_time;
	double delivery_latest_time;
}Request;

/*Gene of a solution*/
typedef struct Service{
	Request *r;
	//Time aqui é referente à HORA em que CHEGA nos pontos do Rider
	//Ou a HORA em que SAI da origem do Driver.
	double time;
	//Tempo de espera nos pontos de source do rider
	//Atualizado ao inserir uma nova carona
	double waiting_time;
	bool is_source;//1-está saindo da origem, 0-está chegando no destino
}Service;

/*A rota guarda o serviço, podendo sobrescrever
 * sem se preocupar com a memória*/
typedef struct Rota{
	Service *list;
	int length;
}Rota;

typedef struct Individuo{
	//Cromossomo é uma lista de rotas
	//Onde cada rota é uma lista de Services
	//Cada Service é uma coleta ou entrega(tanto de motorista como de carona)
	Rota * cromossomo;
	int size;//tamanho da lista de rotas
	float objetivos[5];

	int dominated_by_count;//Número de soluções que dominam ind
	struct Individuo **dominates_list;//Conjunto de soluções dominadas por this
	int dominates_list_capacity;
	int dominates_list_count;//número de soluções dominadas por ind
	int rank;//Qual front este indivíduo está
	float crowding_distance;

	//Contador de Referências pra este indivíduo
	//Mais especificamente, conta a quantidade de referências desse indivíduo por OUTRO indivíduo
	int ref_count;
}Individuo;

/*Os indivíduos são armazenados no heap pra
 * facilitar em alguns aspectos a manipulação do dado.*/
typedef struct Population{
	//posição do front (caso essa população seja um front)
	int id_front;
	Individuo **list;
	int size;
	int max_capacity;
}Population;


typedef struct Fronts{
	int size;//quantidade de fronts (pode ser diferente da capacidade max)
	Population **list;//Cada população é um front
}Fronts;



/*====================Graph=====================================================*/
typedef struct Graph{
	Request *request_list;//Lista de requests
	int drivers;
	int riders;
	int total_requests;
}Graph;


Graph *new_graph(int drivers, int riders, int total_requests);
Population *generate_random_population(int size, Graph *g);
void add_Individuo_front(Fronts * fronts, Individuo *p);
bool dominates(Individuo *a, Individuo *b);
void add_dominated(Individuo *b, Individuo *a);
void fast_nondominated_sort(Population *population, Fronts * fronts);
void crowding_distance_assignment(Population *front_i);
bool crowded_comparison_operator(Individuo *a, Individuo *b);
bool is_rota_valida(Rota *rota);
bool insere_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset);
void desfaz_insercao_carona_rota(Rota *rota, Request *carona, int posicao_insercao, int offset);
double distancia_percorrida(Rota * rota);
void evaluate_objective_functions(Individuo *idv, Graph *g);
void free_population(Population *population);
Population * generate_offspring(Population *parents);
void empty_front_list(Fronts * f);
void sort_by_crowding_distance_assignment(Population *front);
Population * select_reduced_population(Fronts *frontsList, int p_size, Graph *g);
Individuo * new_individuo_by_individuo(Individuo *p, Graph * g);
void merge(Population *p1, Population *p2, Population *big_population);
void complete_free_individuo(Individuo * idv);

#endif /* NSGAII_H_ */
