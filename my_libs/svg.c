#include "svg.h"

#include <stdio.h>

#define SVG_WIDTH 800.0
#define SVG_HEIGHT 600.0
#define SVG_MARGIN 60.0
#define SVG_POINT_RADIUS 4.0

// Chooses a point color by the original Wine class.
static const char *class_color(int label)
{
    if (label == 1) {
        return "#d1495b";
    }
    if (label == 2) {
        return "#0077b6";
    }
    if (label == 3) {
        return "#2a9d8f";
    }
    return "#555555";
}

// Chooses a point color by the k-means cluster number.
static const char *cluster_color(int cluster)
{
    if (cluster == 0) {
        return "#d1495b";
    }
    if (cluster == 1) {
        return "#0077b6";
    }
    if (cluster == 2) {
        return "#2a9d8f";
    }
    return "#555555";
}

// Scales one coordinate from the data range to the SVG range.
static double scale_value(double value, double min_value, double max_value,
                          double start, double end)
{
    if (max_value == min_value) {
        return (start + end) / 2.0;
    }
    return start + (value - min_value) * (end - start) /
        (max_value - min_value);
}

// Finds minimal and maximal PC1 and PC2 values.
static void find_bounds(const Matrix *points, double *min_x, double *max_x,
                        double *min_y, double *max_y)
{
    *min_x = points->values[0];
    *max_x = points->values[0];
    *min_y = points->values[1];
    *max_y = points->values[1];

    for (size_t row = 1; row < points->rows; row++) {
        double x = points->values[row * points->cols];
        double y = points->values[row * points->cols + 1];

        if (x < *min_x) {
            *min_x = x;
        }
        if (x > *max_x) {
            *max_x = x;
        }
        if (y < *min_y) {
            *min_y = y;
        }
        if (y > *max_y) {
            *max_y = y;
        }
    }
}

// Extends the data bounds so centroids are also visible.
static void include_centroid_bounds(const Matrix *centroids, double *min_x,
                                    double *max_x, double *min_y,
                                    double *max_y)
{
    for (size_t row = 0; row < centroids->rows; row++) {
        double x = centroids->values[row * centroids->cols];
        double y = centroids->values[row * centroids->cols + 1];

        if (x < *min_x) {
            *min_x = x;
        }
        if (x > *max_x) {
            *max_x = x;
        }
        if (y < *min_y) {
            *min_y = y;
        }
        if (y > *max_y) {
            *max_y = y;
        }
    }
}

// Writes the legend for original Wine classes.
static void write_legend(FILE *file)
{
    const double x = SVG_WIDTH - 170.0;
    const double y = 70.0;

    fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"14\" "
                  "font-family=\"Arial\">Wine classes</text>\n", x, y);
    for (int label = 1; label <= 3; label++) {
        double item_y = y + 25.0 * label;
        fprintf(file, "<circle cx=\"%.0f\" cy=\"%.0f\" r=\"5\" "
                      "fill=\"%s\" />\n", x + 8.0, item_y - 4.0,
                      class_color(label));
        fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"13\" "
                      "font-family=\"Arial\">Class %d</text>\n",
                      x + 22.0, item_y, label);
    }
}

// Writes the legend for k-means clusters.
static void write_cluster_legend(FILE *file, size_t cluster_count)
{
    const double x = SVG_WIDTH - 170.0;
    const double y = 70.0;

    fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"14\" "
                  "font-family=\"Arial\">K-means clusters</text>\n", x, y);
    for (size_t cluster = 0; cluster < cluster_count; cluster++) {
        double item_y = y + 25.0 * (double)(cluster + 1);
        fprintf(file, "<circle cx=\"%.0f\" cy=\"%.0f\" r=\"5\" "
                      "fill=\"%s\" />\n", x + 8.0, item_y - 4.0,
                      cluster_color((int)cluster));
        fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"13\" "
                      "font-family=\"Arial\">Cluster %lu</text>\n",
                      x + 22.0, item_y, (unsigned long)(cluster + 1));
    }
    fprintf(file, "<rect x=\"%.0f\" y=\"%.0f\" width=\"10\" height=\"10\" "
                  "fill=\"white\" stroke=\"#111\" stroke-width=\"2\" />\n",
                  x + 3.0, y + 25.0 * (double)(cluster_count + 1) - 11.0);
    fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"13\" "
                  "font-family=\"Arial\">Centroid</text>\n",
                  x + 22.0, y + 25.0 * (double)(cluster_count + 1));
}

