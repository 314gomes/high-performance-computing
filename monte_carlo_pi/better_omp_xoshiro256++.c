// Incrementação da versão prévia do código
// mas com troca do algoritmo de geração de
// números aleatórios

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <stdint.h>


// função para rotacionar bits
static __inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

// função para gerar o próximo número pseudo aleatório
static uint64_t next(uint64_t s[4]) {
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

static __inline double to_double(uint64_t x) {
	const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | x >> 12 };
	return u.d - 1.0;
}

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
		// 1. cada thread terá seu próprio estado de números aleatorios
		uint64_t s[4];
		
		// 2. CADA thread inicializa (semeia) seu estado de números aleatórios
		int thread_id = omp_get_thread_num();
		s[0] = 0x01ull << (thread_id + 0);
		s[1] = 0x01ull << (thread_id + 16);
		s[2] = 0x01ull << (thread_id + 32);
		s[3] = 0x01ull << (thread_id + 48);
		
		// O loop é dividido entre as threads
		#pragma omp for reduction (+:count)
		for(long int i = 0; i < n; ++i) {
			double x, y;
			// Gera dois números aleatórios (x e y) entre 0 e 1			
			x = to_double(next(s));
			y = to_double(next(s));


			// printf("Thread %d: x = %.9f, y = %.9f\n", thread_id, x, y);

			// Verifica se o ponto (x, y) está dentro do círculo unitário
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