/*
 * ridesharing.cpp
 *
 *  Created on: 5 de nov de 2015
 *      Author: Arthur
 */

#include <vector>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include "RidesharingNSGA-II.h"
using namespace std;

/**===================Global variables=========================================**/
int riders_requests;
int vehicles_offers;
Grafo* graph;
Population* p;


Population* malloc_population(int pop_size){
	Population* p1 = (Population*) malloc(sizeof(Population));

	p1->list = (individual*) malloc(sizeof(individual)*pop_size);

	for (int i = 0; i < pop_size; i++){
		p1->list[i].n = 0;
		p1->list[i].S = (individual*) malloc(sizeof(individual)*pop_size);
		p1->list[i].Sn = 0;
		p1->list[i].cromossomo = (vertex***) malloc(sizeof(vertex**)*vehicles_offers);
		for (int j = 0; j< graph->size; j++){
			p1->list[j].cromossomo[j] = (vertex**) malloc(sizeof(vertex*)*graph->size);
		}
	}
	return p1;
}

/*Configura o grafo do problema, com as distancias entre cada cidade,
 * caronas e veículos disponíveis*/
void config_problema_minimo(){
	Grafo* g = new Grafo(8);
	graph = g;

	riders_requests = 2;
	vehicles_offers = 2;


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
}



void config_nsga_ii(){
	p = malloc_population(20);

}

void cleanup(){
	delete graph;
}

int main(){
	srand (time(NULL));

	config_problema_minimo();

	config_nsga_ii();

	graph->matrix[2][0].time = 21;

	cout << graph->matrix[2][0].time << endl;

	cout << graph->vertex_list[1].id;


	cleanup();
	return 0;
}
