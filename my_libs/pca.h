#ifndef PCA_H
#define PCA_H

#include "matrix.h"

typedef struct {
    Matrix components; //Матрица главных компонент из двух строк
    Matrix projection; // Матрица из проекций на плоскость, заданную векторами components
    double eigenvalues[2]; //собственные значения ковариационных матриц
    double explained_variance[2]; //доля дисперсии для каждой компоненты
} PCAResult;

Matrix pca_covariance(const Matrix *data);
int pca_fit_transform(const Matrix *data, PCAResult *result);
void pca_destroy(PCAResult *result);

#endif
