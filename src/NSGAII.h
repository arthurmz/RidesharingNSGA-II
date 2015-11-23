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
	double request_arrival_time;
	double service_time_at_source;
	double pickup_location_longitude;
	double pickup_location_latitude;
	double pickup_earliest_time;
	double pickup_latest_time;
	double service_time_at_delivery;
	double delivery_location_long;
	double delivery_location_latitude;
	double delivery_earliest_time;
	double delivery_latest_time;
}Request;

/*Gene of a solution*/
typedef struct Service{
	Request *r;
	bool source;//1-está saindo da origem, 0-está chegando no destino
}Service;

typedef struct Rota{
	Service *list;
	int length;
}Rota;

typedef struct Individuo{
	//Cromossomo é uma lista de rotas
	//Onde cada rota é uma lista de Services
	//Cada Service é uma coleta ou entrega(tanto de motorista como de carona)
	Rota * cromossomo;
	float objetivos[5];

	int dominated_by_count;//Número de soluções que dominam ind
	struct Individual **dominates_list;//Conjunto de soluções dominadas por this
	int dominates_list_count;//número de soluções dominadas por ind
	int rank;//Qual front este indivíduo está
	float crowding_distance;
}Individuo;

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


#endif /* NSGAII_H_ */
