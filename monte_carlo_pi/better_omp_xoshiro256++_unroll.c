// Incrementação da versão prévia do código
// mas com troca do algoritmo de geração de
// números aleatórios

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <stdint.h>

#define XOSHIRO256_UNROLL (8) // define o fator de unroll 

// função para rotacionar bits
static __inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static __inline double to_double(uint64_t x) {
	const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | x >> 12 };
	return u.d - 1.0;
}

// função para gerar o próximos XOSHIRO256_UNROLL números pseudo aleatórios

static uint64_t next(uint64_t s[4][XOSHIRO256_UNROLL], double * const restrict array) {
 	uint64_t t[XOSHIRO256_UNROLL];
	// for(int i = 0; i < XOSHIRO256_UNROLL; i++) array[i] = ((rotl(s[0][i] + s[3][i], 23) + s[0][i]) >> 11) * 0x1.0p-53;
	for(int i = 0; i < XOSHIRO256_UNROLL; i++) array[i] = to_double(rotl(s[0][i] + s[3][i], 23) + s[0][i]);

	for(int i = 0; i < XOSHIRO256_UNROLL; i++) t[i] = s[1][i] << 17;

	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= s[0][i];
	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] ^= s[1][i];
	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[1][i] ^= s[2][i];
	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[0][i] ^= s[3][i];

	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= t[i];

	for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] = rotl(s[3][i], 45);

	// This is just to avoid dead-code elimination
	// return array[0] + array[XOSHIRO256_UNROLL - 1];
}

// função para inicializar o estado do gerador de números aleatórios
static void init(uint64_t s[4][XOSHIRO256_UNROLL]) {
	// semente baseada somente no numero da thread e não no tempo
	unsigned int seed = (unsigned int) omp_get_thread_num() + 1;
	for(int i = 0; i < XOSHIRO256_UNROLL; i++) {
		for(int j = 0; j < 4; j++) {
			// Usando um gerador simples para inicializar o estado
			s[j][i] = j + 100 * i + 10000 * seed;
		}
	}
}


int main() {
	long int n;
	long int count = 0;
	double start, end, wall_clock_time;
	printf("\nn = ");
	scanf("%ld", &n);

	// n deve ser múltiplo de XOSHIRO256_UNROLL
	if (n % XOSHIRO256_UNROLL != 0) {
		printf("n deve ser múltiplo de XOSHIRO256_UNROLL = %d\n", XOSHIRO256_UNROLL);
		return 1;
	}

	// Inicia a medição de tempo
	start = omp_get_wtime();

	// Inicia a região paralela
	#pragma omp parallel
	{
		long int local_count = 0;
		// 1. cada thread terá seu próprio estado de números aleatorios
		uint64_t s[4][XOSHIRO256_UNROLL];
		
		// 2. CADA thread inicializa (semeia) seu estado de números aleatórios
		init(s);

		// imprimir estado inicial para debug
		// for (int i = 0; i < XOSHIRO256_UNROLL; i++) {
		// 	for (int j = 0; j < 4; j++) {
		// 		printf("Thread %d: s[%d][%d] = %lu\n", omp_get_thread_num(), j, i, s[j][i]);
		// 	}
		// }

		
		// O loop é dividido entre as threads
		#pragma omp for simd
		for(long int i = 0; i < n; i += XOSHIRO256_UNROLL) {
			local_count = 0;
			double x[XOSHIRO256_UNROLL], y[XOSHIRO256_UNROLL];
			// Gera XOSHIRO256_UNROLL pontos aleatórios (x e y), x y entre 0 e 1			
			next(s, x);
			next(s, y);

			// printf("Thread %d: x = %.9f, y = %.9f\n", thread_id, x, y);

			// Verifica quais pontos estão dentro do círculo
			// #pragma omp unroll simd
			for(int j = 0; j < XOSHIRO256_UNROLL; j++) {
				// printf("Thread %d: x[%d] = %.9f, y[%d] = %.9f\n", omp_get_thread_num(), j, x[j], j, y[j]);
				local_count += (x[j] * x[j] + y[j] * y[j] <= 1.0);
			}
		}

		#pragma omp reduction(+:count)
		count += local_count;

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