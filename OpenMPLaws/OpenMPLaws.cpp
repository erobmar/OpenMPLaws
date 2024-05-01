/****************************************************************************************//** 
*                                                                                           *
*       OpenMPLaws.cpp      -       Comprobación Leyes Amdahl/Gustafson                     *
*                                                                                           *
* \brief    Multiplica dos matrices incrementando sus tamaños y el número de hilos usado    * 
*           para comprobar de forma empírica la eficiencia de la programación multihilo     *
*                                                                                           *
* \version  1.0                                                                             *
*                                                                                           *
* \author   Eduardo Robledo Martínez - erobmar@gmail.com                                    *
*                                                                                           *
 ********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

 /****************************************************************************************//**
  *                                                                                          *
  * \fn multiplicaMatrices(int** A, int** B, int** C, int N)                                 *
  *                                                                                          *
  * \brief  multiplica dos matrices de tamaño N x N                                          *
  *                                                                                          *
  * \param  A   Primera matriz operando de la multiplicación                                 *
  * \param  B   Segunda matriz operando de la multiplicación                                 *                                                                                          *
  * \param  C   Matriz para el resutlado del producto                                        *
  * \version    1.0                                                                          *
  ********************************************************************************************/
void multiplicaMatrices(int** A, int** B, int** C, int N) {
#pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    // El array de enteros alamcena los distintos tamaños de matriz con los que se va a trabajar y
    // es uno de los valores con los que se puede jugar para profundizar más en el análisis

    int tamanios[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    int numTamanios = sizeof(tamanios) / sizeof(int);
    int maxHilos = omp_get_max_threads();   // Número máximo de hilos que soporta la CPU, depende del equipo en que se ejecute

    FILE* fp;
    errno_t err;
    err = fopen_s(&fp, "tiempos.txt", "w");
    if (fp == NULL) {
        printf("Error al abrir el archivo\n");
        exit(1);
    }

    // Iteramos por cada uno de los tamaños del array definido antes, reservamos memoria para la matriz correpondiente y la
    // inicializamos con valores aleatorios entre 0 y 9
    for (int t = 0; t < numTamanios; t++) {
        int N = tamanios[t];
        int** A, ** B, ** C;
        A = (int**)malloc(N * sizeof(int*));
        B = (int**)malloc(N * sizeof(int*));
        C = (int**)malloc(N * sizeof(int*));
        for (int i = 0; i < N; i++) {
            A[i] = (int*)malloc(N * sizeof(int));
            B[i] = (int*)malloc(N * sizeof(int));
            C[i] = (int*)malloc(N * sizeof(int));
            for (int j = 0; j < N; j++) {
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }
        }

        // Cálculo del tiempo en modo secuencial
        double tiempoInicio = omp_get_wtime();
        multiplicaMatrices(A, B, C, N);
        double tiempoFin = omp_get_wtime();
        double tiempoSecuencial = tiempoFin - tiempoInicio;

        fprintf(fp, "%d,1,%f\n", N, tiempoSecuencial);

        // Iteramos por cada número de hilos entre 1 y el máximo y controlamos el tiempo que ha tardado en cada iteración
        for (int i = 1; i <= maxHilos; i++) {
            omp_set_num_threads(i);
            tiempoInicio = omp_get_wtime();
            multiplicaMatrices(A, B, C, N);
            tiempoFin = omp_get_wtime();
            double tiempoParalelo = tiempoFin - tiempoInicio;

            fprintf(fp, "%d,%d,%f\n", N, i, tiempoParalelo);
        }

        // Liberamos la memoria empleada por las matrices para futuro uso
        for (int i = 0; i < N; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(C);
    }

    fclose(fp);

    return 0;
}