// Saves the PCA projection colored by the original Wine classes.
int svg_write_pca_projection(const char *path, const Matrix *points,
                             const int *labels)
{
    FILE *file;
    double min_x;
    double max_x;
    double min_y;
    double max_y;

    if (path == NULL || !matrix_is_valid(points) || points->cols < 2 ||
        labels == NULL) {
        return 0;
    }

    find_bounds(points, &min_x, &max_x, &min_y, &max_y);

    file = fopen(path, "w");
    if (file == NULL) {
        return 0;
    }

    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                  "width=\"%.0f\" height=\"%.0f\" "
                  "viewBox=\"0 0 %.0f %.0f\">\n",
                  SVG_WIDTH, SVG_HEIGHT, SVG_WIDTH, SVG_HEIGHT);
    fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"white\" />\n");
    fprintf(file, "<text x=\"%.0f\" y=\"35\" font-size=\"20\" "
                  "font-family=\"Arial\">Wine PCA projection</text>\n",
                  SVG_MARGIN);
    fprintf(file, "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
                  "stroke=\"#333\" />\n",
                  SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN,
                  SVG_WIDTH - SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN);
    fprintf(file, "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
                  "stroke=\"#333\" />\n",
                  SVG_MARGIN, SVG_MARGIN,
                  SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN);
    fprintf(file, "<text x=\"%.0f\" y=\"%.0f\" font-size=\"13\" "
                  "font-family=\"Arial\">PC1</text>\n",
                  SVG_WIDTH - SVG_MARGIN - 25.0, SVG_HEIGHT - 25.0);
    fprintf(file, "<text x=\"20\" y=\"%.0f\" font-size=\"13\" "
                  "font-family=\"Arial\">PC2</text>\n", SVG_MARGIN - 20.0);

    for (size_t row = 0; row < points->rows; row++) {
        double x = points->values[row * points->cols];
        double y = points->values[row * points->cols + 1];
        double svg_x = scale_value(x, min_x, max_x, SVG_MARGIN,
                                   SVG_WIDTH - SVG_MARGIN);
        double svg_y = scale_value(y, min_y, max_y, SVG_HEIGHT - SVG_MARGIN,
                                   SVG_MARGIN);

        fprintf(file, "<circle class=\"point\" cx=\"%.2f\" cy=\"%.2f\" "
                      "r=\"%.1f\" fill=\"%s\" opacity=\"0.85\">"
                      "<title>Wine %lu, class %d</title></circle>\n",
                      svg_x, svg_y, SVG_POINT_RADIUS, class_color(labels[row]),
                      (unsigned long)(row + 1), labels[row]);
    }

    write_legend(file);
    fprintf(file, "</svg>\n");
    fclose(file);
    return 1;
}

// Saves the PCA projection colored by k-means clusters.
int svg_write_kmeans_projection(const char *path, const Matrix *points,
                                const int *clusters, const Matrix *centroids,
                                size_t cluster_count)
{
    FILE *file;
    double min_x;
    double max_x;
    double min_y;
    double max_y;

    if (path == NULL || !matrix_is_valid(points) || points->cols < 2 ||
        clusters == NULL || !matrix_is_valid(centroids) ||
        centroids->cols < 2 || cluster_count == 0) {
        return 0;
    }

    find_bounds(points, &min_x, &max_x, &min_y, &max_y);
    include_centroid_bounds(centroids, &min_x, &max_x, &min_y, &max_y);

    file = fopen(path, "w");
    if (file == NULL) {
        return 0;
    }

    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                  "width=\"%.0f\" height=\"%.0f\" "
                  "viewBox=\"0 0 %.0f %.0f\">\n",
                  SVG_WIDTH, SVG_HEIGHT, SVG_WIDTH, SVG_HEIGHT);
    fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"white\" />\n");
    fprintf(file, "<text x=\"%.0f\" y=\"35\" font-size=\"20\" "
                  "font-family=\"Arial\">Wine k-means clusters</text>\n",
                  SVG_MARGIN);
    fprintf(file, "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
                  "stroke=\"#333\" />\n",
                  SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN,
                  SVG_WIDTH - SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN);
    fprintf(file, "<line x1=\"%.0f\" y1=\"%.0f\" x2=\"%.0f\" y2=\"%.0f\" "
                  "stroke=\"#333\" />\n",
                  SVG_MARGIN, SVG_MARGIN,
                  SVG_MARGIN, SVG_HEIGHT - SVG_MARGIN);

    for (size_t row = 0; row < points->rows; row++) {
        double x = points->values[row * points->cols];
        double y = points->values[row * points->cols + 1];
        double svg_x = scale_value(x, min_x, max_x, SVG_MARGIN,
                                   SVG_WIDTH - SVG_MARGIN);
        double svg_y = scale_value(y, min_y, max_y, SVG_HEIGHT - SVG_MARGIN,
                                   SVG_MARGIN);

        fprintf(file, "<circle class=\"point\" cx=\"%.2f\" cy=\"%.2f\" "
                      "r=\"%.1f\" fill=\"%s\" opacity=\"0.85\">"
                      "<title>Wine %lu, cluster %d</title></circle>\n",
                      svg_x, svg_y, SVG_POINT_RADIUS,
                      cluster_color(clusters[row]),
                      (unsigned long)(row + 1), clusters[row] + 1);
    }

    for (size_t cluster = 0; cluster < centroids->rows; cluster++) {
        double x = centroids->values[cluster * centroids->cols];
        double y = centroids->values[cluster * centroids->cols + 1];
        double svg_x = scale_value(x, min_x, max_x, SVG_MARGIN,
                                   SVG_WIDTH - SVG_MARGIN);
        double svg_y = scale_value(y, min_y, max_y, SVG_HEIGHT - SVG_MARGIN,
                                   SVG_MARGIN);

        fprintf(file, "<rect class=\"centroid\" x=\"%.2f\" y=\"%.2f\" "
                      "width=\"14\" height=\"14\" fill=\"%s\" "
                      "stroke=\"#111\" stroke-width=\"2\">"
                      "<title>Centroid %lu</title></rect>\n",
                      svg_x - 7.0, svg_y - 7.0, cluster_color((int)cluster),
                      (unsigned long)(cluster + 1));
    }

    write_cluster_legend(file, cluster_count);
    fprintf(file, "</svg>\n");
    fclose(file);
    return 1;
}
