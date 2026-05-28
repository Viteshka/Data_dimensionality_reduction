#ifndef SVG_H
#define SVG_H

#include "matrix.h"

int svg_write_pca_projection(const char *path, const Matrix *points,
                             const int *labels);
int svg_write_kmeans_projection(const char *path, const Matrix *points,
                                const int *clusters, size_t cluster_count);

#endif
