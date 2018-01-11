#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "geometry.h"

/* vec2 stuff */

vec2_t vec2_new(float x, float y) {
    vec2_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
    return vec2_new(a.x + b.x, a.y + b.y);
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return vec2_new(a.x - b.x, a.y - b.y);
}

/* vec3f stuff */

vec3f_t vec3f_new(float x, float y, float z) {
    vec3f_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

void vec3f_to_array(vec3f_t v, float arr[3]) {
    arr[0] = v.x;
    arr[1] = v.y;
    arr[2] = v.z;
}

vec3f_t vec3f_from_vec4f(vec4f_t v) {
    return vec3f_new(v.x, v.y, v.z);
}

vec3f_t vec3f_scale(vec3f_t v, float scale) {
    return vec3f_new(v.x * scale, v.y * scale, v.z * scale);
}

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
    return vec3f_new(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

/* vec4f stuff */

vec4f_t vec4f_new(float x, float y, float z, float w) {
    vec4f_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

void vec4f_to_array(vec4f_t v, float arr[4]) {
    arr[0] = v.x;
    arr[1] = v.y;
    arr[2] = v.z;
    arr[3] = v.w;
}

vec4f_t vec4f_from_vec3f(vec3f_t v, float w) {
    return vec4f_new(v.x, v.y, v.z, w);
}

vec4f_t vec4f_scale(vec4f_t v, float scale) {
    return vec4f_new(v.x * scale, v.y * scale, v.z * scale, v.w * scale);
}

/* mat4f stuff */

mat4f_t mat4f_identity() {
    int i, j;
    mat4f_t m;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    return m;
}

vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v) {
    int i, j;
    float v_arr[4], o_arr[4];
    vec4f_to_array(v, v_arr);
    for (i = 0; i < 4; i++) {
        o_arr[i] = 0.0f;
        for (j = 0; j < 4; j++) {
            o_arr[i] += m.m[i][j] * v_arr[j];
        }
    }
    return vec4f_new(o_arr[0], o_arr[1], o_arr[2], o_arr[3]);
}

mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b) {
    int i, j, k;
    mat4f_t m;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m.m[i][j] = 0.0f;
            for (k = 0; k < 4; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}

/*
 * for determinant, minor, cofactor and adjoint, see
 * 3D Math Primer for Graphics and Game Development, Chapter 6
 */

typedef struct {float m[3][3];} mat3f_t;

static float mat3f_determinant(mat3f_t *m) {
    return m->m[0][0] * (m->m[1][1] * m->m[2][2] - m->m[1][2] * m->m[2][1])
           + m->m[0][1] * (m->m[1][2] * m->m[2][0] - m->m[1][0] * m->m[2][2])
           + m->m[0][2] * (m->m[1][0] * m->m[2][1] - m->m[1][1] * m->m[2][0]);
}

static float mat4f_minor(mat4f_t *m, int r, int c) {
    int i, j;
    mat3f_t sub_mat;
    assert(r >= 0 && c >= 0 && r < 4 && c < 4);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int row = (i < r) ? i : i + 1;
            int col = (j < c) ? j : j + 1;
            sub_mat.m[i][j] = m->m[row][col];
        }
    }
    return mat3f_determinant(&sub_mat);
}

static float mat4f_cofactor(mat4f_t *m, int r, int c) {
    float minor, sign;
    assert(r >= 0 && c >= 0 && r < 4 && c < 4);
    minor = mat4f_minor(m, r, c);
    sign = ((r + c) % 2 == 0) ? 1.0f : -1.0f;
    return sign * minor;
}

static mat4f_t mat4f_adjoint(mat4f_t *m) {
    int i, j;
    mat4f_t adjoint;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            adjoint.m[i][j] = mat4f_cofactor(m, i, j);
        }
    }
    return adjoint;
}

mat4f_t mat4f_invert_transpose(mat4f_t m) {
    int i, j;
    float determinant;
    mat4f_t adjoint, invert_transpose;

    adjoint = mat4f_adjoint(&m);
    /* calculate the determinant */
    determinant = 0.0f;
    for (i = 0; i < 4; i++) {
        determinant += m.m[0][i] * adjoint.m[0][i];
    }
    assert(fabs(determinant) > 1.0e-6);
    /* invert_transpose = adjoint / determinant */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            invert_transpose.m[i][j] = adjoint.m[i][j] / determinant;
        }
    }
    return invert_transpose;
}

mat4f_t mat4f_inverse(mat4f_t m) {
    return mat4f_transpose(mat4f_invert_transpose(m));
}

mat4f_t mat4f_transpose(mat4f_t m) {
    int i, j;
    mat4f_t transpose;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            transpose.m[i][j] = m.m[j][i];
        }
    }
    return transpose;
}
