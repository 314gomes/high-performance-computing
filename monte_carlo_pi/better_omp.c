// Código OMP com melhoria de legibilidade, mas sem ganho real de desempenho

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

int main() {
	long int n;
	long int count = 0;
	double start, end, wall_clock_time;
	printf("\nn = ");
	scanf("%ld", &n);
	// Inicia a medição de tempo
	start = omp_get_wtime();
	// Inicia a região paralela
	#pragma omp parallel 
	{
		long int local_count = 0;
		// 1. CADA thread terá seu próprio buffer e variável para o resultado.
		// Isso evita a condição de corrida.
		struct drand48_data randBuffer;
		double x, y;

		// 2. CADA thread inicializa (semeia) seu próprio buffer.
		// Usamos o tempo + ID da thread para garantir sementes únicas.
		srand48_r(time(NULL) + omp_get_thread_num(), &randBuffer);

		// O loop é dividido entre as threads
		#pragma omp for private (x, y) reduction (+:count)
		for(long int i = 0; i < n; ++i) {
			// A função armazena o resultado em 'x' e 'y'.
			drand48_r(&randBuffer, &x); // Gera número aleatório e armazena em x
			drand48_r(&randBuffer, &y); // Gera número aleatório e armazena em y

			count += (x * x + y * y <= 1.0);
		}
	}
	
	// Finaliza a medição de tempo
	end = omp_get_wtime();
	// Calcula a estimativa de Pi
	long double pi = 4.0L * ((long double)count / n);
	printf("\nEstimativa de PI = %.9Lf\n", pi);
	wall_clock_time = end - start;
	printf("Tempo de execução: %f segundos\n", wall_clock_time);
	return 0;
}