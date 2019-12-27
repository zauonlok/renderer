#ifndef MATHS_H
#define MATHS_H

typedef struct {float x, y;} vec2_t;
typedef struct {float x, y, z;} vec3_t;
typedef struct {float x, y, z, w;} vec4_t;
typedef struct {float x, y, z, w;} quat_t;
typedef struct {float m[3][3];} mat3_t;
typedef struct {float m[4][4];} mat4_t;

/* float related functions */
float float_min(float a, float b);
float float_max(float a, float b);
float float_lerp(float a, float b, float t);
float float_clamp(float f, float min, float max);
float float_saturate(float f);
float float_from_uchar(unsigned char value);
unsigned char float_to_uchar(float value);
float float_srgb2linear(float value);
float float_linear2srgb(float value);
float float_aces(float value);
void float_print(const char *name, float f);

/* vec2 related functions */
vec2_t vec2_new(float x, float y);
vec2_t vec2_min(vec2_t a, vec2_t b);
vec2_t vec2_max(vec2_t a, vec2_t b);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_mul(vec2_t v, float factor);
vec2_t vec2_div(vec2_t v, float divisor);
float vec2_length(vec2_t v);
float vec2_edge(vec2_t start, vec2_t end, vec2_t v);
void vec2_print(const char *name, vec2_t v);

/* vec3 related functions */
vec3_t vec3_new(float x, float y, float z);
vec3_t vec3_from_vec4(vec4_t v);
vec3_t vec3_min(vec3_t a, vec3_t b);
vec3_t vec3_max(vec3_t a, vec3_t b);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_mul(vec3_t v, float factor);
vec3_t vec3_div(vec3_t v, float divisor);
vec3_t vec3_negate(vec3_t v);
float vec3_length(vec3_t v);
vec3_t vec3_normalize(vec3_t v);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec3_t vec3_lerp(vec3_t a, vec3_t b, float t);
vec3_t vec3_saturate(vec3_t v);
vec3_t vec3_modulate(vec3_t a, vec3_t b);
void vec3_print(const char *name, vec3_t v);

/* vec4 related functions */
vec4_t vec4_new(float x, float y, float z, float w);
vec4_t vec4_from_vec3(vec3_t v, float w);
vec4_t vec4_add(vec4_t a, vec4_t b);
vec4_t vec4_sub(vec4_t a, vec4_t b);
vec4_t vec4_mul(vec4_t v, float factor);
vec4_t vec4_div(vec4_t v, float divisor);
vec4_t vec4_lerp(vec4_t a, vec4_t b, float t);
vec4_t vec4_saturate(vec4_t v);
vec4_t vec4_modulate(vec4_t a, vec4_t b);
void vec4_print(const char *name, vec4_t v);

/* quat related functions */
quat_t quat_new(float x, float y, float z, float w);
float quat_dot(quat_t a, quat_t b);
float quat_length(quat_t q);
quat_t quat_normalize(quat_t q);
quat_t quat_slerp(quat_t a, quat_t b, float t);
void quat_print(const char *name, quat_t q);

/* mat3 related functions */
mat3_t mat3_identity(void);
mat3_t mat3_from_cols(vec3_t c0, vec3_t c1, vec3_t c2);
mat3_t mat3_from_mat4(mat4_t m);
mat3_t mat3_combine(mat3_t m[4], vec4_t weights);
vec3_t mat3_mul_vec3(mat3_t m, vec3_t v);
mat3_t mat3_mul_mat3(mat3_t a, mat3_t b);
mat3_t mat3_inverse(mat3_t m);
mat3_t mat3_transpose(mat3_t m);
mat3_t mat3_inverse_transpose(mat3_t m);
void mat3_print(const char *name, mat3_t m);

/* mat4 related functions */
mat4_t mat4_identity(void);
mat4_t mat4_from_quat(quat_t q);
mat4_t mat4_from_trs(vec3_t t, quat_t r, vec3_t s);
mat4_t mat4_combine(mat4_t m[4], vec4_t weights);
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);
mat4_t mat4_inverse(mat4_t m);
mat4_t mat4_transpose(mat4_t m);
mat4_t mat4_inverse_transpose(mat4_t m);
void mat4_print(const char *name, mat4_t m);

/* transformation matrices */
mat4_t mat4_translate(float tx, float ty, float tz);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate(float angle, float vx, float vy, float vz);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up);
mat4_t mat4_ortho(float left, float right, float bottom, float top,
                  float near, float far);
mat4_t mat4_frustum(float left, float right, float bottom, float top,
                    float near, float far);
mat4_t mat4_orthographic(float right, float top, float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);

#endif
