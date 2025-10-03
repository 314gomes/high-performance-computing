#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include <immintrin.h>

void print_vector(int *vec, int size){
    for(int j=0;j<size;j++)
    {
        printf("%d\t", vec[j]);
        
    }
    printf("\n");
}

void print_matrix(int **mat, int r, int c){
    for(int i=0;i<r;i++)
	{
        print_vector(mat[i], c);
    }
}

void print_square_matrix(int **mat, int size){
    print_matrix(mat, size, size);
}

void get_diff_vector(int *vec1, int *vec2, int *diff, int size){
    #pragma omp simd
    for(int i = 0; i < size; i++){
        diff[i] = abs(vec2[i] - vec1[i]);
    }
}
// define a struct to hold coupled data (required for declared reduction)
typedef struct {
    int value;
    int index;
} ArrInfo_t;

#pragma omp declare reduction( \
    max_info_reduction : ArrInfo_t : \
    omp_out = (omp_in.value > omp_out.value) ? omp_in : omp_out \
) initializer(omp_priv = {INT_MIN, -1})

void get_max_diff(int *vec1, int *vec2, int *diff_vec, int size, int *max_value, int *min_value, int *diff){
    ArrInfo_t max_result = {diff_vec[0], 0};
    
    // does not seem to have a positive effect here :(
    // #pragma omp parallel for reduction(max_info_reduction:max_result)
    for (int i = 0; i < size; i++) {
        ArrInfo_t current_result = {diff_vec[i], i};
        max_result = (current_result.value > max_result.value) ? current_result : max_result;
    }
    
    if(vec1[max_result.index] > vec2[max_result.index]){
        *max_value = vec1[max_result.index];
        *min_value = vec2[max_result.index];
    }
    else{
        *min_value = vec1[max_result.index];
        *max_value = vec2[max_result.index];
    }

    *diff = max_result.value;
}

// this is a generic function to get the vertical differences (off = 0) as well as
// the main diagonal differences (off = 1) and secondary diagonal differences (off = -1)
// there are two possibilities here: gathering every difference in a single
// "diff matrix" or allocating space for each row and storing the
// max diff of each row in a "max diff for each row" vector.
// I'm chosing the second option as it uses considerably less space and I suspect
// it is not going to have a huge impact on performance
void gmd_rows_with_offset(int **mat, int size, int off, int *max_value, int *min_value, int *max_diff){
    int *row_max_diff = malloc(sizeof(int) * (size - 1));
    int *row_max_value = malloc(sizeof(int) * (size - 1));
    int *row_min_value = malloc(sizeof(int) * (size - 1));

    // iterate over rows
    #pragma omp parallel for
    for(int j = 0; j < size - 1; j++){
        int* diff = (int *) malloc((size - abs(off)) * sizeof(int));
        
        if(off >= 1){
            get_diff_vector(
                mat[j],
                mat[j + 1] + off,
                diff, size - abs(off)
            );
            
            get_max_diff(
                mat[j],
                mat[j + 1] + off,
                diff, size - abs(off),
                row_max_value + j,
                row_min_value + j,
                row_max_diff + j
            );
        }

        else{
            get_diff_vector(
                mat[j]  - off,
                mat[j + 1],
                diff, size - abs(off)
            );
            
            get_max_diff(
                mat[j]- off,
                mat[j + 1],
                diff, size - abs(off),
                row_max_value + j,
                row_min_value + j,
                row_max_diff + j

            );
        }
        
        // printf("for row %d, differences: \n", j);
        // print_vector(diff, (size - abs(off)));
        // printf("for row %d, max_value: %d, min_value: %d\n\n", j, row_max_value[j], row_min_value[j]);

        free(diff);
    }

    // printf("row differences vector is is:\n");
    // print_vector(row_max_diff, size - 1);
    get_max_diff(row_max_value, row_min_value, row_max_diff, size - 1, max_value, min_value, max_diff);

}


void transpose_mat_old(int **mat, int size){
    #pragma omp parallel for
    for(int i = 0; i < size; i ++){
        for(int j = 0; j < i; j ++){
            int aux = mat[i][j];
            mat[i][j] = mat[j][i];
            mat[j][i] = aux;
        }
    }
}

