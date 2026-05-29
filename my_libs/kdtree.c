#include "kdtree.h"

#include <stdlib.h>

//возвращает координату точки из матрицы PCA-проекции
static double point_value(const Matrix *points, size_t row, int axis)
{
    return points->values[row * points->cols + (size_t)axis];
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

//вставка точки с заданным номером в KD-дерево
int kdtree_insert(KDNode **root, const Matrix *points, size_t point_index,
                  int depth)
{
    int axis = depth % 2; //ось сравнения зависит от глубины узла

    if (root == NULL || !matrix_is_valid(points) || points->cols < 2) {
        return 0;
    }

    if (*root == NULL) {
        *root = malloc(sizeof(KDNode)); //если узла нет, создаём новый
        if (*root == NULL) {
            return 0;
        }
        (*root)->point_index = point_index;
        (*root)->axis = axis;
        (*root)->left = NULL;
        (*root)->right = NULL;
        return 1;
    }

    if (point_value(points, point_index, (*root)->axis)
        < point_value(points, (*root)->point_index, (*root)->axis)) {
        return kdtree_insert(&(*root)->left, points, point_index,
                             depth + 1); //идём в левое поддерево
    }
    return kdtree_insert(&(*root)->right, points, point_index,
                         depth + 1); //идём в правое поддерево
}

//строит KD-дерево по матрице PCA-проекции
KDNode *kdtree_build(const Matrix *points)
{
    KDNode *root = NULL;

    if (!matrix_is_valid(points) || points->cols < 2) {
        return root;
    }

    for (size_t row = 0; row < points->rows; row++) {
        if (!kdtree_insert(&root, points, row, 0)) { //вставляем каждую строку матрицы
            kdtree_destroy(root);
            return NULL;
        }
    }
    return root;
}

//рекурсивная функция поиска ближайшего соседа
static void nearest_recursive(const KDNode *node, const Matrix *points,
                              size_t query_index, size_t excluded_index,
                              size_t *best_index, double *best_distance)
{
    const KDNode *first;
    const KDNode *second;
    double distance;
    double axis_difference;

    if (node == NULL) {
        return;
    }

    distance = distance_squared(points, node->point_index, query_index);
    if (node->point_index != excluded_index
        && (*best_index == (size_t)-1 || distance < *best_distance)) {
        *best_index = node->point_index;
        *best_distance = distance;
    }

    axis_difference = point_value(points, query_index, node->axis)
        - point_value(points, node->point_index, node->axis); //разность по оси узла
    first = (axis_difference < 0.0) ? node->left : node->right;
    second = (axis_difference < 0.0) ? node->right : node->left;

    nearest_recursive(first, points, query_index, excluded_index, best_index,
                      best_distance); //сначала проверяем более близкое поддерево
    if (*best_index == (size_t)-1
        || axis_difference * axis_difference <= *best_distance) {
        nearest_recursive(second, points, query_index, excluded_index,
                          best_index, best_distance); //проверяем второе поддерево, если там может быть точка ближе
    }
}

//публичная функция поиска ближайшего соседа
size_t kdtree_nearest(const KDNode *root, const Matrix *points,
                      size_t query_index, size_t excluded_index)
{
    size_t best_index = (size_t)-1;
    double best_distance = 0.0;

    if (!matrix_is_valid(points) || points->cols < 2
        || query_index >= points->rows) {
        return best_index; //при некорректных данных возвращаем некорректный индекс
    }

    nearest_recursive(root, points, query_index, excluded_index, &best_index,
                      &best_distance);
    return best_index;
}

//рекурсивный поиск всех точек внутри радиуса
static size_t range_recursive(const KDNode *node, const Matrix *points,
                              size_t query_index, double radius_squared,
                              size_t *result_indices,
                              size_t result_capacity, size_t found)
{
    double axis_difference;

    if (node == NULL) {
        return found;
    }

    if (distance_squared(points, node->point_index, query_index)
        <= radius_squared) {
        if (found < result_capacity) {
            result_indices[found] = node->point_index;
        }
        found++;
    }

    axis_difference = point_value(points, query_index, node->axis)
        - point_value(points, node->point_index, node->axis);
    if (axis_difference <= 0.0
        || axis_difference * axis_difference <= radius_squared) {
        found = range_recursive(node->left, points, query_index,
                                radius_squared, result_indices,
                                result_capacity, found);
    }
    if (axis_difference >= 0.0
        || axis_difference * axis_difference <= radius_squared) {
        found = range_recursive(node->right, points, query_index,
                                radius_squared, result_indices,
                                result_capacity, found);
    }
    return found;
}

//возвращает количество найденных точек и записывает их индексы в result_indices
size_t kdtree_range_query(const KDNode *root, const Matrix *points,
                          size_t query_index, double radius,
                          size_t *result_indices, size_t result_capacity)
{
    if (!matrix_is_valid(points) || points->cols < 2
        || query_index >= points->rows || result_indices == NULL) {
        return 0;
    }
    return range_recursive(root, points, query_index, radius * radius,
                           result_indices, result_capacity, 0);
}

//удаление KD-дерева
void kdtree_destroy(KDNode *root)
{
    if (root == NULL) {
        return;
    }
    kdtree_destroy(root->left);
    kdtree_destroy(root->right);
    free(root);
}
