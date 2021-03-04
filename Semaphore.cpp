/*
 * Semaphore.cpp
 *
 *  Created on: Dec 23, 2018
 *      Author: micha
 */

//#include "stdafx.h"
#include "Semaphore.hpp"
Semaphore::Semaphore(){

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	counter=0;
	wakes=0;
	//count1=0;
}
Semaphore::Semaphore(unsigned val){
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	counter=val;
	wakes=0;
//	count1=0;
}
void Semaphore::up(){
	pthread_mutex_lock(&mutex);

	counter++;
	if(counter<=0){
		wakes++;
		pthread_cond_signal(&cond);
	}

	pthread_mutex_unlock(&mutex);






}
void Semaphore::down(){

	pthread_mutex_lock(&mutex);

	counter--;
	if (counter < 0) {
		do {
			pthread_cond_wait(&cond, &mutex);
		} while (wakes < 1);
		wakes--;
	}


	pthread_mutex_unlock(&mutex);

	}

Semaphore::~Semaphore(){

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}
