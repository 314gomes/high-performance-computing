// to compile: gcc tokenring_sem.c -o tokenring_sem -pthread
// to execute: tokenring_sem
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>


#define THREADS 20

pthread_t threads[THREADS];
sem_t semaphores[THREADS - 1];
sem_t mutex;

int thread_id_from_thread(pthread_t thread) {
	for (int i = 0; i < THREADS; i++) {
		if (pthread_equal(threads[i], thread)) {
			return i; // return thread id 
		}
	}
	return -1; // not found
} // thread_id_from_thread

void *thread_function() {
	// Thread function logic goes here
	// get thread id from self

	int thread_id = thread_id_from_thread(pthread_self());
	if (thread_id < 0) {
		printf("Error getting thread id! Exiting!\n");
		exit(1);
	}
	
	printf("Thread %d started!!!! \n", thread_id);

	// wait for this thread's semaphore
	if(thread_id > 0){
		sem_wait(&semaphores[thread_id]);
	}
	
	// simulate some work
	printf("Thread %d is running.\n", thread_id);


	// initialize thread id + 1
	if (thread_id < THREADS) {
		sem_post(&semaphores[(thread_id + 1) % THREADS]);
	}

	pthread_exit(NULL);


} // thread_function

int main(void) {

	// initialize semaphores
	for (int i = 0; i < THREADS; i++) {
		if (sem_init(&semaphores[i], 0, 0) != 0) {
			printf("Error initializing semaphore %d! Exiting!\n", i);
			exit(1);
		}
	}
	
	// initialize threads

	threads[0] = pthread_self(); // main thread is thread 0

	for (int i = 1; i < THREADS; i++) {
		if (pthread_create(&threads[i], NULL, thread_function, NULL) != 0) {
			printf("Error creating thread %d! Exiting!\n", i);
			exit(1);
		}
	}

	

	// start the first thread (main)
	// just run the function directly
	int i = 0;
	thread_function();

	// wait for all threads to finish
	for (i = 1; i < THREADS; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			printf("Error joining thread %d! Exiting!\n", i);
			exit(1);
		}
	}

	printf("Thread pai saindo.\n");
	fflush(0);
	
	exit(0);	
} // main()
