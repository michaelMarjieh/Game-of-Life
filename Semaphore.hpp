#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "Headers.hpp"
#include<pthread.h>
// Synchronization Warm up 
class Semaphore {
public:
	Semaphore(); // Constructs a new semaphore with a counter of 0
	Semaphore(unsigned val); // Constructs a new semaphore with a counter of val


	void up(); // Mark: 1 Thread has left the critical section
	void down(); // Block untill counter >0, and mark - One thread has entered the critical section.
    ~Semaphore();
private:
    pthread_cond_t cond;
	pthread_mutex_t mutex;
	int counter;
	int wakes;
	//int count1;
};

#endif
