#include "pca.h"

#include "vector.h"

#include <math.h>
#include <stdlib.h>

#define PCA_COMPONENTS 2
#define PCA_ITERATIONS 100
#define PCA_TOLERANCE 0.0000001
//построение ковариационной матрицы
Matrix pca_covariance(const Matrix *data)
{
    Matrix covariance = {0, 0, NULL};
    if (!matrix_is_valid(data) || data->rows < 2) {
        return covariance;
    }

    covariance = matrix_create(data->cols, data->cols);
    if (!matrix_is_valid(&covariance)) {
        return covariance;
    }

    for (size_t left = 0; left < data->cols; left++) {
        for (size_t right = 0; right < data->cols; right++) {
            double sum = 0.0;

            for (size_t row = 0; row < data->rows; row++) {
                sum += data->values[row * data->cols + left]
                    * data->values[row * data->cols + right];
            }
            // covariance[left][right] = sum(data[row][left] * data[row][right]) / (rows - 1)
            covariance.values[left * covariance.cols + right] =
                sum / (double)(data->rows - 1);
        }
    }
    return covariance;
}
//возвращает максимальное отличие между двумя векторами
static double component_difference(const double *left, const double *right,
                                   size_t size)
{
    double difference = 0.0;


    for (size_t index = 0; index<size; index++) {
        double current = left[index] - right[index];
        current = (current>=0) ? current : (-current);
        if (current>difference) {
            difference = current;
        }
    }
    return difference;
}
//поиск собственного вектора ковариационной матрицы
static int power_iteration(const Matrix *covariance, double *component,
                           double *eigenvalue)
{
    double *next; //временный вектор для хранения результата умножения
   
    next = malloc(covariance->cols * sizeof(double));
    if (next == NULL) {
        return 0;
    }

    for (size_t column = 0; column<covariance->cols; column++) {
        component[column] = 1.0;
    }
    vector_normalize(component, covariance->cols);

    for (size_t iteration = 0; iteration < PCA_ITERATIONS; iteration++) {
        if (!matrix_vector_multiply(covariance, component,
                                    covariance->cols, next)
            || !vector_normalize(next, covariance->cols)) { //умножаем вектор на матрицу, 
            free(next);                                     //сокращаем его длину до 1 и проверяем
            return 0;
        }

        if (component_difference(component, next, covariance->cols)){ //если на итерации разница между прошлым 
            for (size_t column = 0; column < covariance->cols; column++) { //и новым вектором меньше
                component[column] = next[column];                          //погрешности завершаем
            }
            break;
        }
        for (size_t column = 0; column < covariance->cols; column++) {
            component[column] = next[column];
        }
    }

    matrix_vector_multiply(covariance, component, covariance->cols, next);
    *eigenvalue = vector_dot(component, next, covariance->cols);
    free(next);
    return 1;
}
//вычитает влияние главной компоненты
static void deflate(Matrix *covariance, const double *component,
                    double eigenvalue)
{
    for (size_t row = 0; row<covariance->rows; row++) {
        for (size_t column = 0; column<covariance->cols; column++) {
            covariance->values[row * covariance->cols + column] -=
                eigenvalue * component[row] * component[column];
        }
    }
}
//сумма элементов главной диагонали
static double matrix_trace(const Matrix *matrix)
{
    double trace = 0.0;
    
    for (size_t index = 0; index<matrix->rows; index++) {
        trace += matrix->values[index * matrix->cols + index];
    }
    return trace;
}

//перевод данных в координаты компонент - проекция данных на плоскость 
static void project_data(const Matrix *data, PCAResult *result)
{
   
    for (size_t row = 0; row < data->rows; row++) {
        for (size_t component = 0; component < PCA_COMPONENTS; component++) {
            double coordinate = 0.0;

            for (size_t column = 0; column < data->cols; column++) {
                coordinate += data->values[row * data->cols + column]
                    * result->components.values[component * result->components.cols + column];
            }
            result->projection.values[row * result->projection.cols + component] = coordinate;
        }
    }
}

//Выполнение PCA
int pca_fit_transform(const Matrix *data, PCAResult *result)
{
    Matrix covariance = {0, 0, NULL};
    double total_variance; //общая дисперсия данных


    if (!matrix_is_valid(data) || data->cols < PCA_COMPONENTS
        || result == NULL) {
        return 0;
    }

    covariance = pca_covariance(data);
    result->components = matrix_create(PCA_COMPONENTS, data->cols);
    result->projection = matrix_create(data->rows, PCA_COMPONENTS);
    if (!matrix_is_valid(&covariance)
        || !matrix_is_valid(&result->components)
        || !matrix_is_valid(&result->projection)) { //проверка корректности матриц
        matrix_destroy(&covariance);
        pca_destroy(result);
        return 0;
    }

    total_variance = matrix_trace(&covariance);
    for (size_t component = 0; component < PCA_COMPONENTS; component++) {
        double *direction =
            &result->components.values[component * result->components.cols]; //указатель на текущую компоненту

        if (!power_iteration(&covariance, direction,
                             &result->eigenvalues[component])) { //поиск компоненты
            matrix_destroy(&covariance);
            pca_destroy(result);
            return 0;
        }
        result->explained_variance[component] =
            result->eigenvalues[component] / total_variance; //вычисление доли дисперсии для компоненты
        deflate(&covariance, direction, result->eigenvalues[component]); //вычитаем влияние компоненты
    }

    project_data(data, result);  //делаем проекцию данных на плоскость заданную компонентами
    matrix_destroy(&covariance); //удаляем ковареционную матрицу
    return 1;
}

//Удаление объекта структуры
void pca_destroy(PCAResult *result)
{
    if (result == NULL) {
        return;
    }
    matrix_destroy(&result->components);
    matrix_destroy(&result->projection);
    result->eigenvalues[0] = 0.0;
    result->eigenvalues[1] = 0.0;
    result->explained_variance[0] = 0.0;
    result->explained_variance[1] = 0.0;
}
