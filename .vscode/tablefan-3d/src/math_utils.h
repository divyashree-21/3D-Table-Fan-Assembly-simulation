#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float m[4][4];
} Mat4;

Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_subtract(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, float scale);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Mat4 mat4_identity(void);
Mat4 mat4_translate(float x, float y, float z);
Mat4 mat4_rotate(float angle, Vec3 axis);
Mat4 mat4_scale(float x, float y, float z);
Mat4 mat4_multiply(Mat4 a, Mat4 b);

#endif // MATH_UTILS_H