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
 * 3D Math Primer for Graphics and Game Development, Chapter 6
 */

typedef struct {float m[3][3];} mat3_t;

static float mat3_determinant(mat3_t m) {
    float a = +m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
    float b = -m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    float c = +m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return a + b + c;
}

static float mat4_minor(mat4_t m, int r, int c) {
    mat3_t submatrix;
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int row = (i < r) ? i : i + 1;
            int col = (j < c) ? j : j + 1;
            submatrix.m[i][j] = m.m[row][col];
        }
    }
    return mat3_determinant(submatrix);
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
    /* calculate the determinant */
    determinant = 0;
    for (i = 0; i < 4; i++) {
        determinant += m.m[0][i] * adjoint.m[0][i];
    }
    assert(fabs(determinant) > 1e-5);
    inv_determinant = 1 / determinant;
    /* inverse_transpose = adjoint / determinant */
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

/* transformation matrices */

/*
 * for translation, scaling, and rotation matrices, see
 * 3D Math Primer for Graphics and Game Development, Chapter 5
 */

/*
 * dx, dy, dz: the x, y, and z coordinates of a translation vector
 *
 *  1  0  0 dx
 *  0  1  0 dy
 *  0  0  1 dz
 *  0  0  0  1
 *
 * reference:
 *     http://docs.gl/gl2/glTranslate
 */
mat4_t mat4_translate(float dx, float dy, float dz) {
    mat4_t m = mat4_identity();
    m.m[0][3] = dx;
    m.m[1][3] = dy;
    m.m[2][3] = dz;
    return m;
}

/*
 * sx, sy, sz: scale factors along the x, y, and z axes, respectively
 *
 * sx  0  0  0
 *  0 sy  0  0
 *  0  0 sz  0
 *  0  0  0  1
 *
 * reference:
 *     http://docs.gl/gl2/glScale
 */
