#include "brute_force.h"

//возвращает координату точки из матрицы PCA-проекции
static double point_value(const Matrix *points, size_t row, size_t col)
{
    return points->values[row * points->cols + col];
}

//считает квадрат евклидова расстояния между двумя точками
static double distance_squared(const Matrix *points, size_t left, size_t right)
{
    double dx = point_value(points, left, 0)
        - point_value(points, right, 0); //разность координат по PC1
    double dy = point_value(points, left, 1)
        - point_value(points, right, 1); //разность координат по PC2

    return dx * dx + dy * dy;
}

//поиск ближайшей точки полным перебором
size_t brute_force_nearest(const Matrix *points, size_t query_index,
                           size_t excluded_index)
{
    size_t best_index = (size_t)-1; //индекс ближайшей найденной точки
    double best_distance = 0.0;     //квадрат лучшего найденного расстояния

    if (!matrix_is_valid(points) || points->cols < 2
        || query_index >= points->rows) { //проверяем корректность входных данных
        return best_index;
    }

    for (size_t row = 0; row < points->rows; row++) {
        double distance;

        if (row == excluded_index) { //пропускаем точку, которую нельзя учитывать
            continue;
        }

        distance = distance_squared(points, row, query_index);
        if (best_index == (size_t)-1 || distance < best_distance) {
            best_index = row;
            best_distance = distance;
        }
    }
    return best_index;
}

//поиск всех точек внутри заданного радиуса полным перебором
size_t brute_force_range_query(const Matrix *points, size_t query_index,
                               double radius, size_t *result_indices,
                               size_t result_capacity)
{
    size_t found = 0; //количество найденных точек
    double radius_squared = radius * radius; //сравниваем квадраты расстояний

    if (!matrix_is_valid(points) || points->cols < 2
        || query_index >= points->rows || result_indices == NULL) { //проверяем входные данные
        return 0;
    }

    for (size_t row = 0; row < points->rows; row++) {
        if (distance_squared(points, row, query_index) <= radius_squared) {
            if (found < result_capacity) {
                result_indices[found] = row; //записываем индекс, если хватает места
            }
            found++;
        }
    }
    return found;
}
