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

union nfo{
	float time;
	float distance;
};

typedef struct vtx{
	//Common
	int id;
	float time_window[4];
	vtx* destiny;
	float ED;//Earliest Departure time 0 - 24 em horas Ex : 2,5 = 2:30h
	float LA;//Latest Arrival time 0 - 24 em horas Ex : 2,5 = 2:30h
	float AT;//variável de tempo A
	float BT;//Variável de tempo B

	//Rider=================================
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


class Grafo{
public:
	nfo** matrix;
	vertex* vertex_list;
	int n;
	//Constroi um grafo completo com n vértices
	Grafo(int n_) : n(n_) {
		matrix = (nfo**) malloc(sizeof(nfo*)*n);
		for(int i=0; i< n; i++){
			matrix[i] = (nfo*) malloc (sizeof(nfo)*n);
		}

		vertex_list = (vertex*) malloc(sizeof(vertex)*n);

		for (int i = 0; i < n; i++){
			vertex_list[i].id = i;
			vertex_list[i].AT = 0;
			vertex_list[i].BT = 1.3;
			vertex_list[i].AD = 0;
			vertex_list[i].BD = 1.3;
			vertex_list[i].s = Service_time;//tempo de serviço é zero para todos os riders
			vertex_list[i].demand = 1;//demanda é 1 para todos os riders
			vertex_list[i].hasRider = false;
			vertex_list[i].hasVehicle = false;
			vertex_list[i].demand = 0;
		}
	}

	~Grafo(){
		free(vertex_list);
		for (int i = 0; i < n; i++){
			free(matrix[i]);
		}
		free(matrix);
	}
};


/**==================== NSGA-II ==============================================**/

typedef struct ind{
	float vehicle_trip_total_time;
	float vehicle_trip_total_distance;
	float riders_trip_total_time;
	float riders_trip_total_distance;
	float riders_unmatched;

	int* gene;
	int n;//Número de soluções que dominam ind
	struct ind* S;//Conjunto de soluções dominadas por ind
	int Sn;//número de soluções dominadas por ind
}individual;


typedef struct popl{

	individual* list;

}Population;



#endif /* RIDESHARINGNSGA_II_H_ */
