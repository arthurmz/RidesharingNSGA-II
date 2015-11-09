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
	float AT;//vari�vel de tempo A
	float BT;//Vari�vel de tempo B

	//Rider=================================
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


class Grafo{
public:
	nfo** matrix;
	vertex* vertex_list;
	int size;
	//Constroi um grafo completo com n v�rtices
	Grafo(int n_) : size(n_) {
		matrix = (nfo**) malloc(sizeof(nfo*)*size);
		for(int i=0; i< size; i++){
			matrix[i] = (nfo*) malloc (sizeof(nfo)*size);
		}

		vertex_list = (vertex*) malloc(sizeof(vertex)*size);

		for (int i = 0; i < size; i++){
			vertex_list[i].id = i;
			vertex_list[i].AT = 0;
			vertex_list[i].BT = 1.3;
			vertex_list[i].AD = 0;
			vertex_list[i].BD = 1.3;
			vertex_list[i].s = Service_time;//tempo de servi�o � zero para todos os riders
			vertex_list[i].demand = 1;//demanda � 1 para todos os riders
			vertex_list[i].hasRider = false;
			vertex_list[i].hasVehicle = false;
			vertex_list[i].demand = 0;
		}
	}

	~Grafo(){
		free(vertex_list);
		for (int i = 0; i < size; i++){
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


	/*O cromosomo � uma lista de rotas, onde cada rota � uma lista de v�rtices
	 * A primeira dimens�o ter� tamanho N igual ao n�mero de ve�culos dispon�veis
	 * A segunda dimens�o ter� tamanho igual ao n�mero total de v�rtices do grafo
	 * Todos as solu��es ter�o N rotas, mas cada rota pode ter uma qtd vari�vel de v�rtices
	 * O fim da lista � encontrado ao verificar que o v�rtice lido � o destino*/
	vertex*** cromossomo;
	int n;//N�mero de solu��es que dominam ind
	struct ind* S;//Conjunto de solu��es dominadas por ind
	int Sn;//n�mero de solu��es dominadas por ind
}individual;


typedef struct popl{

	individual* list;

}Population;



#endif /* RIDESHARINGNSGA_II_H_ */
