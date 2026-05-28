#include "kmeans.h"

#include <stdlib.h>
//квадрат расстояния от точки до цендродида
static double squared_distance(const Matrix *points, size_t row,
                               const Matrix *centroids, size_t cluster)
{
    double distance = 0.0;

    for (size_t col = 0; col < points->cols; col++) {
        double difference = points->values[row * points->cols + col]
            - centroids->values[cluster * centroids->cols + col];
        distance += difference * difference;
    }
    return distance;
}
//задание начальных центроид в случайных точках
static void initialize_centroids(const Matrix *points, Matrix *centroids)
{
    for (size_t cluster = 0; cluster<centroids->rows; cluster++) {
        size_t source_row = cluster * points->rows / centroids->rows; //номер строки - начального центроида
                                                                     //начальная центроида берётся из разных мест датасета
        for (size_t col = 0; col<points->cols; col++) {
            centroids->values[cluster * centroids->cols + col] =
                points->values[source_row * points->cols + col];  //запись центроиды
        }
    }
}
//назначение каждой точки близжайшему центроиду
static int assign_points(const Matrix *points, KMeansResult *result)
{
    int changed = 0;

    for (size_t row = 0; row<points->rows; row++) {
        int best_cluster = 0;
        double best_distance =
            squared_distance(points, row, &result->centroids, 0);

        for (size_t cluster = 1; cluster<result->clusters; cluster++) {
            double distance =
                squared_distance(points, row, &result->centroids, cluster);

            if (distance < best_distance) {
                best_distance = distance;
                best_cluster = (int)cluster;
            }
        }

        if (result->labels[row] != best_cluster) {
            result->labels[row] = best_cluster;
            changed = 1;
        }
    }
    return changed;
}
//пересчёт центроиды по точкам кластеров
static int update_centroids(const Matrix *points, KMeansResult *result)
{
    double *sums;
    size_t *counts;

    sums = calloc(result->clusters * points->cols, sizeof(double));
    counts = calloc(result->clusters, sizeof(size_t));
    if (sums == NULL || counts == NULL) {
        free(sums);
        free(counts);
        return 0;
    }

    for (size_t row = 0; row < points->rows; row++) {
        int cluster = result->labels[row];
        counts[cluster]++;

        for (size_t col = 0; col < points->cols; col++) {
            sums[(size_t)cluster * points->cols + col] +=
                points->values[row * points->cols + col]; 
        }
    }

    for (size_t cluster = 0; cluster < result->clusters; cluster++) {
        if (counts[cluster] == 0) {
            continue;
        }

        for (size_t col = 0; col < points->cols; col++) {
            result->centroids.values[cluster * result->centroids.cols + col] =
                sums[cluster * points->cols + col] / (double)counts[cluster]; //centroid = sum(cluster_points_coordinate)
        }                                                                     // /count(cluster_points)
    }

    free(sums);
    free(counts);
    return 1;
}

//выполнение алгоритма k-means
int kmeans_fit(const Matrix *points, size_t clusters, size_t max_iterations,
               KMeansResult *result)
{
    if (!matrix_is_valid(points) || clusters == 0 || clusters > points->rows
        || max_iterations == 0 || result == NULL) {
        return 0;
    }

    result->centroids = matrix_create(clusters, points->cols);
    result->labels = malloc(points->rows * sizeof(int));
    result->clusters = clusters;
    result->iterations = 0;
    if (!matrix_is_valid(&result->centroids) || result->labels == NULL) {
        kmeans_destroy(result);
        return 0;
    }

    for (size_t row = 0; row < points->rows; row++) {
        result->labels[row] = -1;
    }

    initialize_centroids(points, &result->centroids);

    for (size_t iteration = 0; iteration < max_iterations; iteration++) {
        int changed = assign_points(points, result);

        result->iterations = iteration + 1;
        if (!update_centroids(points, result)) {
            kmeans_destroy(result);
            return 0;
        }
        if (!changed) {
            break;
        }
    }
    return 1;
}
// удаление структуры
void kmeans_destroy(KMeansResult *result)
{
    if (result == NULL) {
        return;
    }

    matrix_destroy(&result->centroids);
    free(result->labels);
    result->labels = NULL;
    result->clusters = 0;
    result->iterations = 0;
}
