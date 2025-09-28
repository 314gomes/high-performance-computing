#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<omp.h>
#include<time.h>
//Quicksort adaptado de //https://www.geeksforgeeks.org/quick-sort/
int partition (double *arr, int low, int high, int C){
    int i, j;
    double pivot,swap;
    
    // pivot (Element to be placed at right position)
    pivot = arr[high*C];  
 
    i = (low - 1);  // Index of smaller element
    for (j = low; j <= high-1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j*C] <= pivot)
        {
            i++;    // increment index of smaller element
            
            // swap arr[i] and arr[j]
            swap = arr[i*C];
    arr[i*C] = arr[j*C];
    arr[j*C] = swap;
        }
    }
    
    //swap arr[i + 1] and arr[high]
    swap = arr[(i + 1)*C];
    arr[(i + 1)*C] = arr[high*C];
    arr[high*C] = swap;
    
    return (i + 1);
  
} // fim partition
/* low  --> Starting index,  high  --> Ending index */
void quicksort(double *arr, int low, int high, int C){
    int pi;
    
    if (low < high)  {
        /* pi is partitioning index, arr[pi] is now
           at right place */
        pi = partition(arr, low, high, C);
        quicksort(arr, low, pi - 1, C);  // Before pi
        quicksort(arr, pi + 1, high, C); // After pi
    }
    
} // fim quicksort
/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot 
   https://www.geeksforgeeks.org/quick-sort/
*/
void ordena_colunas_cm(double *matriz, int lin, int col) {
  int j;
  
  #pragma omp parallel for
  for (j = 0; j < col; j++) {
      //manda o endereco do primeiro elemento da coluna, limites inf e sup e a largura da matriz
      quicksort(&matriz[j*lin], 0, lin-1, 1);
  }
}

void calcula_media_cm(double *matriz_cm, double *vet, int lin, int col){
    int i,j;
    double soma;

    // #pragma omp parallel for
    for(j = 0; j < col; j++){
        soma = 0;
        // #pragma omp parallel simd reduction(+=:soma)
        for(i = 0; i < lin; i++){
            soma+=matriz_cm[j*lin + i];
        }
        vet[j]=soma/lin; 
    }
}

void calcula_media_harmonica_cm(double *matriz_cm, double *vet, int lin, int col){
    for(int j = 0; j < col; j++){
        double soma = 0;
        
        for(int i = 0; i < lin; i++){
            soma += (1 / (matriz_cm[j*lin + i]));
        }

        vet[j] = lin/soma;
    }
}

void calcula_mediana_cm(double *matriz, double *vet, int lin, int col) {
    for(int j = 0; j < col; j++){
        vet[j] = matriz[j * lin + (lin/2)];
        if(!(lin%2))  {
            vet[j]+=matriz[j * lin + ((lin - 1)/2)];
            vet[j]*=0.5;
        } 
    }
}

//Adaptado de https://www.clubedohardware.com.br/forums/topic/1291570-programa-em-c-que-calcula-moda-media-e-mediana/
void calcula_moda_cm(double *matriz, double *moda, int lin, int col){
    for(int j = 0; j < col; j++){
        int frequencyCounter = 0;
        int maxFrequency = 0;
        float moda_local;

        for(int i = 0; i < lin - 1; i++){
            if(matriz[j * lin + i] == matriz[j * lin + i + 1]){
                frequencyCounter++;
            }
            else{
                if(frequencyCounter > maxFrequency){
                    maxFrequency = frequencyCounter;
                    moda_local = matriz[j * lin + i];
                }

                frequencyCounter = 0;
            }

        }
        if(maxFrequency == 0){
            moda[j] = -1;
        }
        else{
            moda[j] = moda_local;
        }
    }
}

void calcula_variancia_cm(double *matriz, double *media,double *variancia, int lin, int col){
    for(int j = 0; j < col; j++){
        double soma = 0;
        for(int i = 0; i < lin; i++){
            soma += pow((matriz[j * lin + i] - media[j]), 2);
        }
        variancia[j]=soma/(lin-1); 
    }
}

void calcula_desvio_padrao(double *variancia,double *dp, int col)
{
    int i;
    for(i=0;i<col;i++){
        dp[i]=sqrt(variancia[i]);
    }  
}

void calcula_coeficiente_variacao(double *media,double *dp,double *cv, int col)
{
    int i;
    for(i=0;i<col;i++){
        cv[i]=dp[i]/media[i];
    }  
}

