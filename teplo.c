#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
int SIZE ;
double err_max ;
int iter_max ;

int main() {
	clock_t start;
	start = clock();
	scanf("%lf %d %d", &err_max, &SIZE, &iter_max);//задаем параметры (точность, размер сетки, количество итераций) через параметры командной строки.

	//динамическое выделение памяти для массивов  
	double** mas = (double**)malloc(SIZE * sizeof(double*));
	for (int i = 0; i < SIZE; i++)
		mas[i] = (double*)calloc(SIZE, sizeof(double));
	
		
	double** mas_old = (double**)malloc(SIZE * sizeof(double*));
	for (int i = 0; i < SIZE; i++)
		mas_old[i] = (double*)calloc(SIZE, sizeof(double));

	//Значение в углах
	mas[0][0] = 10;
	mas[SIZE-1][0] = 20;
	mas[0][SIZE-1] = 20;
	mas[SIZE-1][SIZE-1] = 30;


	//запись краевых значений матрицы
	for (int i = 1; i < SIZE-1; i++) {
		mas[0][i] = mas[0][i-1]+ (mas[0][SIZE-1] - mas[0][0] )/ SIZE;
		mas[i][0] = mas[i-1][0]+   (mas[SIZE-1][0] - mas[0][0]) / SIZE;
		mas[SIZE-1][i] = mas[SIZE - 1][i-1]+  (mas[SIZE-1][SIZE-1] - mas[SIZE-1][0]) / SIZE;
		mas[i][SIZE-1] = mas[i-1][SIZE - 1] + (mas[SIZE-1][SIZE-1] - mas[0][SIZE-1]) / SIZE;
	}


	int iter = 0;
	double err = err_max*10;  //10^(-6)

	//присвоение mas_old - mas
	for (int i = 0; i < SIZE; i++) 
		for (int j = 0; j < SIZE; j++)
			mas_old[i][j] = mas[i][j];


	//главный цикл программы для вычисления значений матрицы, времени и ошибки 

#pragma acc data copyin(err, SIZE, mas[0:SIZE][0:SIZE], mas_old[0:SIZE][0:SIZE])
	while ((err > err_max || iter == 1) && iter < iter_max) {//пока не достигнута минимальная ошибка или максимальное допустимое значение итераций
		if (iter % 100 == 0) {
			err = 0;
#pragma acc update device(err)
		}

		//err = 0;
#pragma acc data present(mas_old, mas, err)
#pragma acc parallel loop independent collapse(2) reduction(max:err) async(1)
		for (int i = 1; i < SIZE - 1; i++) {
			for (int j = 1; j < SIZE - 1; j++) {
				mas[i][j] = (mas_old[i - 1][j] + mas_old[i][j - 1] + mas_old[i + 1][j] + mas_old[i][j + 1]) / 4; //подсчет значения матрицы
				err = fmax(err, fabs(mas[i][j] - mas_old[i][j]));//сравнение с максимальной ошибкой на всей матрице 

		
			}
		}
#pragma acc wait(1)
		if (iter % 100 == 0) {
#pragma acc update host(err)
		}

		double **m= mas;
		mas = mas_old; //перезапись mas в mas_old  
		mas_old = m;

		iter += 1;
		if (iter % 100 == 0 && iter != 1) {// при достижении 100 итераций, вывод времени и ошибки на данной итерации 
			double t = (double)(clock() - start) / CLOCKS_PER_SEC;
			printf(" time: %lf\n", t);
			printf("%d  %lf", iter, err);
			printf("\n");
		}

	}
	//освобождение памяти  
	for (int i = 0; i < SIZE; i++)
		free(mas_old[i]);
	free(mas_old);
	for (int i = 0; i < SIZE; i++)
		free(mas[i]);
	free(mas);

	//Вывод общего времени 
	double t = (double)(clock() - start) / CLOCKS_PER_SEC;
	printf(" time: %lf\n", t);
	return 0;
}


