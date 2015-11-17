/*
 * ridesharing.cpp
 *
 *  Created on: 5 de nov de 2015
 *      Author: Arthur
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include "RidesharingNSGA-II.h"

/**===================Global variables=========================================**/
int RIDERS_REQUESTS;
int VEHICLES_OFFERS;
Grafo* graph;
Population* population;
float Pcrossover;
float Pmutation;


/*Recebe uma rota que é parte do cromossomo (uma linha do cromossomo)
 * e adiciona o carona na penultima posição, antes de chegar no destino
 * TODO, tá faltando todas as validações de janela de tempo
 *
 * position informa qual a posição para inserir o carona, à partir do vértice de origem do veículo.
 * Ex.: position = 0: primeira carona depois de sair do source
 * position = 1: segunda carona.. e assim por diante*/
bool add_rider_vehicle(vertex** rota_cromossomo, vertex* rider, int insertionPosition){
	int i = 0;
	while (rota_cromossomo[i] != NULL && i < graph->size){
		if (rota_cromossomo[i] == rider) return false;
		i++;
	}
	//chegando aqui, i-1 == ultima posicao
	int lastPosition = i-1;
	vertex* destiny = rota_cromossomo[lastPosition];
	rota_cromossomo[lastPosition] = rider;
	rota_cromossomo[lastPosition+1] = rider->destiny;
	rota_cromossomo[lastPosition+2] = destiny;
	rider->matched = true;
	return true;
}

/*Aloca na memória uma população de tamanho pop_size*/
Population* initialize_population(int pop_size){
	Population* p1 = (struct Population *) malloc(sizeof(struct Population));

	p1->list = (Individual*) malloc(sizeof(Individual));
	p1->size = pop_size;

	for (int i = 0; i < pop_size; i++){
		p1->list[i].n = 0;
		p1->list[i].S = (Individual *) calloc(pop_size, sizeof(Individual));
		p1->list[i].Sn = 0;
		p1->list[i].riders_trip_total_distance = 0;
		p1->list[i].riders_trip_total_time = 0;
		p1->list[i].riders_unmatched = 0;
		p1->list[i].rotas = 0;
		p1->list[i].vehicle_trip_total_distance = 0;
		p1->list[i].vehicle_trip_total_time = 0;
		p1->list[i].cromossomo = (vertex***) calloc(VEHICLES_OFFERS, sizeof(vertex**));
		for (int j = 0; j< VEHICLES_OFFERS; j++){
			p1->list[i].cromossomo[j] = (vertex**) calloc(graph->size, sizeof(vertex*));
		}
	}

	for (int i = 0; i < pop_size; i++){
		Individual* indv = &p1->list[i];

		for (int k = 0; k < VEHICLES_OFFERS; k++){
			//Inicializa as K rotas do cromossomo atual com as origens e destinos de todas as ofertas de veículos
			indv->cromossomo[k][0] = graph->vehicle_origins[k];
			indv->cromossomo[k][1] = graph->vehicle_origins[k]->destiny;

			//Escolhe aleatoriamente 0 ou 1 carona da lista de caronas e adiciona à rota atual do cromossomo
			int coin = rand() % RIDERS_REQUESTS + 1; //SE DER EXATAMENTE RIDERS_REQUESTS, CONSIDERA COMO 'não dar carona'
			if (coin < RIDERS_REQUESTS && !graph->rider_origins[coin]->matched){
				add_rider_vehicle(indv->cromossomo[k], graph->rider_origins[coin], 0);
			}
		}
		clean_matches_graph(graph);

	}
	return p1;
}



/*Configura o grafo do problema, com as distancias entre cada cidade,
 * caronas e veículos disponíveis*/
