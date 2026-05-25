#include "matrix.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *input;
    Matrix matrix = {0, 0, NULL};
    Matrix transposed = {0, 0, NULL};
    double *first_row = NULL;
    double *ones = NULL;
    double *result = NULL;
    unsigned long input_rows;
    unsigned long input_cols;
    size_t rows;
    size_t cols;
    
    input = fopen("input.txt", "r");
    if (input == NULL) {
        printf("Cannot open input.txt\n");
        return 1;
    }

    if (fscanf(input, "%lu %lu", &input_rows, &input_cols) != 2) {
        printf("Cannot read matrix size\n");
        fclose(input);
        return 1;
    }
    rows = (size_t)input_rows;
    cols = (size_t)input_cols;
   
    matrix = matrix_create(rows, cols);
    if (!matrix_is_valid(&matrix)) {
        printf("Cannot create matrix\n");
        fclose(input);
        return 1;
    }

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            double value;

            if (fscanf(input, "%lf", &value) != 1) {
                printf("Cannot read matrix value\n");
                fclose(input);
                matrix_destroy(&matrix);
                return 1;
            }
            matrix_set(&matrix, row, col, value);
        }
    }
    fclose(input);

    printf("Matrix from file:\n");
    matrix_print(&matrix);

    transposed = matrix_transpose(&matrix);
    printf("\nTransposed matrix:\n");
    matrix_print(&transposed);

    first_row = malloc(cols * sizeof(double));
    ones = malloc(cols * sizeof(double));
    result = malloc(rows * sizeof(double));
    if (first_row == NULL || ones == NULL || result == NULL) {
        printf("Cannot allocate vectors\n");
        free(first_row);
        free(ones);
        free(result);
        matrix_destroy(&transposed);
        matrix_destroy(&matrix);
        return 1;
    }

    for (size_t col = 0; col < cols; col++) {
        matrix_get(&matrix, 0, col, &first_row[col]);
        ones[col] = 1.0;
    }

    printf("First row as vector:\n");
    vector_print(first_row, cols);
    printf("Vector norm: %.2f\n", vector_norm(first_row, cols));
    vector_normalize(first_row, cols);
    printf("Normalized vector:\n");
    vector_print(first_row, cols);

    matrix_vector_multiply(&matrix, ones, cols, result);
    printf("Matrix multiplied by [1 ... 1]:\n");
    vector_print(result, rows);

    free(first_row);
    free(ones);
    free(result);
    matrix_destroy(&transposed);
    matrix_destroy(&matrix);
    return 0;
}