mat4_t mat4_scale(float sx, float sy, float sz) {
    mat4_t m = mat4_identity();
    assert(sx != 0 && sy != 0 && sz != 0);
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 * vx, vy, vz: the x, y, and z coordinates of a vector, respectively
 *
 * nx*nx*(1-cos)+cos     ny*nx*(1-cos)-sin*nz  nz*nx*(1-cos)+sin*ny  0
 * nx*ny*(1-cos)+sin*nz  ny*ny*(1-cos)+cos     nz*ny*(1-cos)-sin*nx  0
 * nx*nz*(1-cos)-sin*ny  ny*nz*(1-cos)+sin*nx  nz*nz*(1-cos)+cos     0
 * 0                     0                     0                     1
 *
 * nx, ny, nz: normalized coordinates of the vector, respectively
 * sin, cos: sin(angle) and cos(angle)
 *
 * reference:
 *     http://docs.gl/gl2/glRotate
 */
mat4_t mat4_rotate(float angle, float vx, float vy, float vz) {
    vec3_t n = vec3_normalize(vec3_new(vx, vy, vz));
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float one_minus_cos = 1 - c;
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

/*
 * angle: the angle of rotation, in radians
 *
 *  1  0  0  0
 *  0  c -s  0
 *  0  s  c  0
 *  0  0  0  1
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_anglestoaxes.html
 *     https://github.com/g-truc/glm/blob/master/glm/gtx/rotate_vector.inl
 */
mat4_t mat4_rotate_x(float angle) {
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c  0  s  0
 *  0  1  0  0
 * -s  0  c  0
 *  0  0  0  1
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_anglestoaxes.html
 *     https://github.com/g-truc/glm/blob/master/glm/gtx/rotate_vector.inl
 */
mat4_t mat4_rotate_y(float angle) {
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c -s  0  0
 *  s  c  0  0
 *  0  0  1  0
 *  0  0  0  1
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_anglestoaxes.html
 *     https://github.com/g-truc/glm/blob/master/glm/gtx/rotate_vector.inl
 */
mat4_t mat4_rotate_z(float angle) {
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
 * eye: the position of the eye point
 * target: the position of the look-at target
 * up: the direction of the up vector
 *
 * x_axis.x  y_axis.x  z_axis.x  eye.x
 * x_axis.y  y_axis.y  z_axis.y  eye.y
 * x_axis.z  y_axis.z  z_axis.z  eye.z
 *        0         0         0      1
 *
 * z_axis: normalize(eye-target), the negative front vector
 * x_axis: normalize(cross(up,z_axis)), the right vector
 * y_axis: cross(z_axis,x_axis), the up vector
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_camera.html
 *     https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
 */
mat4_t mat4_camera(vec3_t eye, vec3_t target, vec3_t up) {
    vec3_t z_axis = vec3_normalize(vec3_sub(eye, target));  /* right-handed */
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

/*
 * eye: the position of the eye point
 * target: the position of the look-at target
 * up: the direction of the up vector
 *
 * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
 * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
 * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
 *        0         0         0                 1
 *
 * z_axis: normalize(eye-target), the negative front vector
 * x_axis: normalize(cross(up,z_axis)), the right vector
 * y_axis: cross(z_axis,x_axis), the up vector
 *
 * equivalent to mat4_inverse(mat4_camera(eye,target,up))
 *     camera = translation*rotation
 *     lookat = inverse(camera)
 *            = inverse(translation*rotation)
 *            = inverse(rotation)*inverse(translation)
 *            = transpose(rotation)*inverse(translation)
 *
 * note that the rotation matrix is orthogonal, which means
 *     transpose(rotation) == inverse(rotation)
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_camera.html
 *     https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
 */
mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up) {
    vec3_t z_axis = vec3_normalize(vec3_sub(eye, target));  /* right-handed */
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
 * left, right: the coordinates for the left and right clipping planes
 * bottom, top: the coordinates for the bottom and top clipping planes
 * near, far: the distances to the near and far depth clipping planes
 *
 * 2/(r-l)        0         0  -(r+l)/(r-l)
 *       0  2/(t-b)         0  -(t+b)/(t-b)
 *       0        0  -2/(f-n)  -(f+n)/(f-n)
 *       0        0         0             1
 *
 * reference:
 *     http://docs.gl/gl2/glOrtho
 *     http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
mat4_t mat4_ortho(float left, float right, float bottom, float top,
                  float near, float far) {
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(near > 0 && far > 0);
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2 / x_range;
    m.m[1][1] = 2 / y_range;
    m.m[2][2] = -2 / z_range;
    m.m[0][3] = -(left + right) / x_range;
    m.m[1][3] = -(bottom + top) / y_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

/*
 * left, right: the coordinates for the left and right clipping planes
 * bottom, top: the coordinates for the bottom and top clipping planes
 * near, far: the distances to the near and far depth clipping planes
 *
 * 2n/(r-l)         0   (r+l)/(r-l)           0
 *        0  2n/(t-b)   (t+b)/(t-b)           0
 *        0         0  -(f+n)/(f-n)  -2fn/(f-n)
 *        0         0            -1           0
 *
 * reference:
 *     http://docs.gl/gl2/glFrustum
 *     http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
mat4_t mat4_frustum(float left, float right, float bottom, float top,
                    float near, float far) {
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(near > 0 && far > 0);
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2 * near / x_range;
    m.m[1][1] = 2 * near / y_range;
    m.m[0][2] = (left + right) / x_range;
    m.m[1][2] = (bottom + top) / y_range;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2 * near * far / z_range;
    m.m[3][2] = -1;
    m.m[3][3] = 0;
    return m;
}

/*
 * xmag, ymag: the horizontal and vertical magnifications of the view
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/xmag       0         0             0
 *      0  1/ymag         0             0
 *      0       0  -2/(f-n)  -(f+n)/(f-n)
 *      0       0         0             1
 *
 * equivalent to mat4_ortho as long as the volume is symmetric
 *     mat4_ortho(-xmag, xmag, -ymag, ymag, near, far);
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_projectionmatrix.html
 *     https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
 */
mat4_t mat4_orthographic(float xmag, float ymag, float near, float far) {
    mat4_t m = mat4_identity();
    float z_range = far - near;
    assert(xmag > 0 && ymag > 0);
    assert(near > 0 && far > 0 && z_range > 0);
    m.m[0][0] = 1 / xmag;
    m.m[1][1] = 1 / ymag;
    m.m[2][2] = -2 / z_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

/*
 * fovy: the field of view angle in the y direction, in radians
 * aspect: the aspect ratio, defined as width divided by height
 * near, far: the distances to the near and far depth clipping planes
 *
 * x_scale        0             0           0
 *      0   y_scale             0           0
 *      0         0  -(f+n)/(f-n)  -2fn/(f-n)
 *      0         0            -1           0
 *
 * x_scale: 1/(aspect*tan(fovy/2))
 * y_scale: 1/tan(fovy/2)
 *
 * equivalent to mat4_frustum as long as the frustum is symmetric
 *     float tan_half_fovy = (float)tan(fovy / 2);
 *     float half_height = near * tan_half_fovy;
 *     float half_width = aspect * half_height;
 *     mat4_frustum(-half_width, half_width, -half_height, half_height,
 *                  near, far);
 *
 * reference:
 *     http://www.songho.ca/opengl/gl_projectionmatrix.html
 *     https://github.com/g-truc/glm/blob/master/glm/gtx/rotate_vector.inl
 */
mat4_t mat4_perspective(float fovy, float aspect, float near, float far) {
    float z_range = far - near;
    float tan_half_fovy = (float)tan(fovy / 2);
    float y_scale = 1 / tan_half_fovy;
    float x_scale = y_scale / aspect;
    mat4_t m = mat4_identity();
    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && far > near);
    m.m[0][0] = x_scale;
    m.m[1][1] = y_scale;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2 * near * far / z_range;
    m.m[3][2] = -1;
    m.m[3][3] = 0;
    return m;
}
