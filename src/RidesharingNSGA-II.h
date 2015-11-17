/*
 * RidesharingNSGA-II.h
 *
 *  Created on: 9 de nov de 2015
 *      Author: Arthur
 */

#ifndef RIDESHARINGNSGA_II_H_
#define RIDESHARINGNSGA_II_H_

//Static===================================================================
const float MPs = 16;//16 metros por segundo
const float Service_time = 0;//Tempo gasto para servir um Rider

typedef struct Info{
	float time;
	float distance;
}Info;

typedef struct vtx{
	//Common
	int id;
	float time_window[4];
	struct vtx* destiny;
	float ED;//Earliest Departure time 0 - 24 em horas Ex : 2,5 = 2:30h
	float LA;//Latest Arrival time 0 - 24 em horas Ex : 2,5 = 2:30h
	float AT;//variável de tempo A
	float BT;//Variável de tempo B

	//Rider=================================
	bool matched;//se já foi feito o match desse carona
	bool hasRider;
	int demand;//dmi
	float MTT;//rider's maximum travel time em horas
	float s;//Service time si (tempo que gasta atendendo um rider em horas)
	//Vehicle===============================
	bool hasVehicle;
	int Capacity;//Capacity
	float AD;//variável de distância A
	float BD;//Variável de distância B
	float MTD;//maximum vehicle's travel distance em metros

} vertex;


typedef struct Grafo{
	Info** matrix;
	vertex* vertex_list;
	vertex** vehicle_origins;//lista explicita com os vértices onde saem motoristas
	vertex** rider_origins;//lista explicita com os vértices onde saem os caronas
	int size;

}Grafo;

Grafo * new_grafo(int size){

	Grafo * g = (Grafo *) malloc(sizeof(Grafo));

	g->size = size;

	g->matrix = (Info**) calloc(size, sizeof(Info*));
	for(int i=0; i< size; i++){
		g->matrix[i] = (Info*) calloc (size, sizeof(Info));
	}

	g->vertex_list = (vertex*) calloc(size, sizeof(vertex));
	g->vehicle_origins = (vertex**) calloc(size, sizeof(vertex*));
	g->rider_origins = (vertex**) calloc(size, sizeof(vertex*));

	for (int i = 0; i < size; i++){
		g->vertex_list[i].id = i;
		g->vertex_list[i].AT = 0;
		g->vertex_list[i].BT = 1.3;
		g->vertex_list[i].AD = 0;
		g->vertex_list[i].BD = 1.3;
		g->vertex_list[i].s = Service_time;//tempo de serviço é zero para todos os riders
		g->vertex_list[i].demand = 1;//demanda é 1 para todos os riders
		g->vertex_list[i].hasRider = false;
		g->vertex_list[i].hasVehicle = false;
		g->vertex_list[i].demand = 0;
	}

	return g;
}

/*Desfaz os matchs registrados na lista de caronas*/
void clean_matches_graph(Grafo *g){
	int i = 0;
	while (g->rider_origins[i] != NULL){
		g->rider_origins[i]->matched = false;
		i++;
	}
}

void clean_graph(Grafo * g){
	if (g != NULL) {
		if (g->vertex_list != NULL)
			free(g->vertex_list);
		if (g->matrix != NULL) {
			for(int i=0; i< g->size; i++){
				if (g->matrix[i] != NULL)
					free(g->matrix[i]);
			}
			free(g->matrix);
		}
		free(g);
	}
}


/**==================== NSGA-II ==============================================**/


typedef struct Individual{
	float vehicle_trip_total_time;
	float vehicle_trip_total_distance;
	float riders_trip_total_time;
	float riders_trip_total_distance;
	float riders_unmatched;

	int rotas;//??????????


	/*O cromosomo é uma lista de rotas, onde cada rota é uma lista de vértices
	 * A primeira dimensão terá tamanho N igual ao número de veículos disponíveis
	 * A segunda dimensão terá tamanho igual ao número total de vértices do grafo
	 * Todos as soluções terão N rotas, mas cada rota pode ter uma qtd variável de vértices
	 * O fim da lista é encontrado ao verificar que o vértice lido é o destino*/
	vertex*** cromossomo;
	int n;//Número de soluções que dominam ind
	struct Individual* S;//Conjunto de soluções dominadas por ind
	int Sn;//número de soluções dominadas por ind
}Individual;


typedef struct Population{
	Individual* list;
	int size;
}Population;



#endif /* RIDESHARINGNSGA_II_H_ */