void config_problema_minimo(){
	Grafo* g = new_grafo(8);
	graph = g;

	RIDERS_REQUESTS = 2;
	VEHICLES_OFFERS = 2;


	/*Configurando as distancias.*/
	g->matrix[0][0].distance = 0;
	g->matrix[0][0].time = g->matrix[0][0].distance / MPs;
	g->matrix[0][1].distance = 1200;
	g->matrix[0][1].time = g->matrix[0][1].distance / MPs;
	g->matrix[0][2].distance = 7120;
	g->matrix[0][2].time = g->matrix[0][2].distance / MPs;
	g->matrix[0][3].distance = 3000;
	g->matrix[0][3].time = g->matrix[0][3].distance / MPs;
	g->matrix[0][4].distance = 4500;
	g->matrix[0][4].time = g->matrix[0][4].distance / MPs;
	g->matrix[0][5].distance = 3000;
	g->matrix[0][5].time = g->matrix[0][5].distance / MPs;

	g->matrix[1][0].distance = 1200 / MPs;
	g->matrix[1][0].time = g->matrix[1][0].distance / MPs;
	g->matrix[1][1].distance = 0;
	g->matrix[1][1].time = g->matrix[1][1].distance / MPs;
	g->matrix[1][2].distance = 5000;
	g->matrix[1][2].time = g->matrix[1][2].distance / MPs;
	g->matrix[1][3].distance = 8000;
	g->matrix[1][3].time = g->matrix[1][3].distance / MPs;
	g->matrix[1][4].distance = 4700;
	g->matrix[1][4].time = g->matrix[1][4].distance / MPs;
	g->matrix[1][5].distance = 5850;
	g->matrix[1][5].time = g->matrix[1][5].distance / MPs;

	g->matrix[2][0].distance = 7120;
	g->matrix[2][0].time = g->matrix[2][0].distance / MPs;
	g->matrix[2][1].distance = 5000;
	g->matrix[2][1].time = g->matrix[2][1].distance / MPs;
	g->matrix[2][2].distance = 0;
	g->matrix[2][2].time = g->matrix[2][2].distance / MPs;
	g->matrix[2][3].distance = 3460;
	g->matrix[2][3].time = g->matrix[2][3].distance / MPs;
	g->matrix[2][4].distance = 9000;
	g->matrix[2][4].time = g->matrix[2][4].distance / MPs;
	g->matrix[2][5].distance = 5000;
	g->matrix[2][5].time = g->matrix[2][5].distance / MPs;

	g->matrix[3][0].distance = 3000;
	g->matrix[3][0].time = g->matrix[3][0].distance / MPs;
	g->matrix[3][1].distance = 8000;
	g->matrix[3][1].time = g->matrix[3][1].distance / MPs;
	g->matrix[3][2].distance = 3460;
	g->matrix[3][2].time = g->matrix[3][2].distance / MPs;
	g->matrix[3][3].distance = 0;
	g->matrix[3][3].time = g->matrix[3][3].distance / MPs;
	g->matrix[3][4].distance = 6000;
	g->matrix[3][4].time = g->matrix[3][4].distance / MPs;
	g->matrix[3][5].distance = 1020;
	g->matrix[3][5].time = g->matrix[3][5].distance / MPs;

	g->matrix[4][0].distance = 4500;
	g->matrix[4][0].time = g->matrix[4][0].distance / MPs;
	g->matrix[4][1].distance = 4700;
	g->matrix[4][1].time = g->matrix[4][1].distance / MPs;
	g->matrix[4][2].distance = 9000;
	g->matrix[4][2].time = g->matrix[4][2].distance / MPs;
	g->matrix[4][3].distance = 6000;
	g->matrix[4][3].time = g->matrix[4][3].distance / MPs;
	g->matrix[4][4].distance = 0;
	g->matrix[4][4].time = g->matrix[4][4].distance / MPs;
	g->matrix[4][5].distance = 4000;
	g->matrix[4][5].time = g->matrix[4][5].distance / MPs;

	g->matrix[5][0].distance = 3000;
	g->matrix[5][0].time = g->matrix[5][0].distance / MPs;
	g->matrix[5][1].distance = 5850;
	g->matrix[5][1].time = g->matrix[5][1].distance / MPs;
	g->matrix[5][2].distance = 5000;
	g->matrix[5][2].time = g->matrix[5][2].distance / MPs;
	g->matrix[5][3].distance = 1020;
	g->matrix[5][3].time = g->matrix[5][3].distance / MPs;
	g->matrix[5][4].distance = 4000;
	g->matrix[5][4].distance = g->matrix[5][4].distance / MPs;
	g->matrix[5][5].distance = 0;
	g->matrix[5][5].time = g->matrix[5][5].distance / MPs;

	/*Configurando o motorista 1*/
	vertex* mt1 = &g->vertex_list[0];
	mt1->ED = 8;
	mt1->LA = 15.5;
	mt1->MTT = mt1->AT + (mt1->BT * g->matrix[0][6].time);
	mt1->MTD = mt1->AD + (mt1->BD * g->matrix[0][6].distance);
	mt1->destiny = &g->vertex_list[6];
	mt1->hasVehicle = true;
	mt1->time_window[0] = mt1->ED;
	mt1->time_window[1] = mt1->LA - mt1->MTT;
	mt1->time_window[2] = mt1->ED + mt1->MTT;
	mt1->time_window[3] = mt1->LA;
	mt1->Capacity = 20;
	g->vehicle_origins[0] = mt1;

	/*Configurando o motorista 2*/
	vertex* mt2 = &g->vertex_list[2];
	mt2->ED = 7;
	mt2->LA = 16.5;
	mt2->MTT = mt2->AT + (mt2->BT * g->matrix[2][4].time);
	mt2->MTD = mt2->AD + (mt2->BD * g->matrix[2][4].distance);
	mt2->destiny = &g->vertex_list[4];
	mt2->hasVehicle = true;
	mt2->time_window[0] = mt2->ED;
	mt2->time_window[1] = mt2->LA - mt1->MTT;
	mt2->time_window[2] = mt2->ED + mt1->MTT;
	mt2->time_window[3] = mt2->LA;
	mt2->Capacity = 20;
	g->vehicle_origins[1] = mt2;

	/*Configurando o carona 1*/
	vertex* rider1 = &g->vertex_list[7];
	rider1->ED = 11.5;
	rider1->LA = 15;
	rider1->MTT = rider1->AT + (rider1->BT * g->matrix[7][3].time);
	rider1->destiny = &g->vertex_list[3];
	rider1->hasRider = true;
	rider1->demand = 3;
	rider1->time_window[0] = rider1->ED;
	rider1->time_window[1] = rider1->LA - mt1->MTT;
	rider1->time_window[2] = rider1->ED + mt1->MTT;
	rider1->time_window[3] = rider1->LA;
	g->rider_origins[0] = rider1;


	/*Configurando o carona 2*/
	vertex* rider2 = &g->vertex_list[1];
	rider2->ED = 9;
	rider2->LA = 13;
	rider2->MTT = rider2->AT + (rider2->BT * g->matrix[1][5].time);
	rider2->destiny = &g->vertex_list[5];
	rider2->hasRider = true;
	rider2->demand = 7;
	rider2->time_window[0] = rider2->ED;
	rider2->time_window[1] = rider2->LA - mt1->MTT;
	rider2->time_window[2] = rider2->ED + mt1->MTT;
	rider2->time_window[3] = rider2->LA;
	g->rider_origins[1] = rider2;
}




