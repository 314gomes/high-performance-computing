#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <omp.h>

void calc_diferenca(int **matriz, int linha, int coluna, int tam, int *dif, int *valor_maior, int *valor_menor)
{
    int aux_dif = 0;
    int i, j;

	*dif = 0;
    for (i = linha - 1; i <= linha + 1; i++) 
	{
        for (j = coluna - 1; j <= coluna + 1; j++) 
		{
			if ( (i >= 0 && i < tam) && (j >= 0 && j < tam) && (i !=  linha || j!=coluna)  )
			{
                aux_dif = abs(matriz[linha][coluna] - matriz[i][j]);
                if (aux_dif > *dif)
				{
                    *dif = aux_dif;
					if (matriz[linha][coluna] > matriz[i][j]) 
					{
		                *valor_maior=matriz[linha][coluna];
		                *valor_menor=matriz[i][j];
					} else 
					{
						*valor_maior=matriz[i][j];
		                *valor_menor=matriz[linha][coluna];
					}
                }
            }
        }
    }
    return;
}

int main(int argc, char **argv)
{
    int tam,seed,i,j=0;
    int **matriz;

    int valor_maior_aux,valor_menor_aux;
    int valor_maior,valor_menor=0;

    int dif_aux, diferenca = INT_MIN;

	double wtime;


	if ( argc  != 3)
    {
		printf("Wrong arguments. Please use arguments <amount_of_elements> <seed_for_rand> \n");
		exit(-1);
    }

    tam = atoi(argv[1]);
    seed = atoi(argv[2]);

    matriz = ((int**)malloc(tam*sizeof(int*)));
	for(i=0;i<tam;i++)
	{
        matriz[i]=((int*)malloc(tam*sizeof(int)));
    }

    wtime = omp_get_wtime();

	srand(seed);	// setting starting point to rand();
    for(i=0;i<tam;i++)
	{
        for(j=0;j<tam;j++)
		{
            matriz[i][j] = rand() % 100; // generate numbers
        }
    }

    wtime = omp_get_wtime ( ) - wtime; 
    printf("Time to fill data = %.5f\n\n", wtime );

	matriz[0][2] = 0;
	matriz[1][1] = 101;
	
	wtime = omp_get_wtime();
	
    for (i = 0; i < tam; i++) 
	{
        for (j = 0; j < tam; j++) 
		{
        	calc_diferenca(matriz, i, j, tam, &dif_aux, &valor_maior_aux, &valor_menor_aux);

		    if (diferenca < dif_aux)
			{
		        diferenca = dif_aux;
		        valor_maior = valor_maior_aux;
		        valor_menor = valor_menor_aux;
		    }
        }
    }
	
    wtime = omp_get_wtime ( ) - wtime; 
    printf("Time to solve = %.5f\n\n", wtime );

    printf("%d %d \n",valor_maior,valor_menor);
	fflush(0);



    for(i=0;i<tam;i++)
	{
       free(matriz[i]); 
    }
    free(matriz);
	
	return(0);
}
