// normalize an array of ints by the largest value in sample and calculate average of normalized vector

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

void print_vector(int *vec, int size){
    for(int j=0;j<size;j++)
    {
        printf("%d\t", vec[j]);
        
    }
    printf("\n");
}

int main(int argc, char **argv){
	if ( argc  != 3)
    {
		printf("Wrong arguments. Please use arguments <amount_of_elements> <seed_for_rand> \n");
		exit(-1);
    }

    int n = atoi(argv[1]);
    int seed = atoi(argv[2]);

	int *arr;
	float *arr_norm;
	float avg = 0;

	// alocate space for vectors
	arr = (int *)malloc(n * sizeof(int));
	arr_norm = (float *)malloc(n * sizeof(float));


	// fill vector with pseudorandom numbers
	for(int i = 0; i < n; i ++){
		arr[i] = rand() % 100;
	}

	// print_vector(arr, n);

	// find largest element in vector
	int elem_max = arr[0];

	double timestamp = omp_get_wtime();

	#pragma omp parallel for reduction(max: elem_max)
	for(int i = 0; i < n; i++){
		if(arr[i] > elem_max){
			elem_max = arr[i];
		}
	}

	printf("maximum element: %d\n", elem_max);
	// normalize and calculate average
	#pragma omp parallel for simd reduction(+:avg)
	for(int i = 0; i < n; i++){
		arr_norm[i] = (float) arr[i] / elem_max;
		avg += arr_norm[i];
	}

	printf("sum = %f, avg = %f\n", avg, avg/n);
	printf("time taken = %f\n", omp_get_wtime() - timestamp);

}