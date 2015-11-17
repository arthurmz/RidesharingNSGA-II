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
	float AT;//vari�vel de tempo A
	float BT;//Vari�vel de tempo B

	//Rider=================================
	bool matched;//se j� foi feito o match desse carona
	bool hasRider;
	int demand;//dmi
	float MTT;//rider's maximum travel time em horas
	float s;//Service time si (tempo que gasta atendendo um rider em horas)
	//Vehicle===============================
	bool hasVehicle;
	int Capacity;//Capacity
	float AD;//vari�vel de dist�ncia A
	float BD;//Vari�vel de dist�ncia B
	float MTD;//maximum vehicle's travel distance em metros

} vertex;


typedef struct Grafo{
	Info** matrix;
	vertex* vertex_list;
	vertex** vehicle_origins;//lista explicita com os v�rtices onde saem motoristas
	vertex** rider_origins;//lista explicita com os v�rtices onde saem os caronas
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
		g->vertex_list[i].s = Service_time;//tempo de servi�o � zero para todos os riders
		g->vertex_list[i].demand = 1;//demanda � 1 para todos os riders
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


	/*O cromosomo � uma lista de rotas, onde cada rota � uma lista de v�rtices
	 * A primeira dimens�o ter� tamanho N igual ao n�mero de ve�culos dispon�veis
	 * A segunda dimens�o ter� tamanho igual ao n�mero total de v�rtices do grafo
	 * Todos as solu��es ter�o N rotas, mas cada rota pode ter uma qtd vari�vel de v�rtices
	 * O fim da lista � encontrado ao verificar que o v�rtice lido � o destino*/
	vertex*** cromossomo;
	int n;//N�mero de solu��es que dominam ind
	struct Individual* S;//Conjunto de solu��es dominadas por ind
	int Sn;//n�mero de solu��es dominadas por ind
}Individual;


typedef struct Population{
	Individual* list;
	int size;
}Population;



#endif /* RIDESHARINGNSGA_II_H_ */
