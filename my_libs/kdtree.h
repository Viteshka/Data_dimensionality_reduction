#ifndef KDTREE_H
#define KDTREE_H

#include "matrix.h"

typedef struct KDNode {
    size_t point_index; //номер строки в матрице PCA-проекции
    int axis;           //ось сравнения: 0 - PC1, 1 - PC2
    struct KDNode *left;
    struct KDNode *right;
} KDNode;

//вставка точки в KD-дерево
int kdtree_insert(KDNode **root, const Matrix *points, size_t point_index,
                  int depth);

//построение KD-дерева по матрице точек
KDNode *kdtree_build(const Matrix *points);

//поиск ближайшего соседа через KD-дерево
size_t kdtree_nearest(const KDNode *root, const Matrix *points,
                      size_t query_index, size_t excluded_index);

//поиск всех точек, попадающих в радиус
size_t kdtree_range_query(const KDNode *root, const Matrix *points,
                          size_t query_index, double radius,
                          size_t *result_indices, size_t result_capacity);

//удаление KD-дерева
void kdtree_destroy(KDNode *root);

#endif
