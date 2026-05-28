#include "my_libs/dataset.h"
#include "my_libs/pca.h"
#include "my_libs/svg.h"

#include <stdio.h>

static void print_projection(const PCAResult *pca, const int *labels,
                             size_t limit)
{
    for (size_t row = 0; row < limit && row < pca->projection.rows; row++) {
        printf("Wine %lu: PC1=%7.3f PC2=%7.3f class=%d\n",
               (unsigned long)(row + 1),
               pca->projection.values[row * pca->projection.cols],
               pca->projection.values[row * pca->projection.cols + 1],
               labels[row]);
    }
}

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : "data/wine/wine.data";
    Dataset dataset = {{0, 0, NULL}, NULL};
    PCAResult pca = {{0, 0, NULL}, {0, 0, NULL}, {0.0, 0.0}, {0.0, 0.0}};
    size_t class_count[3] = {0, 0, 0};

    if (!dataset_load_wine(path, &dataset)) {
        printf("Cannot read Wine dataset: %s\n", path);
        return 1;
    }

    for (size_t row = 0; row < dataset.features.rows; row++) {
        class_count[dataset.labels[row] - 1]++;
    }

    printf("Wine dataset: %lu wines, %lu features\n",
           (unsigned long)dataset.features.rows,
           (unsigned long)dataset.features.cols);
    printf("Classes: 1=%lu, 2=%lu, 3=%lu\n",
           (unsigned long)class_count[0], (unsigned long)class_count[1],
           (unsigned long)class_count[2]);

    if (!dataset_standardize(&dataset.features)) {
        printf("Cannot standardize dataset\n");
        dataset_destroy(&dataset);
        return 1;
    }

    if (!pca_fit_transform(&dataset.features, &pca)) {
        printf("Cannot calculate PCA projection\n");
        dataset_destroy(&dataset);
        return 1;
    }

    printf("\nPCA explained variance:\n");
    printf("PC1: %.2f%%\n", pca.explained_variance[0] * 100.0);
    printf("PC2: %.2f%%\n", pca.explained_variance[1] * 100.0);
    printf("Together: %.2f%%\n",
           (pca.explained_variance[0] + pca.explained_variance[1]) * 100.0);
    printf("\nFirst 5 projected wines:\n");
    print_projection(&pca, dataset.labels, 5);
    printf("\nLabels are printed for checking, not used in PCA.\n");

    if (!svg_write_pca_projection("results/pca_projection.svg",
                                  &pca.projection, dataset.labels)) {
        printf("Cannot save PCA SVG\n");
        pca_destroy(&pca);
        dataset_destroy(&dataset);
        return 1;
    }

    printf("SVG saved: results/pca_projection.svg\n");
    pca_destroy(&pca);
    dataset_destroy(&dataset);
    return 0;
}
