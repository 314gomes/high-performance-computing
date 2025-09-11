// to compile: gcc prodcons_1_thread_sem.c -o prodcons_1_thread_sem -pthread
// to execute: prodcons_1_thread_sem
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_PRODUCED 27
#define MAX_QUEUE 7
#define MAX_P 3
#define MAX_C 3

sem_t  mutex, empty, full;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;


int create_item(void) {

	return(rand()%1000);

}

void insert_into_queue(int item) {
	
	queue[item_available++] = item;
	produced++;
	printf("producing item:%d, value:%d, queued:%d \n", produced, item, item_available); 
	return;

}

int extract_from_queue() {

	consumed++;
	printf("cosuming item:%d, value:%d, queued:%d \n", consumed, queue[item_available-1], item_available-1); 
	
	return(queue[--item_available]);

}

void process_item(int my_item) {
	static int printed=0;

	printf("Printed:%d, value:%d, queued:%d \n", printed++, my_item, item_available);

	return;

}


void *producer(void) {
	int item, stay = 1;
	
	do {
	  item = create_item();
	  sem_wait(&empty);
	  sem_wait(&mutex);
      if(produced < MAX_PRODUCED ) 
	    insert_into_queue(item);
      else
        stay = 0;
	  sem_post(&mutex);
	  sem_post(&full);
	}while(stay);

	printf("\nThread producer saindo.\n\n");
	fflush(0);

	pthread_exit(0);
}

void *consumer(void) {
	int my_item = 0, stay = 1;

	do{

	  sem_wait(&full);
	  sem_wait(&mutex);
      if(consumed < MAX_PRODUCED){
	    my_item = extract_from_queue();
      }else{
        stay = 0;
      }
	  sem_post(&mutex);
	  sem_post(&empty);

	  if(stay)
        process_item(my_item);

	}while(stay);

	printf("\nThread consumer saindo.\n\n");
	fflush(0);
	
	pthread_exit(0);
}

int main(void) {
	pthread_t prod_handle[MAX_P], cons_handle[MAX_C];

    item_available = 0;

	sem_init(&mutex, 0 , 1);
	sem_init(&empty, 0, MAX_QUEUE);
	sem_init(&full, 0, 0);

    for(int i = 0; i<MAX_P; i++){
        if (pthread_create(&prod_handle[i], 0, (void *) producer, (void *) 0) != 0) { 
            printf("Error creating thread producer! Exiting! \n");
            exit(0);
        }
    }

    for(int i = 0; i<MAX_C; i++){
        if (pthread_create(&cons_handle[i], 0, (void *) consumer, (void *) 0) != 0) { 
            printf("Error creating thread consumer! Exiting! \n");
            exit(0);
        }
    }

	fflush(0);
    
    for(int i = 0; i<MAX_P; i++)
	    pthread_join(prod_handle[i], 0);

    for(int i = 0; i<MAX_C; i++)
	    pthread_join(cons_handle[i], 0);

	// getchar();

	printf("Parent thread leaving.\n");
	fflush(0);
	
	exit(0);	
} 
