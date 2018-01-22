#include "geometry.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

void vec2_print(vec2_t v, const char *name) {
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

void vec3_to_array(vec3_t v, float arr[3]) {
    arr[0] = v.x;
    arr[1] = v.y;
    arr[2] = v.z;
}

vec3_t vec3_add(vec3_t a, vec3_t b) {
    return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3_t vec3_scale(vec3_t v, float scale) {
    return vec3_new(v.x * scale, v.y * scale, v.z * scale);
}

float vec3_length(vec3_t v) {
    return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalize(vec3_t v) {
    float length = vec3_length(v);
    return vec3_new(v.x / length, v.y / length, v.z / length);
}

float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return vec3_new(a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x);
}

void vec3_print(vec3_t v, const char *name) {
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

void vec4_to_array(vec4_t v, float arr[4]) {
    arr[0] = v.x;
    arr[1] = v.y;
    arr[2] = v.z;
    arr[3] = v.w;
}

vec4_t vec4_scale(vec4_t v, float scale) {
    return vec4_new(v.x * scale, v.y * scale, v.z * scale, v.w * scale);
}

void vec4_print(vec4_t v, const char *name) {
    printf("vec4 %s =\n", name);
    printf("    %12f    %12f    %12f    %12f\n", v.x, v.y, v.z, v.w);
}

/* mat4 related functions */

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    int i, j;
    float v_arr[4], o_arr[4];
    vec4_to_array(v, v_arr);
    for (i = 0; i < 4; i++) {
        o_arr[i] = 0.0f;
        for (j = 0; j < 4; j++) {
            o_arr[i] += m.m[i][j] * v_arr[j];
        }
    }
    return vec4_new(o_arr[0], o_arr[1], o_arr[2], o_arr[3]);
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
    int i, j, k;
    mat4_t m;
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
 * for determinant, minor, cofactor, adjoint, and inverse, see
 * 3D Math Primer for Graphics and Game Development, Chapter 6
 */

typedef struct {float m[3][3];} mat3_t;

static float mat3_determinant(const mat3_t *m) {
    return m->m[0][0] * (m->m[1][1] * m->m[2][2] - m->m[1][2] * m->m[2][1])
           + m->m[0][1] * (m->m[1][2] * m->m[2][0] - m->m[1][0] * m->m[2][2])
           + m->m[0][2] * (m->m[1][0] * m->m[2][1] - m->m[1][1] * m->m[2][0]);
}

static float mat4_minor(const mat4_t *m, int r, int c) {
    int i, j;
    mat3_t submatrix;
    assert(r >= 0 && c >= 0 && r < 4 && c < 4);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int row = (i < r) ? i : i + 1;
            int col = (j < c) ? j : j + 1;
            submatrix.m[i][j] = m->m[row][col];
        }
    }
    return mat3_determinant(&submatrix);
}

static float mat4_cofactor(const mat4_t *m, int r, int c) {
    float sign, minor;
    assert(r >= 0 && c >= 0 && r < 4 && c < 4);
    sign = ((r + c) % 2 == 0) ? 1.0f : -1.0f;
    minor = mat4_minor(m, r, c);
    return sign * minor;
}

static mat4_t mat4_adjoint(const mat4_t *m) {
    int i, j;
    mat4_t adjoint;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            adjoint.m[i][j] = mat4_cofactor(m, i, j);
        }
    }
    return adjoint;
}

mat4_t mat4_inverse_transpose(mat4_t m) {
    int i, j;
    float determinant;
    mat4_t adjoint, inverse_transpose;

    adjoint = mat4_adjoint(&m);
    /* calculate the determinant */
    determinant = 0.0f;
    for (i = 0; i < 4; i++) {
        determinant += m.m[0][i] * adjoint.m[0][i];
    }
    assert(fabs(determinant) > 1.0e-6);
    /* inverse_transpose = adjoint / determinant */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] / determinant;
        }
    }
    return inverse_transpose;
}

mat4_t mat4_inverse(mat4_t m) {
    return mat4_transpose(mat4_inverse_transpose(m));
}

mat4_t mat4_transpose(mat4_t m) {
    int i, j;
    mat4_t transpose;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            transpose.m[i][j] = m.m[j][i];
        }
    }
    return transpose;
}