/*Avalia a aptidão dos elementos da população em relação
 * a cada um dos objetivos*/
void evaluate_against_objective_functions(Population * pop){
	float vttdmax = 0;//guarda a maior distancia total de uma solução
	float vttdmin = FLT_MAX;
	float vtttmax = 0;
	float vtttmin = FLT_MAX;

	for (int i = 0; i < pop->size; i++){//para cada indivíduo da população
		Individual* indv = &pop->list[i];
		for (int r = 0; r < VEHICLES_OFFERS; r++){//para cada rota
			vertex** rota = indv->cromossomo[r];
			vertex* source = rota[0];
			//vertex* destiny = source->destiny;
			int v = 1;
			while (rota[v] != NULL){
				indv->vehicle_trip_total_distance += graph->matrix[source->id][rota[v]->id].distance;
				indv->vehicle_trip_total_time += graph->matrix[source->id][rota[v]->id].time;
				v++;
			}
			if (indv->vehicle_trip_total_distance > vttdmax)
				vttdmax = indv->vehicle_trip_total_distance;
			if (indv->vehicle_trip_total_distance < vttdmin)
				vttdmin = indv->vehicle_trip_total_distance;

			if (indv->vehicle_trip_total_time > vtttmax)
				vtttmax = indv->vehicle_trip_total_time;
			if (indv->vehicle_trip_total_time < vtttmin)
				vtttmin = indv->vehicle_trip_total_time;
		}
	}

	/*
	 * Normalizando Os valores das distancias usando a fórmula (0 é máximo e 1 é mínimo)
	 *   Zmax - Zi
	 * ------------
	 *  Zmax - Zmin
	 *
	 * */
	for (int i = 0; i < pop->size; i++){
		Individual* indv = &pop->list[i];
		indv->vehicle_trip_total_distance = (vttdmax - indv->vehicle_trip_total_distance) / vttdmax-vttdmin;
	}

}


/**
 * Questões levantadas:
 *
 * Se a variável binária Xkij Ã© 1 apenas em um caso onde atende
 *  o ultimo delivery e vai direto pro destino, nÃ£o existe variação de solução
 *  quanto Ã s cidades entre o ultimo delivery e o destino do veiculo??
 *  Nesse caso eu não tenho que me preocupar em minimizar essa distancia final percorrida
 *  entre i+v,k+v???
 *
 *  Exato: na matriz, existe uma distância e um tempo conhecidos pra ir de k pra k+v
 *  assim, nÃ£o existe possibilidade de eu tentar fazer um caminho menor, passando por outras cidades.
 *  já que nenhuma outra cidade FAZ PARTE DO CAMINHO DE K PRA K+V!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *  a menor distancia entre k e k+v será a que tá na matriz
 *  adicionando um detour, a distancia fica k -> i -> i+n -> k+v
 */
int main(){

	Pcrossover = 0.75;
	Pmutation = 0.01;

	srand (time(NULL));

	config_problema_minimo();

	population = initialize_population(2);

	evaluate_against_objective_functions(population);

	graph->matrix[2][0].time = 21;

	printf("%d\n", graph->matrix[2][0].time);
	printf("%d\n", graph->vertex_list[1].id);


	clean_graph(graph);
	return 0;
}
