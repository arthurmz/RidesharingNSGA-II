/*
 * helper.c
 *
 *  Created on: 21 de nov de 2015
 *      Author: arthur
 */

#include "Helper.h"
#include "StaticVariables.h"
#include <stdio.h>
#include <stdlib.h>


/*Aloca um novo Fronts na memória, de tamanho max_capacity
 * Cada elemento de list é um ponteiro pra uma população que é alocada
 * Cada população guarda uma lista de ponteiros pros Individuos
 * Os indivíduos não são alocados*/
Fronts* new_front_list(int max_capacity){
	Fronts* f = (Fronts*) calloc(1, sizeof(Fronts));
	f->size = 0;
	f->list = (Population**) calloc(max_capacity, sizeof(Population*));
	for (int i = 0; i < max_capacity; i++){
		f->list[i] = calloc(1, sizeof(Population));
		Population *fronti = f->list[i];
		fronti->id_front = 0;
		fronti->size = 0;
		fronti->max_capacity = max_capacity;
		fronti->list = calloc(max_capacity, sizeof(Individuo*));
	}
	return f;
}

/*
void empty_front_list(Fronts * f){
	int max = f->list[0]->max_capacity;
	for (int i = 0; i < max; i++){
		for (int j = 0; j < max; j++){
			f->list[i]->size = 0;
			f->list[i]->list[j] = NULL;
		}
	}
}*/

/*
void clean_front_list(Fronts * f){
	for (int i = 0; i < f->size; i++){
		Population *fronti = f->list[i];
		fronti->id_front = 0;
		fronti->size = 0;
		for (int j = 0; j < fronti->size; j++){
			fronti->list[j] = NULL;
		}
	}
	f->size = 0;
}*/

/*TESTADO OK*/
Individuo * new_individuo(int drivers_qtd, int riders_qtd){
	Individuo *ind = calloc(1, sizeof(Individuo));
	ind->cromossomo = (Rota*) calloc(drivers_qtd, sizeof(Rota));
	ind->size = drivers_qtd;

	for (int i = 0; i < drivers_qtd; i++){
		Service * result = calloc(VEHICLE_CAPACITY+2, sizeof(Service));
		ind->cromossomo[i].list = result;
		ind->cromossomo[i].length = 0;
	}
	return ind;
}

void complete_free_individuo(Individuo * idv){
	if (idv != NULL){
		for (int i = 0; i < idv->size; i++){
			free(idv->cromossomo[i].list);
		}
		free(idv->cromossomo);
		free(idv);
	}
}

/*Aloca uma nova população de tamanho max_capacity
 * Cada elemento de list é um ponteiro pra indivíduo NÃO ALOCADO*/
Population* new_empty_population(int max_capacity){
	Population *p = (Population*) calloc(1, sizeof(Population));
	p->max_capacity = max_capacity;
	p->size = 0;
	p->list = calloc(max_capacity, sizeof(Individuo*));
	return p;
}

void sort(Population *front, int k){
	int tam = front->size;
	double min;
	double aux;
	for (int i = 0; i < (tam-1); i++){
		min = i;
		for (int j = (i+1); j < tam; j++) {
			if(front->list[j]->objetivos[k] < front->list[min]->objetivos[k])
			min = j;
		}
		if (i != min) {
			aux = front->list[i];
			front->list[i] = front->list[min];
			front->list[min] = aux;
		}
	}

}

/*Distância em km*/
double haversine_helper(double lat1, double lon1, double lat2, double lon2){
	double R = 6372.8;
	double to_rad = 3.1415926536 / 180;
	double dLat = to_rad * (lat2 - lat1);
	double dLon = to_rad * (lon2 - lon1);

	lat1 = to_rad * lat1;
	lat2 = to_rad * lat2;

	double a = pow (sin(dLat/2),2) + pow(sin(dLon/2),2) * cos(lat1) * cos(lat2);
	double c = 2 * asin(sqrt(a));
	return R * c;
}

double haversine(Request *a, Request *b){
	return haversine_helper(a->pickup_location_latitude, a->pickup_location_longitude,
			b->delivery_location_latitude, b->delivery_location_long);
}

/*Tempo em minutos*/
double time_between_requests(Request *a, Request *b){
	double distance = haversine(a, b);
	return distance / VEHICLE_SPEED * 60;
}

Graph * parse_file(char *filename){
	FILE *fp=fopen(filename, "r");

	if (fp == NULL){
		printf("Não foi possível abrir o arquivo %s\n", filename);
		return NULL;
	}
	else{
		printf("Arquivo aberto corretamente! %s\n", filename);
	}

	int total_requests;
	int drivers;
	int riders;

	char linha_temp[1000];
	fgets(linha_temp, 1000, fp);
	sscanf(linha_temp, "%i", &total_requests);
	fgets(linha_temp, 1000, fp);
	sscanf(linha_temp, "%i", &drivers);
	fgets(linha_temp, 1000, fp);
	sscanf(linha_temp, "%i", &riders);
	Graph * g = new_graph(drivers, riders, total_requests);

	int index_request = 0;

	while (!feof(fp) && index_request < total_requests){
		char linha[10000];
		fgets(linha, 10000, fp);

		Request *rq = &g->request_list[index_request++];

		sscanf(linha, "%i %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ",
				&rq->req_no,
				&rq->request_arrival_time,
				&rq->service_time_at_source,
				&rq->pickup_location_longitude,
				&rq->pickup_location_latitude,
				&rq->pickup_earliest_time,
				&rq->pickup_latest_time,
				&rq->service_time_at_delivery,
				&rq->delivery_location_long,
				&rq->delivery_location_latitude,
				&rq->delivery_earliest_time,
				&rq->delivery_latest_time);
		if(rq->req_no < drivers)
			rq->driver = true;
		else
			rq->driver = false;
	}

	fclose(fp);

	return g;
}

/*
void dealoc_population(Population *p){
	if (p != NULL){
		if (p->list != NULL){
			free(p->list);
		}
		free(p);
	}

}*/

/*aleatoriza o vetor informado*/
void shuffle(int *array, int n) {
    if (n > 1) {
    	int i;
		for (i = 0; i < n - 1; i++) {
		  int j = i + rand() / (RAND_MAX / (n - i) + 1);
		  int t = array[j];
		  array[j] = array[i];
		  array[i] = t;
		}
    }
}



/*Desaloca a população, mantendo os indivíduos alocados*/
void free_population(Population *population){
	if (population != NULL){
		free(population->list);
		free(population);
	}
}

/*Desaloca a população, desalocando também os indivíduos*/
void complete_free_population(Population *population){
	if (population != NULL){
		for (int i = 0; i < population->size; i++){
			if (population->list[i] != NULL)
				free(population->list[i]);
		}
		free(population);
	}
}

/*Desaloca a população da memória, sem desalocar o front!!!*/
void free_population_fronts(Fronts * f){
	for (int i = 0; i < f->size; f++){
		complete_free_population(f->list[i]);
	}
	free(f->list);
	free(f);
}