// Inspired from https://stackoverflow.com/a/16743203/20619087
// the sse implementation ended up being faster than the avx2 one
void transpose_mat_sse(int **mat, int size){
    #pragma omp parallel loop
    for(int i = 0; i < size; i += 4){
        __m128i row0j;
        __m128i row1j;
        __m128i row2j;
        __m128i row3j;
        
        __m128i rowj0;
        __m128i rowj1;
        __m128i rowj2;
        __m128i rowj3;
        
        __m128i tmp0;
        __m128i tmp1;
        __m128i tmp2;
        __m128i tmp3;
        
        for(int j = 0; j < i; j += 4){
            row0j = _mm_load_si128((const __m128i*) (&mat[i + 0][j]));
            row1j = _mm_load_si128((const __m128i*) (&mat[i + 1][j]));
            row2j = _mm_load_si128((const __m128i*) (&mat[i + 2][j]));
            row3j = _mm_load_si128((const __m128i*) (&mat[i + 3][j]));
            
            tmp0 = _mm_unpacklo_epi32(row0j, row1j);
            tmp1 = _mm_unpackhi_epi32(row0j, row1j);
            tmp2 = _mm_unpacklo_epi32(row2j, row3j);
            tmp3 = _mm_unpackhi_epi32(row2j, row3j);
            
            row0j = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp0), _mm_castsi128_ps(tmp2)));
            row1j = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp2), _mm_castsi128_ps(tmp0)));
            row2j = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp1), _mm_castsi128_ps(tmp3)));
            row3j = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp3), _mm_castsi128_ps(tmp1)));
            
            rowj0 = _mm_load_si128((const __m128i*) (&mat[j + 0][i]));
            rowj1 = _mm_load_si128((const __m128i*) (&mat[j + 1][i]));
            rowj2 = _mm_load_si128((const __m128i*) (&mat[j + 2][i]));
            rowj3 = _mm_load_si128((const __m128i*) (&mat[j + 3][i]));
            
            _mm_store_si128((__m128i*) (&mat[j + 0][i]), row0j);
            _mm_store_si128((__m128i*) (&mat[j + 1][i]), row1j);
            _mm_store_si128((__m128i*) (&mat[j + 2][i]), row2j);
            _mm_store_si128((__m128i*) (&mat[j + 3][i]), row3j);
                        
            tmp0 = _mm_unpacklo_epi32(rowj0, rowj1);
            tmp1 = _mm_unpackhi_epi32(rowj0, rowj1);
            tmp2 = _mm_unpacklo_epi32(rowj2, rowj3);
            tmp3 = _mm_unpackhi_epi32(rowj2, rowj3);
            
            rowj0 = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp0), _mm_castsi128_ps(tmp2)));
            rowj1 = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp2), _mm_castsi128_ps(tmp0)));
            rowj2 = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp1), _mm_castsi128_ps(tmp3)));
            rowj3 = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp3), _mm_castsi128_ps(tmp1)));

            _mm_store_si128((__m128i*) (&mat[i + 0][j]), rowj0);
            _mm_store_si128((__m128i*) (&mat[i + 1][j]), rowj1);
            _mm_store_si128((__m128i*) (&mat[i + 2][j]), rowj2);
            _mm_store_si128((__m128i*) (&mat[i + 3][j]), rowj3);

        }

        row0j = _mm_load_si128((const __m128i*) (&mat[i + 0][i]));
        row1j = _mm_load_si128((const __m128i*) (&mat[i + 1][i]));
        row2j = _mm_load_si128((const __m128i*) (&mat[i + 2][i]));
        row3j = _mm_load_si128((const __m128i*) (&mat[i + 3][i]));

        tmp0 = _mm_unpacklo_epi32(row0j, row1j);
        tmp1 = _mm_unpackhi_epi32(row0j, row1j);
        tmp2 = _mm_unpacklo_epi32(row2j, row3j);
        tmp3 = _mm_unpackhi_epi32(row2j, row3j);

        row0j = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp0), _mm_castsi128_ps(tmp2)));
        row1j = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp2), _mm_castsi128_ps(tmp0)));
        row2j = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tmp1), _mm_castsi128_ps(tmp3)));
        row3j = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp3), _mm_castsi128_ps(tmp1)));

        _mm_store_si128((__m128i*) (&mat[i + 0][i]), row0j);
        _mm_store_si128((__m128i*) (&mat[i + 1][i]), row1j);
        _mm_store_si128((__m128i*) (&mat[i + 2][i]), row2j);
        _mm_store_si128((__m128i*) (&mat[i + 3][i]), row3j);
    }
}

