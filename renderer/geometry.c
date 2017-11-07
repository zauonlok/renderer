#include <math.h>
#include "geometry.h"

/* constructors */

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

vec4f_t vec4f_new(float x, float y, float z, float w) {
    vec4f_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
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

/* vec3f stuff */

vec3f_t vec3f_add(vec3f_t a, vec3f_t b) {
    return vec3f_new(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3f_t vec3f_sub(vec3f_t a, vec3f_t b) {
    return vec3f_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

float vec3f_length(vec3f_t v) {
    return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3f_t vec3f_normalize(vec3f_t v) {
    float length = vec3f_length(v);
    return vec3f_new(v.x / length, v.y / length, v.z / length);
}

float vec3f_dot(vec3f_t a, vec3f_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3f_t vec3f_cross(vec3f_t a, vec3f_t b) {
    return vec3f_new(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

/* mat4f stuff */

vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v) {
    float v_arr[4], o_arr[4];
    int i, j;
    v_arr[0] = v.x;
    v_arr[1] = v.y;
    v_arr[2] = v.z;
    v_arr[3] = v.w;
    for (i = 0; i < 4; i++) {
        o_arr[i] = 0.0f;
        for (j = 0; j < 4; j++) {
            o_arr[i] += m.m[i][j] * v_arr[j];
        }
    }
    return vec4f_new(o_arr[0], o_arr[1], o_arr[2], o_arr[2]);
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