void mat4_print(mat4_t m, const char *name) {
    int i, j;
    printf("mat4 %s =\n", name);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf("    %12f", m.m[i][j]);
        }
        printf("\n");
    }
}

/* transformation matrices */

mat4_t mat4_identity() {
    mat4_t m = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return m;
}

/*
 * for translation, scaling, and rotation matrices, see
 * 3D Math Primer for Graphics and Game Development, Chapter 5
 */

mat4_t mat4_translation(float dx, float dy, float dz) {
    /*
     * dx, dy, dz: the x, y, and z coordinates of a translation vector
     *
     *  1  0  0 dx
     *  0  1  0 dy
     *  0  0  1 dz
     *  0  0  0  1
     */
    mat4_t m = mat4_identity();
    m.m[0][3] = dx;
    m.m[1][3] = dy;
    m.m[2][3] = dz;
    return m;
}

mat4_t mat4_scaling(float sx, float sy, float sz) {
    /*
     * sx, sy, sz: scale factors along the x, y, and z axes, respectively
     *
     * sx  0  0  0
     *  0 sy  0  0
     *  0  0 sz  0
     *  0  0  0  1
     */
    mat4_t m = mat4_identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

mat4_t mat4_rotation(float angle, float vx, float vy, float vz) {
    /*
     * angle: the angle of rotation in radians
     * vx, vy, vz: the x, y, and z coordinates of the vector to rotate around
     *
     * nx*nx*(1-cos)+cos     ny*nx*(1-cos)-nz*sin  nz*nx*(1-cos)+ny*sin  0
     * nx*ny*(1-cos)+nz*sin  ny*ny*(1-cos)+cos     nz*ny*(1-cos)-nx*sin  0
     * nx*nz*(1-cos)-ny*sin  ny*nz*(1-cos)+nx*sin  nz*nz*(1-cos)+cos     0
     * 0                     0                     0                     1
     *
     * nx, ny, nz: normalized coordinates of the vector to rotate around
     * sin, cos: sin(angle) and cos(angle)
     */
    vec3_t n = vec3_normalize(vec3_new(vx, vy, vz));
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float one_minus_cos = 1.0f - c;
    mat4_t m = mat4_identity();

    m.m[0][0] = one_minus_cos * n.x * n.x + c;
    m.m[0][1] = one_minus_cos * n.y * n.x - s * n.z;
    m.m[0][2] = one_minus_cos * n.z * n.x + s * n.y;

    m.m[1][0] = one_minus_cos * n.x * n.y + s * n.z;
    m.m[1][1] = one_minus_cos * n.y * n.y + c;
    m.m[1][2] = one_minus_cos * n.z * n.y - s * n.x;

    m.m[2][0] = one_minus_cos * n.x * n.z - s * n.y;
    m.m[2][1] = one_minus_cos * n.y * n.z + s * n.x;
    m.m[2][2] = one_minus_cos * n.z * n.z + c;

    return m;
}

mat4_t mat4_rotation_x(float angle) {
    /*
     * angle: the angle of rotation in radians
     *
     *  1  0  0  0
     *  0  c -s  0
     *  0  s  c  0
     *  0  0  0  1
     */
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

mat4_t mat4_rotation_y(float angle) {
    /*
     * angle: the angle of rotation in radians
     *
     *  c  0  s  0
     *  0  1  0  0
     * -s  0  c  0
     *  0  0  0  1
     */
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

mat4_t mat4_rotation_z(float angle) {
    /*
     * angle: the angle of rotation in radians
     *
     *  c -s  0  0
     *  s  c  0  0
     *  0  0  1  0
     *  0  0  0  1
     */
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return m;
}

/*
 * for orthographic projection and perspective projection, see
 * http://www.songho.ca/opengl/gl_projectionmatrix.html
 */

mat4_t mat4_ortho(float left, float right, float bottom, float top,
                  float near, float far) {
    /*
     * left, right: the coordinates for the left and right clipping planes
     * bottom, top: the coordinates for the bottom and top clipping planes
     * near, far: the distances to the near and far depth clipping planes
     *
     * 2/(r-l)        0         0  -(r+l)/(r-l)
     *       0  2/(t-b)         0  -(t+b)/(t-b)
     *       0        0  -2/(f-n)  -(f+n)/(f-n)
     *       0        0         0             1
     */
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(near > 0 && far > 0 && far > near);
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2.0f / x_range;
    m.m[1][1] = 2.0f / y_range;
    m.m[2][2] = -2.0f / z_range;
    m.m[0][3] = -(left + right) / x_range;
    m.m[1][3] = -(bottom + top) / y_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

mat4_t mat4_frustum(float left, float right, float bottom, float top,
                    float near, float far) {
    /*
     * left, right: the coordinates for the left and right clipping planes
     * bottom, top: the coordinates for the bottom and top clipping planes
     * near, far: the distances to the near and far depth clipping planes
     *
     * 2n/(r-l)         0   (r+l)/(r-l)           0
     *        0  2n/(t-b)   (t+b)/(t-b)           0
     *        0         0  -(f+n)/(f-n)  -2fn/(f-n)
     *        0         0            -1           0
     */
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(near > 0 && far > 0 && far > near);
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2.0f * near / x_range;
    m.m[1][1] = 2.0f * near / y_range;
    m.m[0][2] = (left + right) / x_range;
    m.m[1][2] = (bottom + top) / y_range;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2.0f * near * far / z_range;
    m.m[3][2] = -1.0f;
    m.m[3][3] = 0.0f;
    return m;
}

mat4_t mat4_orthographic(float fovy, float aspect, float near, float far) {
    /*
     * fovy: the field of view angle in the y direction, in radians
     * aspect: the ratio of x (width) to y (height)
     * rear: the distance from the viewer to the near clipping plane
     * far: the distance from the viewer to the far clipping plane
     *
     * zoom_x       0         0             0
     *      0  zoom_y         0             0
     *      0       0  -2/(f-n)  -(f+n)/(f-n)
     *      0       0         0             1
     *
     * zoom_x: 1/(aspect*tan(fovy/2))
     * zoom_y: 1/tan(fovy/2)
     *
     * equivalent to mat4_ortho as long as the volume is symmetric
     *     float tan_half_fovy = (float)tan(fovy / 2.0f);
     *     float half_height = near * tan_half_fovy;
     *     float half_width = aspect * half_height;
     *     mat4_ortho(-half_width, half_width, -half_height, half_height,
     *                near, far);
     */
    float z_range = far - near;
    float tan_half_fovy = (float)tan(fovy / 2.0f);
    float zoom_y = 1.0f / tan_half_fovy;
    float zoom_x = zoom_y / aspect;
    mat4_t m = mat4_identity();
    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && far > near);
    m.m[0][0] = zoom_x;
    m.m[1][1] = zoom_y;
    m.m[2][2] = -2.0f / z_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

mat4_t mat4_perspective(float fovy, float aspect, float near, float far) {
    /*
     * fovy: the field of view angle in the y direction, in radians
     * aspect: the ratio of x (width) to y (height)
     * rear: the distance from the viewer to the near clipping plane
     * far: the distance from the viewer to the far clipping plane
     *
     * zoom_x       0             0           0
     *      0  zoom_y             0           0
     *      0       0  -(f+n)/(f-n)  -2fn/(f-n)
     *      0       0            -1           0
     *
     * zoom_x: 1/(aspect*tan(fovy/2))
     * zoom_y: 1/tan(fovy/2)
     *
     * equivalent to mat4_frustum as long as the frustum is symmetric
     *     float tan_half_fovy = (float)tan(fovy / 2.0f);
     *     float half_height = near * tan_half_fovy;
     *     float half_width = aspect * half_height;
     *     mat4_frustum(-half_width, half_width, -half_height, half_height,
     *                  near, far);
     */
    float z_range = far - near;
    float tan_half_fovy = (float)tan(fovy / 2.0f);
    float zoom_y = 1.0f / tan_half_fovy;
    float zoom_x = zoom_y / aspect;
    mat4_t m = mat4_identity();
    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && far > near);
    m.m[0][0] = zoom_x;
    m.m[1][1] = zoom_y;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2.0f * near * far / z_range;
    m.m[3][2] = -1.0f;
    m.m[3][3] = 0.0f;
    return m;
}

/*
 * for camera matrix and lookat matrix, see
 * http://www.songho.ca/opengl/gl_camera.html
 */

mat4_t mat4_camera(vec3_t eye, vec3_t center, vec3_t up) {
    /*
     * eye: the position of the eye point
     * center: the position of the reference point
     * up: the direction of the up vector
     *
     * x_axis.x  y_axis.x  z_axis.x  eye.x
     * x_axis.y  y_axis.y  z_axis.y  eye.y
     * x_axis.z  y_axis.z  z_axis.z  eye.z
     *        0         0         0      1
     *
     * z_axis: normalize(eye-center), the negative front vector
     * x_axis: normalize(cross(up,z_axis)), the right vector
     * y_axis: cross(z_axis,x_axis), the up vector
     */
    vec3_t z_axis = vec3_normalize(vec3_sub(eye, center));  /* right-handed */
    vec3_t x_axis = vec3_normalize(vec3_cross(up, z_axis));
    vec3_t y_axis = vec3_cross(z_axis, x_axis);
    mat4_t m = mat4_identity();

    m.m[0][0] = x_axis.x;
    m.m[1][0] = x_axis.y;
    m.m[2][0] = x_axis.z;

    m.m[0][1] = y_axis.x;
    m.m[1][1] = y_axis.y;
    m.m[2][1] = y_axis.z;

    m.m[0][2] = z_axis.x;
    m.m[1][2] = z_axis.y;
    m.m[2][2] = z_axis.z;

    m.m[0][3] = eye.x;
    m.m[1][3] = eye.y;
    m.m[2][3] = eye.z;

    return m;
}

mat4_t mat4_lookat(vec3_t eye, vec3_t center, vec3_t up) {
    /*
     * eye: the position of the eye point
     * center: the position of the reference point
     * up: the direction of the up vector
     *
     * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
     * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
     * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
     *        0         0         0                 1
     *
     * z_axis: normalize(eye-center), the negative front vector
     * x_axis: normalize(cross(up,z_axis)), the right vector
     * y_axis: cross(z_axis,x_axis), the up vector
     *
     * equivalent to mat4_inverse(mat4_camera(eye,center,up))
     *     camera = translation*rotation
     *     lookat = inverse(camera)
     *            = inverse(rotation)*inverse(translation)
     *            = transpose(rotation)*inverse(translation)
     *
     * note that the rotation matrix is an orthogonal matrix, which means
     *     transpose(rotation) == inverse(rotation)
     */
    vec3_t z_axis = vec3_normalize(vec3_sub(eye, center));  /* right-handed */
    vec3_t x_axis = vec3_normalize(vec3_cross(up, z_axis));
    vec3_t y_axis = vec3_cross(z_axis, x_axis);
    mat4_t m = mat4_identity();

    m.m[0][0] = x_axis.x;
    m.m[0][1] = x_axis.y;
    m.m[0][2] = x_axis.z;

    m.m[1][0] = y_axis.x;
    m.m[1][1] = y_axis.y;
    m.m[1][2] = y_axis.z;

    m.m[2][0] = z_axis.x;
    m.m[2][1] = z_axis.y;
    m.m[2][2] = z_axis.z;

    m.m[0][3] = -vec3_dot(x_axis, eye);
    m.m[1][3] = -vec3_dot(y_axis, eye);
    m.m[2][3] = -vec3_dot(z_axis, eye);

    return m;
}

/*
 * for viewport matrix, see
 * http://www.songho.ca/opengl/gl_transform.html
 */

mat4_t mat4_viewport(int x, int y, int width, int height) {
    /*
     * x, y: the lower left corner of the viewport rectangle
     * width, height: the width and height of the viewport
     *
     * w/2    0        0    x+w/2
     *   0  h/2        0    y+h/2
     *   0    0  (f-n)/2  (f+n)/2
     *   0    0        0        1
     */
    const float near = 0.0f;
    const float far = 1.0f;
    mat4_t m = mat4_identity();
    assert(width > 0 && height > 0);
    m.m[0][0] = width / 2.0f;
    m.m[0][3] = x + width / 2.0f;
    m.m[1][1] = height / 2.0f;
    m.m[1][3] = y + height / 2.0f;
    m.m[2][2] = (far - near) / 2.0f;
    m.m[2][3] = (far + near) / 2.0f;
    return m;
}