void gmd_mat(int **mat, int size, int *max_value, int *min_value, int *max_diff){
    int max_values[4];
    int min_values[4];
    int diffs[4];
    
    
    gmd_rows_with_offset(mat, size, 0, &max_values[0], &min_values[0], &diffs[0]);
    gmd_rows_with_offset(mat, size, 1, &max_values[1], &min_values[1], &diffs[1]);
    gmd_rows_with_offset(mat, size, -1, &max_values[2], &min_values[2], &diffs[2]);
    
    // printf("Matrix is:\n");
    // print_square_matrix(mat, size); 
    // transpose_mat_old(mat, size);
    transpose_mat_sse(mat, size);

    // printf("Transpose is:\n");
    // print_square_matrix(mat, size); 

    gmd_rows_with_offset(mat, size, 0, &max_values[3], &min_values[3], &diffs[3]);    

    // print_vector(max_values, 4);
    // print_vector(min_values, 4);
    // print_vector(diffs, 4);

    get_max_diff(max_values, min_values, diffs, 4, max_value, min_value, max_diff);
}



// Main idea: help the compiler use SIMD instructions by separating
// the problems of finding the max horizontal, vertical and diagonal diffs
// main problem: finding the difference vector for the horizontal neighbours
// is not so trivial for SIMD instructions, as data is scattered
// To solve this, I thought of two possibilities: trying to get the compiler to
// use scatter instructions or calculating the transpose of a matrix.
// Will try to analyze the differences
int main(int argc, char **argv)
{
    int tam,seed;

    int valor_maior_aux, valor_menor_aux;
    int valor_maior, valor_menor=0;

    int dif_aux, diferenca = INT_MIN;

	double wtime;


	if ( argc  != 3)
    {
		printf("Wrong arguments. Please use arguments <amount_of_elements> <seed_for_rand> \n");
		exit(-1);
    }

    tam = atoi(argv[1]);
    seed = atoi(argv[2]);

    wtime = omp_get_wtime();

	srand(seed);	// setting starting point to rand();
    int **matrix = (int **)malloc(tam * sizeof(int *)); // alocar espaço para ponteiro de cada linha
    for(int i=0;i<tam;i++)
	{
        matrix[i] = (int *) _mm_malloc(tam * sizeof(int), 16); // alocar a linha
        for(int j=0;j<tam;j++)
		{
            // store matrix in row-major order
            matrix[i][j] = rand() % 100;;
            // matrix[i][j] = i * 10 + j;
        }
    }

    wtime = omp_get_wtime ( ) - wtime; 
    printf("Time to fill data = %.5f\n\n", wtime );

	// matriz[0][2] = 0;
	// matriz[1][1] = 99;

    // matrix[0][2] = 0;
    // matrix[1][1] = 101;


    // printf("Matrix is:\n");
    // print_square_matrix(matrix, tam); 
    // printf("\n");


    int *vec1, *vec2, *diff;
    diff = (int *)malloc(tam * sizeof(int));

	wtime = omp_get_wtime();

    gmd_mat(matrix, tam, &valor_maior, &valor_menor, &diferenca);
    
    wtime = omp_get_wtime ( ) - wtime;

    printf("max_val = %d, min_val = %d\n\n", valor_maior, valor_menor);
    printf("Time to solve = %.5f\n\n", wtime );

    // printf("%d %d \n",valor_maior,valor_menor);
	fflush(0);
	
	return(0);
}