int main(int argc,char **argv){
    int lin,col,i,j; //Define as variáveis de índices e dimensões
    double *matriz,*mediana,*media,*media_har,*moda,*variancia,*dp,*cv; //Define a matriz (forma linear), vetores de medidas estatísticas
    double *matriz_column_major;
    
    fscanf(stdin, "%d ", &lin); //Lê a quantidade de linhas da matriz
    fscanf(stdin, "%d\n", &col); //Lê a quantidade de colunas da matriz
    matriz=(double *)malloc(lin*col * sizeof(double)); //Aloca a matriz
    matriz_column_major=(double *)malloc(lin*col * sizeof(double)); //Aloca a matriz
    media=(double *)malloc(col * sizeof(double)); //Aloca o vetor de media
    media_har=(double *)malloc(col * sizeof(double)); //Aloca o vetor de media harmônica
    mediana=(double *)malloc(col * sizeof(double)); //Aloca o vetor de mediana
    moda=(double *)malloc(col * sizeof(double)); //Aloca o vetor de moda
    variancia=(double *)malloc(col * sizeof(double)); //Aloca o vetor de variância
    cv=(double *)malloc(col * sizeof(double)); //Aloca o vetor de coeficiente de variação
    dp=(double *)malloc(col * sizeof(double)); //Aloca o vetor de desvio padrão
    
    
    double time_read = omp_get_wtime();
    {
        for(i=0;i<lin;i++){
            for(j=0;j<col;j++){
                double read;
                fscanf(stdin, "%lf ",&(read)); //Lê a matriz
                matriz[i*col + j] = read;
            }
            {
                for(j = 0; j < col; j++)
                    matriz_column_major[j*lin+i] = matriz[i*col + j];
            }
            
        }
    }
    time_read = omp_get_wtime() - time_read;
    
    // print matrix to verify correct input
    // for(i=0;i<lin;i++){
    //     for(j=0;j<col;j++){
    //         printf("%lf ", matriz[i*col+j]);
    //     }
    //     printf("\n");
    // }

    // printf("##### \n");
    
    // for(i=0;i<lin;i++){
    //     for(j=0;j<col;j++){
    //         printf("%lf ", matriz_column_major[j*lin+i]);
    //     }
    //     printf("\n");
    // }

    // printf("##### \n");


    double time_avg = omp_get_wtime();
    calcula_media_cm(matriz_column_major, media, lin, col);
    time_avg = omp_get_wtime() - time_avg;
    
    double time_harmonic = omp_get_wtime();
    calcula_media_harmonica_cm(matriz_column_major, media_har, lin, col);
    time_harmonic = omp_get_wtime() - time_harmonic;
    
    double time_sort_cm = omp_get_wtime();
    ordena_colunas_cm(matriz_column_major,lin,col);
    time_sort_cm = omp_get_wtime() - time_sort_cm;
    
    // printf("##### \n");
    
    // for(i=0;i<lin;i++){
    //     for(j=0;j<col;j++){
    //         printf("%lf ", matriz_column_major[j*lin+i]);
    //     }
    //     printf("\n");
    // }

    // printf("##### \n");

    double time_median = omp_get_wtime();
    calcula_mediana_cm(matriz_column_major,mediana,lin,col);
    time_median = omp_get_wtime() - time_median;
    
    double time_mode = omp_get_wtime();
    calcula_moda_cm(matriz_column_major, moda, lin, col);
    time_mode = omp_get_wtime() - time_mode;
    
    double time_variance = omp_get_wtime();
    calcula_variancia_cm(matriz_column_major,media,variancia,lin,col);
    time_variance = omp_get_wtime() - time_variance;
    
    double time_stddev = omp_get_wtime();
    calcula_desvio_padrao(variancia,dp,col);
    time_stddev = omp_get_wtime() - time_stddev;
    
    double time_cv = omp_get_wtime();
    calcula_coeficiente_variacao(media,dp,cv,col);
    time_cv = omp_get_wtime() - time_cv;
    
    
    
    
    for(i=0;i<col;i++)
        printf("%.1lf ",media[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",media_har[i]);
    printf("\n");
    for(i=0;i<col;i++)
       printf("%.1lf ",mediana[i]);
    printf("\n");
    for(i=0;i<col;i++)
       printf("%.1lf ",moda[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",variancia[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",dp[i]);
    printf("\n");
    for(i=0;i<col;i++)
        printf("%.1lf ",cv[i]);


    
    free(matriz); //Desaloca a matriz
    free(media); //Desaloca o vetor de media
    free(media_har); //Desaloca o vetor de media_har
    free(mediana); //Desaloca vetor de mediana
    free(moda); //Desaloca vetor de moda
    free(variancia);  //Desaloca vetor de variância
    free(dp); //Desaloca vetor de desvio padrão
    free(cv); //Desaloca vetor de coeficiente de variação


    // print times in a neat format
    printf("\n");
    fflush(stdout);
    fprintf(stderr, "Time taken to read input: %lf seconds\n", time_read);
    fprintf(stderr, "Time taken to calculate average: %lf seconds\n", time_avg);
    fprintf(stderr, "Time taken to calculate harmonic mean: %lf seconds\n", time_harmonic);
    fprintf(stderr, "Time taken to sort columns (column major): %lf seconds\n", time_sort_cm);
    fprintf(stderr, "Time taken to calculate mode: %lf seconds\n", time_mode);
    fprintf(stderr, "Time taken to calculate median: %lf seconds\n", time_median);
    fprintf(stderr, "Time taken to calculate variance: %lf seconds\n", time_variance);
    // fprintf(stderr, "Time taken to calculate standard deviation: %lf seconds\n", time_stddev);
    // fprintf(stderr, "Time taken to calculate coefficient of variation: %lf seconds\n", time_cv);

}