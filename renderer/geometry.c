#include <math.h>
#include "geometry.h"

vec2i_t vec2i_new(int x, int y) {
    vec2i_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec3i_t vec3i_new(int x, int y, int z) {
    vec3i_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec2f_t vec2f_new(float x, float y) {
    vec2f_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec3f_t vec3f_new(float x, float y, float z) {
    vec3f_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec2i_t vec2i_add(vec2i_t v0, vec2i_t v1) {
    vec2i_t v;
    v.x = v0.x + v1.x;
    v.y = v0.y + v1.y;
    return v;
}

vec2i_t vec2i_sub(vec2i_t v0, vec2i_t v1) {
    vec2i_t v;
    v.x = v0.x - v1.x;
    v.y = v0.y - v1.y;
    return v;
}

vec3f_t vec3f_normalize(vec3f_t v) {
    float length = (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return vec3f_new(v.x / length, v.y / length, v.z / length);
}

vec3f_t vec3f_sub(vec3f_t a, vec3f_t b) {
    return vec3f_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3f_t vec3f_cross(vec3f_t a, vec3f_t b) {
    vec3f_t result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

float vec3f_dot(vec3f_t a, vec3f_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b) {
    mat4f_t m;
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (k = 0; k < 4; k++) {
                sum += a.m[i][k] * b.m[k][j];
            }
            m.m[i][j] = sum;
        }
    }
    return m;
}

mat4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v) {
    vec4f_t output;
    int i;
    for (i = 0; i < 4; i++) {
        float e = 0.0f;
        e += m.m[i][0] * v.x;
        e += m.m[i][1] * v.y;
        e += m.m[i][2] * v.z;
        e += m.m[i][3] * v.w;

        output.x = e;
        /* error here */

    }
}

mat4f_t mat4f_identity() {
    mat4f_t m;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    return m;
}
