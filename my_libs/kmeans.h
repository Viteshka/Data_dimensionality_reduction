#ifndef KMEANS_H
#define KMEANS_H

#include "matrix.h"

#include <stddef.h>

typedef struct {
    Matrix centroids; //матрица центроидов
    int *labels;      //номера кластеров для всех точек по индексам
    size_t clusters;  //колличество кластеров
    size_t iterations;//колличество итераций
} KMeansResult;

int kmeans_fit(const Matrix *points, size_t clusters, size_t max_iterations,
               KMeansResult *result);
void kmeans_destroy(KMeansResult *result);

#endif
