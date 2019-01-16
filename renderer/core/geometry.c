#include "geometry.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* vec2 related functions */

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

vec2_t vec2_mul(vec2_t v, float factor) {
    return vec2_new(v.x * factor, v.y * factor);
}

vec2_t vec2_div(vec2_t v, float divisor) {
    return vec2_mul(v, 1 / divisor);
}

void vec2_print(const char *name, vec2_t v) {
    printf("vec2 %s =\n", name);
    printf("    %12f    %12f\n", v.x, v.y);
}

/* vec3 related functions */

vec3_t vec3_new(float x, float y, float z) {
    vec3_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec3_t vec3_from_vec4(vec4_t v) {
    return vec3_new(v.x, v.y, v.z);
}

vec3_t vec3_add(vec3_t a, vec3_t b) {
    return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3_t vec3_mul(vec3_t v, float factor) {
    return vec3_new(v.x * factor, v.y * factor, v.z * factor);
}

vec3_t vec3_div(vec3_t v, float divisor) {
    return vec3_mul(v, 1 / divisor);
}

float vec3_length(vec3_t v) {
    return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalize(vec3_t v) {
    return vec3_div(v, vec3_length(v));
}

float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    float x = a.y * b.z - a.z * b.y;
    float y = a.z * b.x - a.x * b.z;
    float z = a.x * b.y - a.y * b.x;
    return vec3_new(x, y, z);
}

vec3_t vec3_saturate(vec3_t v) {
    float x = (v.x < 0) ? 0 : ((v.x > 1) ? 1 : v.x);
    float y = (v.y < 0) ? 0 : ((v.y > 1) ? 1 : v.y);
    float z = (v.z < 0) ? 0 : ((v.z > 1) ? 1 : v.z);
    return vec3_new(x, y, z);
}

vec3_t vec3_modulate(vec3_t a, vec3_t b) {
    return vec3_new(a.x * b.x, a.y * b.y, a.z * b.z);
}

void vec3_print(const char *name, vec3_t v) {
    printf("vec3 %s =\n", name);
    printf("    %12f    %12f    %12f\n", v.x, v.y, v.z);
}

/* vec4 related functions */

vec4_t vec4_new(float x, float y, float z, float w) {
    vec4_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

vec4_t vec4_from_vec3(vec3_t v, float w) {
    return vec4_new(v.x, v.y, v.z, w);
}

vec4_t vec4_saturate(vec4_t v) {
    float x = (v.x < 0) ? 0 : ((v.x > 1) ? 1 : v.x);
    float y = (v.y < 0) ? 0 : ((v.y > 1) ? 1 : v.y);
    float z = (v.z < 0) ? 0 : ((v.z > 1) ? 1 : v.z);
    float w = (v.w < 0) ? 0 : ((v.w > 1) ? 1 : v.w);
    return vec4_new(x, y, z, w);
}

vec4_t vec4_modulate(vec4_t a, vec4_t b) {
    return vec4_new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

void vec4_print(const char *name, vec4_t v) {
    printf("vec4 %s =\n", name);
    printf("    %12f    %12f    %12f    %12f\n", v.x, v.y, v.z, v.w);
}

/* mat3 related functions */

mat3_t mat3_identity(void) {
    mat3_t m = {{
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
    }};
    return m;
}

vec3_t mat3_mul_vec3(mat3_t m, vec3_t v) {
    float product[3];
    int i;
    for (i = 0; i < 3; i++) {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        product[i] = a + b + c;
    }
    return vec3_new(product[0], product[1], product[2]);
}

mat3_t mat3_mul_mat3(mat3_t a, mat3_t b) {
    mat3_t m;
    int i, j, k;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            m.m[i][j] = 0;
            for (k = 0; k < 3; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}

mat3_t mat3_inverse(mat3_t m) {
    return mat3_transpose(mat3_inverse_transpose(m));
}

mat3_t mat3_transpose(mat3_t m) {
    mat3_t transpose;
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            transpose.m[i][j] = m.m[j][i];
        }
    }
    return transpose;
}

static float mat3_determinant(mat3_t m) {
    float a = +m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
    float b = -m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    float c = +m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return a + b + c;
}

static mat3_t mat3_adjoint(mat3_t m) {
    mat3_t adjoint;
    adjoint.m[0][0] = +(m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
    adjoint.m[0][1] = -(m.m[1][0] * m.m[2][2] - m.m[2][0] * m.m[1][2]);
    adjoint.m[0][2] = +(m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]);
    adjoint.m[1][0] = -(m.m[0][1] * m.m[2][2] - m.m[2][1] * m.m[0][2]);
    adjoint.m[1][1] = +(m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]);
    adjoint.m[1][2] = -(m.m[0][0] * m.m[2][1] - m.m[2][0] * m.m[0][1]);
    adjoint.m[2][0] = +(m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2]);
    adjoint.m[2][1] = -(m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
    adjoint.m[2][2] = +(m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1]);
    return adjoint;
}

mat3_t mat3_inverse_transpose(mat3_t m) {
    mat3_t adjoint, inverse_transpose;
    float determinant, inv_determinant;
    int i, j;

    adjoint = mat3_adjoint(m);
    determinant = mat3_determinant(m);
    assert(fabs(determinant) > EPSILON);
    inv_determinant = 1 / determinant;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] * inv_determinant;
        }
    }
    return inverse_transpose;
}

