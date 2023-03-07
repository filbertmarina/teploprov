#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


int SIZE; //size of matrix
int iter_max;
double err_max;
int main(int argc, char* argv[]) {
	clock_t start;
	start = clock();

	scanf ("%lf %d %d", &err_max, &SIZE, &iter_max);
	double** mas = (double**)malloc(SIZE * sizeof(double*));
	for (int i = 0; i < SIZE; i++)
		mas[i] = (double*)calloc(SIZE, sizeof(double));
	

	double** mas_old = (double**)malloc(SIZE * sizeof(double*));
	for (int i = 0; i < SIZE; i++)
		mas_old[i] = (double*)calloc(SIZE, sizeof(double));

	mas[0][0] = 10;
	mas[SIZE-1][0] = 20;//1 -строки 2- стлб
	mas[0][SIZE-1] = 20;
	mas[SIZE-1][SIZE-1] = 30;

	for (int i = 1; i < SIZE-1; i++) {
		mas[0][i] = mas[0][i-1]+ (mas[0][SIZE-1] - mas[0][0] )/ SIZE;
		mas[i][0] = mas[i-1][0]+   (mas[SIZE-1][0] - mas[0][0]) / SIZE;
		mas[SIZE-1][i] = mas[SIZE - 1][i-1]+  (mas[SIZE-1][SIZE-1] - mas[SIZE-1][0]) / SIZE;
		mas[i][SIZE-1] = mas[i-1][SIZE - 1] + (mas[SIZE-1][SIZE-1] - mas[0][SIZE-1]) / SIZE;
	}


	int iter = 0;
	double err = err_max*10;  //10^(-6)

	for (int i = 0; i < SIZE; i++) 
		for (int j = 0; j < SIZE; j++)
			mas_old[i][j] = mas[i][j];

#pragma acc data copyin(err, SIZE, mas[0:SIZE][0:SIZE], mas_old[0:SIZE][0:SIZE])
	while ((err > err_max || iter ==1) && iter < iter_max) {
		iter += 1;
		err = 0;
#pragma acc parallel loop async 
		for (int i = 1; i < SIZE - 1; i++) 
			for (int j = 1; j < SIZE - 1; j++) {
				mas[i][j] = (mas_old[i - 1][j] + mas_old[i][j - 1] + mas_old[i + 1][j] + mas_old[i][j + 1]) / 4;
				err = fmax(err, fabs(mas[i][j] - mas_old[i][j]));

			}
#pragma acc update self(mas[1:SIZE-1][1:SIZE-1]) async
#pragma acc wait
		

		double **m= mas;
		mas = mas_old;
		mas_old = m;
		
		if (iter % 100 == 0 || iter == 1) {
			printf("%d  %lf", iter, err);
			printf("\n");
		}

	}
	double t = (double)(clock() - start) / CLOCKS_PER_SEC;
	printf(" time: %lf\n", t);
	for (int i = 0; i < SIZE; i++)
		free(mas_old[i]);
	free(mas_old);
	for (int i = 0; i < SIZE; i++)
		free(mas[i]);
	free(mas);

	return 0;
}


