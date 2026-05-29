#ifndef BRUTE_FORCE_H
#define BRUTE_FORCE_H

#include "matrix.h"

//поиск ближайшего соседа полным перебором
size_t brute_force_nearest(const Matrix *points, size_t query_index,
                           size_t excluded_index);

//поиск всех точек, попадающих в радиус
size_t brute_force_range_query(const Matrix *points, size_t query_index,
                               double radius, size_t *result_indices,
                               size_t result_capacity);

#endif
