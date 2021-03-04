#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"

// Single Producer - Multiple Consumer queue
template <typename T>
class PCQueue
{

public:
	//constructor
	PCQueue();
    int getSize();
	~PCQueue();
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.
	T pop(); 

	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer 
	void push(const T& item);
    int consumers_waiting;

private:
	queue<T>* items;
	int producers_inside;
	int consumers_inside;
	int producer_waiting;

	pthread_cond_t producer_allowed;
	pthread_cond_t consumer_allowed;
	pthread_cond_t notEmpty;
	pthread_mutex_t mutex;

};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif


template <typename T>
PCQueue<T>::PCQueue(){
	items=new queue<T>;
	producer_waiting=0;
	producers_inside=0;
	consumers_inside=0;
    consumers_waiting=0;
	pthread_cond_init(&producer_allowed, NULL);
	pthread_cond_init(&consumer_allowed, NULL);
	pthread_cond_init(&notEmpty, NULL);
	pthread_mutex_init(&mutex, NULL);
}
template <typename T>
int PCQueue<T>::getSize(){
 return items->size();
}
template <typename T>
void PCQueue<T>::push(const T& item){
	//lock producing and wait for producers and consumers
	pthread_mutex_lock(&mutex);
	producer_waiting++;
	while( producers_inside + consumers_inside > 0){
		pthread_cond_wait(&producer_allowed, &mutex);
	}
	producer_waiting--;
	producers_inside++;
	//------------ enqueue --------------
	items->push(item);
	pthread_cond_signal(&notEmpty);
	//-----------------------------------
	//unlock producing
	producers_inside--;
	if(producers_inside==0){
        pthread_cond_signal(&producer_allowed);
		pthread_cond_broadcast(&consumer_allowed);

	}
	pthread_mutex_unlock(&mutex);

}


template <class T>
PCQueue<T>::~PCQueue(){
	pthread_cond_destroy(&producer_allowed);
	pthread_cond_destroy(&notEmpty);
	pthread_cond_destroy(&consumer_allowed);
	pthread_mutex_destroy(&mutex);
	delete items;
}


template <class T>
T PCQueue<T>::pop(){
	//lock producing and wait for producers and consumers
	pthread_mutex_lock(&mutex);
	consumers_waiting++;
    while( items->size() ==0 ) {
        pthread_cond_wait(&notEmpty, &mutex);
    }
	while( producers_inside>0 || producer_waiting>0){
		pthread_cond_wait(&consumer_allowed, &mutex);
	}

    consumers_waiting--;
	consumers_inside++;
	//------------ dequeue --------------
	T subhi=items->front();
	items->pop();
	//-----------------------------------
	//unlock producing
	consumers_inside--;
	if(consumers_inside==0){

		pthread_cond_signal(&producer_allowed);
	}
	pthread_mutex_unlock(&mutex);
	return subhi;
}
