#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
	clock_t start;
	start = clock();
	int a1 = atoi(argv[1]);
	int a2 = atoi(argv[2]);
	double** mas = (double**)malloc(a1 * sizeof(double*));
	for (int i = 0; i < a1; i++)
		mas[i] = (double*)calloc(a1, sizeof(double));
	

	double** mas_old = (double**)malloc(a1 * sizeof(double*));
	for (int i = 0; i < a1; i++)
		mas_old[i] = (double*)calloc(a1, sizeof(double));

	mas[0][0] = 10;
	mas[a1-1][0] = 20;//1 -строки 2- стлб
	mas[0][a1-1] = 20;
	mas[a1-1][a1-1] = 30;

	for (int i = 1; i < a1-1; i++) {
		mas[0][i] = mas[0][i-1]+ (mas[0][a1-1] - mas[0][0] )/ a1;
		mas[i][0] = mas[i-1][0]+   (mas[a1-1][0] - mas[0][0]) / a1;
		mas[a1-1][i] = mas[a1 - 1][i-1]+  (mas[a1-1][a1-1] - mas[a1-1][0]) / a1;
		mas[i][a1-1] = mas[i-1][a1 - 1] + (mas[a1-1][a1-1] - mas[0][a1-1]) / a1;
	}


	int iter = 0;
	double err = argc;  //10^(-6)

	for (int i = 0; i < a1; i++) 
		for (int j = 0; j < a1; j++)
			mas_old[i][j] = mas[i][j];

#pragma acc enter data copyin(err, argv, mas[0:argv][0:argv], mas_old[0:argv][0:argv])
	while ((err > argc || iter ==1) && iter < a2) {
		if (iter%100==0){
			err = 0;
#pragma acc update device(err)
		}
		iter += 1;
		err = 0;

#pragma acc data present(mas_old, mas, err)
#pragma acc parallel loop independent collapse(1)
		for (int i = 1; i < a1 - 1; i++) {
			for (int j = 1; j < a1 - 1; j++) {
				mas[i][j] = (mas_old[i - 1][j] + mas_old[i][j - 1] + mas_old[i + 1][j] + mas_old[i][j + 1]) / 4;
				err = fmax(err, fabs(mas[i][j] - mas_old[i][j]));


			}
			if (iter % 100 == 0) {
#pragma acc update host(err)
			}
		}

		

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
	for (int i = 0; i < a1; i++)
		free(mas_old[i]);
	free(mas_old);
	for (int i = 0; i < a1; i++)
		free(mas[i]);
	free(mas);
#pragma acc exit data copyin(err, argv, mas[0:argv][0:argv], mas_old[0:argv][0:argv])
	return 0;
}


