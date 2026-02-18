#include <math.h>
#include "math_utils.h"

void normalize(float *vector) {
    float length = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
    if (length > 0) {
        vector[0] /= length;
        vector[1] /= length;
        vector[2] /= length;
    }
}

void cross_product(const float *a, const float *b, float *result) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

void matrix_multiply(const float *a, const float *b, float *result) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] +
                                a[i * 4 + 1] * b[1 * 4 + j] +
                                a[i * 4 + 2] * b[2 * 4 + j] +
                                a[i * 4 + 3] * b[3 * 4 + j];
        }
    }
}

void translate(float *matrix, float x, float y, float z) {
    matrix[12] += x;
    matrix[13] += y;
    matrix[14] += z;
}

void rotate(float *matrix, float angle, float x, float y, float z) {
    float rad = angle * (M_PI / 180.0f);
    float c = cos(rad);
    float s = sin(rad);
    float one_c = 1.0f - c;

    matrix[0] = x * x * one_c + c;
    matrix[1] = y * x * one_c + z * s;
    matrix[2] = x * z * one_c - y * s;
    matrix[4] = x * y * one_c - z * s;
    matrix[5] = y * y * one_c + c;
    matrix[6] = y * z * one_c + x * s;
    matrix[8] = x * z * one_c + y * s;
    matrix[9] = y * z * one_c - x * s;
    matrix[10] = z * z * one_c + c;
    matrix[15] = 1.0f;
}