void mat3_print(const char *name, mat3_t m) {
    int i, j;
    printf("mat3 %s =\n", name);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("    %12f", m.m[i][j]);
        }
        printf("\n");
    }
}

/* mat4 related functions */

mat4_t mat4_identity(void) {
    mat4_t m = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};
    return m;
}

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    float product[4];
    int i;
    for (i = 0; i < 4; i++) {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        float d = m.m[i][3] * v.w;
        product[i] = a + b + c + d;
    }
    return vec4_new(product[0], product[1], product[2], product[3]);
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
    mat4_t m;
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m.m[i][j] = 0;
            for (k = 0; k < 4; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}

mat4_t mat4_inverse(mat4_t m) {
    return mat4_transpose(mat4_inverse_transpose(m));
}

mat4_t mat4_transpose(mat4_t m) {
    mat4_t transpose;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            transpose.m[i][j] = m.m[j][i];
        }
    }
    return transpose;
}

/*
 * for determinant, minor, cofactor, adjoint, and inverse, see
 * 3D Math Primer for Graphics and Game Development, 2nd Edition, Chapter 6
 */

static float mat4_minor(mat4_t m, int r, int c) {
    mat3_t cut_down;
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int row = (i < r) ? i : i + 1;
            int col = (j < c) ? j : j + 1;
            cut_down.m[i][j] = m.m[row][col];
        }
    }
    return mat3_determinant(cut_down);
}

static float mat4_cofactor(mat4_t m, int r, int c) {
    float sign = ((r + c) % 2 == 0) ? 1.0f : -1.0f;
    float minor = mat4_minor(m, r, c);
    return sign * minor;
}

static mat4_t mat4_adjoint(mat4_t m) {
    mat4_t adjoint;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            adjoint.m[i][j] = mat4_cofactor(m, i, j);
        }
    }
    return adjoint;
}

mat4_t mat4_inverse_transpose(mat4_t m) {
    mat4_t adjoint, inverse_transpose;
    float determinant, inv_determinant;
    int i, j;

    adjoint = mat4_adjoint(m);
    determinant = 0;
    for (i = 0; i < 4; i++) {
        determinant += m.m[0][i] * adjoint.m[0][i];
    }
    assert(fabs(determinant) > EPSILON);
    inv_determinant = 1 / determinant;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] * inv_determinant;
        }
    }
    return inverse_transpose;
}

void mat4_print(const char *name, mat4_t m) {
    int i, j;
    printf("mat4 %s =\n", name);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf("    %12f", m.m[i][j]);
        }
        printf("\n");
    }
}
